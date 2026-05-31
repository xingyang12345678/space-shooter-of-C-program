#include "battle.h"
#define DIRTY_AREA_LIMIT 4

static int16_t pre_enemy_w[ENEMY_POOL];
static int16_t pre_enemy_h[ENEMY_POOL];
static lv_area_t dirty_areas[DIRTY_AREA_LIMIT];
static int16_t dirty_area_count = 0;
static bool dirty_batching = false;

static bool clip_rect(int16_t *x, int16_t *y, int16_t *w, int16_t *h,
                      int16_t max_w, int16_t max_h)
{
    if (*x >= max_w || *y >= max_h || *x + *w <= 0 || *y + *h <= 0)
        return false;
    if (*x < 0) {
        *w += *x;
        *x = 0;
    }
    if (*y < 0) {
        *h += *y;
        *y = 0;
    }
    if (*x + *w > max_w)
        *w = max_w - *x;
    if (*y + *h > max_h)
        *h = max_h - *y;
    return *w > 0 && *h > 0;
}

static int32_t dirty_area_size(const lv_area_t *area)
{
    return (int32_t)(area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
}

static bool dirty_area_overlaps(const lv_area_t *a, const lv_area_t *b)
{
    return !(a->x2 < b->x1 || b->x2 < a->x1 || a->y2 < b->y1 || b->y2 < a->y1);
}

static lv_area_t dirty_area_joined(const lv_area_t *a, const lv_area_t *b)
{
    lv_area_t joined;
    joined.x1 = a->x1 < b->x1 ? a->x1 : b->x1;
    joined.y1 = a->y1 < b->y1 ? a->y1 : b->y1;
    joined.x2 = a->x2 > b->x2 ? a->x2 : b->x2;
    joined.y2 = a->y2 > b->y2 ? a->y2 : b->y2;
    return joined;
}

static int16_t align_img_x(int16_t x)
{
    return x & ~1;
}

static void align_dirty_area_for_dma(lv_obj_t *canvas, lv_area_t *area)
{
    lv_img_dsc_t *dsc = lv_canvas_get_img(canvas);
    int16_t max_x = dsc->header.w - 1;

    area->x1 &= ~3;
    area->x2 |= 3;
    if (area->x1 < 0)
        area->x1 = 0;
    if (area->x2 > max_x)
        area->x2 = max_x;
}

static void invalidate_canvas_area(lv_obj_t *canvas, int16_t x, int16_t y,
                                   int16_t w, int16_t h)
{
    lv_area_t area = { x, y, x + w - 1, y + h - 1 };
    align_dirty_area_for_dma(canvas, &area);

    if (!dirty_batching) {
        lv_area_t canvas_coords;
        lv_obj_get_coords(canvas, &canvas_coords);
        area.x1 += canvas_coords.x1;
        area.x2 += canvas_coords.x1;
        area.y1 += canvas_coords.y1;
        area.y2 += canvas_coords.y1;
        lv_obj_invalidate_area(canvas, &area);
        return;
    }

    for (int16_t i = 0; i < dirty_area_count; i++) {
        if (dirty_area_overlaps(&dirty_areas[i], &area)) {
            dirty_areas[i] = dirty_area_joined(&dirty_areas[i], &area);
            return;
        }
    }

    if (dirty_area_count < DIRTY_AREA_LIMIT) {
        dirty_areas[dirty_area_count++] = area;
        return;
    }

    int16_t best = 0;
    int32_t best_cost = 0x7fffffff;
    for (int16_t i = 0; i < dirty_area_count; i++) {
        lv_area_t joined = dirty_area_joined(&dirty_areas[i], &area);
        int32_t cost = dirty_area_size(&joined) - dirty_area_size(&dirty_areas[i]);
        if (cost < best_cost) {
            best_cost = cost;
            best = i;
        }
    }
    dirty_areas[best] = dirty_area_joined(&dirty_areas[best], &area);
}

static void flush_dirty_areas(lv_obj_t *canvas)
{
    lv_area_t canvas_coords;
    lv_obj_get_coords(canvas, &canvas_coords);
    for (int16_t i = 0; i < dirty_area_count; i++) {
        lv_area_t area = dirty_areas[i];
        area.x1 += canvas_coords.x1;
        area.x2 += canvas_coords.x1;
        area.y1 += canvas_coords.y1;
        area.y2 += canvas_coords.y1;
        lv_obj_invalidate_area(canvas, &area);
    }
    dirty_area_count = 0;
}

static void fill_canvas_rect(lv_obj_t *canvas, int16_t x, int16_t y,
                             int16_t w, int16_t h, lv_color_t color)
{
    lv_img_dsc_t *dsc = lv_canvas_get_img(canvas);
    int16_t canvas_w = dsc->header.w;
    int16_t canvas_h = dsc->header.h;

    if (!clip_rect(&x, &y, &w, &h, canvas_w, canvas_h))
        return;

    lv_color_t *buf = (lv_color_t *)dsc->data;
    for (int16_t row = 0; row < h; row++) {
        lv_color_t *dst = buf + (y + row) * canvas_w + x;
        for (int16_t col = 0; col < w; col++)
            dst[col] = color;
    }
    invalidate_canvas_area(canvas, x, y, w, h);
}

static void fill_canvas_circle(lv_obj_t *canvas, int16_t x, int16_t y,
                               int16_t diameter, lv_color_t color)
{
    lv_img_dsc_t *dsc = lv_canvas_get_img(canvas);
    int16_t canvas_w = dsc->header.w;
    int16_t canvas_h = dsc->header.h;
    int16_t draw_x = x;
    int16_t draw_y = y;
    int16_t draw_w = diameter;
    int16_t draw_h = diameter;
    int16_t cx = diameter / 2;
    int16_t cy = diameter / 2;
    int16_t r = diameter / 2;

    if (!clip_rect(&draw_x, &draw_y, &draw_w, &draw_h, canvas_w, canvas_h))
        return;

    lv_color_t *buf = (lv_color_t *)dsc->data;
    for (int16_t row = 0; row < draw_h; row++) {
        int16_t src_y = draw_y - y + row;
        int16_t dy = src_y - cy;
        lv_color_t *dst = buf + (draw_y + row) * canvas_w + draw_x;
        for (int16_t col = 0; col < draw_w; col++) {
            int16_t src_x = draw_x - x + col;
            int16_t dx = src_x - cx;
            if (dx * dx + dy * dy <= r * r)
                dst[col] = color;
        }
    }
    invalidate_canvas_area(canvas, draw_x, draw_y, draw_w, draw_h);
}

static void draw_canvas_img(lv_obj_t *canvas, int16_t x, int16_t y,
                            const lv_img_dsc_t *img)
{
    lv_img_dsc_t *canvas_dsc = lv_canvas_get_img(canvas);
    int16_t canvas_w = canvas_dsc->header.w;
    int16_t canvas_h = canvas_dsc->header.h;
    int16_t draw_x = align_img_x(x);
    int16_t draw_y = y;
    int16_t draw_w = img->header.w;
    int16_t draw_h = img->header.h;
    int16_t src_x = 0;
    int16_t src_y = 0;

    if (draw_x < 0) {
        src_x = -draw_x;
        draw_w += draw_x;
        draw_x = 0;
    }
    if (draw_y < 0) {
        src_y = -draw_y;
        draw_h += draw_y;
        draw_y = 0;
    }
    if (!clip_rect(&draw_x, &draw_y, &draw_w, &draw_h, canvas_w, canvas_h))
        return;

    lv_color_t *dst_buf = (lv_color_t *)canvas_dsc->data;
    const lv_color_t *src_buf = (const lv_color_t *)img->data;
    for (int16_t row = 0; row < draw_h; row++) {
        lv_color_t *dst = dst_buf + (draw_y + row) * canvas_w + draw_x;
        const lv_color_t *src = src_buf + (src_y + row) * img->header.w + src_x;
        if ((((uint32_t)dst | (uint32_t)src) & 3U) == 0U) {
            uint32_t *dst32 = (uint32_t *)dst;
            const uint32_t *src32 = (const uint32_t *)src;
            int16_t pairs = draw_w / 2;
            for (int16_t col = 0; col < pairs; col++)
                dst32[col] = src32[col];
            if (draw_w & 1)
                dst[draw_w - 1] = src[draw_w - 1];
        } else {
            for (int16_t col = 0; col < draw_w; col++)
                dst[col] = src[col];
        }
    }
    invalidate_canvas_area(canvas, draw_x, draw_y, draw_w, draw_h);
}

void init_pre_bullet(void)
{
    for (int16_t i = 0; i < BULLET_POOL; i++) {
        pre_bullet_positions[i][0] = -1000;
        pre_bullet_positions[i][1] = -1000;
    }
}

void init_pre_ebullet(void)
{
    for (int16_t i = 0; i < EBULLET_POOL; i++) {
        pre_ebullet_positions[i][0] = -1000;
        pre_ebullet_positions[i][1] = -1000;
    }
}

void init_pre_enemy(void)
{
    for (int16_t i = 0; i < ENEMY_POOL; i++) {
        pre_enemy[i][0] = -1000;
        pre_enemy[i][1] = -1000;
        pre_enemy_w[i]  = E_W;
        pre_enemy_h[i]  = E_H;
    }
}

void init_pre_seek(void)
{
    for (int i = 0; i < SBULLET_POOL; i++) {
        pre_seek[i][0] = -1000;
        pre_seek[i][1] = -1000;
    }
}

void init_pre_mine(void)
{
    for (int i = 0; i < MINE_POOL; i++) {
        pre_mine[i][0] = -1000;
        pre_mine[i][1] = -1000;
    }
}

void clear_area(lv_obj_t *canvas, int16_t x, int16_t y, int16_t w, int16_t h)
{
    fill_canvas_rect(canvas, x, y, w, h, lv_color_black());
}


int16_t enemy_w(struct Enemy *e)
{
    switch (e->type) {
    case E_SHIELD: return E1_W;
    case E_RAM:    return E2_W;
    case E_CARRIER: return E3_W;
    case E_MINION: return E3M_W;
    case E_BOSS1:  return BOSS1_W;
    case E_BOSS2:  return BOSS2_W;
    case E_BOSS3:  return BOSS3_W;
    case E_BOSS4:  return BOSS4_W;
    default:       return E_W;
    }
}

int16_t enemy_h(struct Enemy *e)
{
    switch (e->type) {
    case E_SHIELD: return E1_H;
    case E_RAM:    return E2_H;
    case E_CARRIER: return E3_H;
    case E_MINION: return E3M_H;
    case E_BOSS1:  return BOSS1_H;
    case E_BOSS2:  return BOSS2_H;
    case E_BOSS3:  return BOSS3_H;
    case E_BOSS4:  return BOSS4_H;
    default:       return E_H;
    }
}

static lv_img_dsc_t *pick_enemy_dsc(struct Enemy *e, lv_img_dsc_t *enemy_dsc,
                                     lv_img_dsc_t *e1_shield_dsc,
                                     lv_img_dsc_t *e1_plain_dsc,
                                     lv_img_dsc_t *e2_ram_dsc,
                                     lv_img_dsc_t *e3_carrier_dsc,
                                     lv_img_dsc_t *e3_minion_dsc,
                                     lv_img_dsc_t *boss1_dsc,
                                     lv_img_dsc_t *boss2_dsc,
                                     lv_img_dsc_t *boss3_dsc,
                                     lv_img_dsc_t *boss4_dsc)
{
    switch (e->type) {
    case E_SHIELD:
        return (e->shield_hp > 0) ? e1_shield_dsc : e1_plain_dsc;
    case E_RAM:     return e2_ram_dsc;
    case E_CARRIER: return e3_carrier_dsc;
    case E_MINION:  return e3_minion_dsc;
    case E_BOSS1:   return boss1_dsc;
    case E_BOSS2:   return boss2_dsc;
    case E_BOSS3:   return boss3_dsc;
    case E_BOSS4:   return boss4_dsc;
    default:        return enemy_dsc;
    }
}

void render(lv_obj_t *m_canva, struct Player *player, struct Enemy *enemy_pool,
            struct Bullet *bullet_pool, struct Bullet *ebullet_pool,
            struct Bullet *sbullet_pool, struct Mine *mine_pool,
            lv_img_dsc_t *player_dsc, lv_img_dsc_t *player_shield_dsc,
            lv_draw_rect_dsc_t *circle_dsc, lv_draw_rect_dsc_t *ebullet_dsc,
            lv_img_dsc_t *enemy_dsc, lv_img_dsc_t *seek_dsc,
            lv_img_dsc_t *mine_dsc,
            lv_img_dsc_t *e1_shield_dsc, lv_img_dsc_t *e1_plain_dsc,
            lv_img_dsc_t *e2_ram_dsc,
            lv_img_dsc_t *e3_carrier_dsc, lv_img_dsc_t *e3_minion_dsc,
            lv_img_dsc_t *boss1_dsc, lv_img_dsc_t *boss2_dsc,
            lv_img_dsc_t *boss3_dsc, lv_img_dsc_t *boss4_dsc)
{
    dirty_area_count = 0;
    dirty_batching = true;

    // clear previous positions 
    if (pre_player_x != -1000)
        clear_area(m_canva, pre_player_x, pre_player_y, PS_W, PS_H);
    for (int16_t i = 0; i < MINE_POOL; i++)
        if (pre_mine[i][0] != -1000)
            clear_area(m_canva, pre_mine[i][0], pre_mine[i][1], MINE_W, MINE_H);
    for (int16_t i = 0; i < BULLET_POOL; i++)
        if (pre_bullet_positions[i][0] != -1000)
            clear_area(m_canva, pre_bullet_positions[i][0], pre_bullet_positions[i][1],
                       10, 10);
    for (int16_t i = 0; i < EBULLET_POOL; i++)
        if (pre_ebullet_positions[i][0] != -1000)
            clear_area(m_canva, pre_ebullet_positions[i][0],
                       pre_ebullet_positions[i][1], 10, 10);
    for (int16_t i = 0; i < ENEMY_POOL; i++)
        if (pre_enemy[i][0] != -1000)
            clear_area(m_canva, pre_enemy[i][0], pre_enemy[i][1],
                       pre_enemy_w[i], pre_enemy_h[i]);
    for (int16_t i = 0; i < SBULLET_POOL; i++)
        if (pre_seek[i][0] != -1000)
            clear_area(m_canva, pre_seek[i][0], pre_seek[i][1], 50, 50);
    // render mine
    for (int16_t i = 0; i < MINE_POOL; i++) {
        if (mine_pool[i].is_used) {
            draw_canvas_img(m_canva, mine_pool[i].x, mine_pool[i].y, mine_dsc);
            pre_mine[i][0] = align_img_x(mine_pool[i].x);
            pre_mine[i][1] = mine_pool[i].y;
        } else {
            pre_mine[i][0] = -1000;
            pre_mine[i][1] = -1000;
        }
    }
    // render player
    lv_img_dsc_t *p_dsc = (player_shield_timer > 0) ? player_shield_dsc : player_dsc;
    draw_canvas_img(m_canva, player->x, player->y, p_dsc);
    // render enemy
    for (int16_t i = 0; i < ENEMY_POOL; i++) {
        if (enemy_pool[i].is_used) {
            lv_img_dsc_t *dsc = pick_enemy_dsc(&enemy_pool[i], enemy_dsc,
                                                e1_shield_dsc, e1_plain_dsc,
                                                e2_ram_dsc, e3_carrier_dsc,
                                                e3_minion_dsc,
                                                boss1_dsc, boss2_dsc,
                                                boss3_dsc, boss4_dsc);
            draw_canvas_img(m_canva, enemy_pool[i].x, enemy_pool[i].y, dsc);
            pre_enemy[i][0] = align_img_x(enemy_pool[i].x);
            pre_enemy[i][1] = enemy_pool[i].y;
            pre_enemy_w[i]  = enemy_w(&enemy_pool[i]);
            pre_enemy_h[i]  = enemy_h(&enemy_pool[i]);
        } else {
            if (pre_enemy[i][0] != -1000)
                clear_area(m_canva, pre_enemy[i][0], pre_enemy[i][1],
                           pre_enemy_w[i], pre_enemy_h[i]);
            pre_enemy[i][0] = -1000;
            pre_enemy[i][1] = -1000;
        }
    }
    // render bullet 
    for (int16_t i = 0; i < BULLET_POOL; i++) {
        if (bullet_pool[i].is_used) {
            fill_canvas_circle(m_canva, bullet_pool[i].x, bullet_pool[i].y,
                               10, circle_dsc->bg_color);
            pre_bullet_positions[i][0] = bullet_pool[i].x;
            pre_bullet_positions[i][1] = bullet_pool[i].y;
        } else {
            pre_bullet_positions[i][0] = -1000;
            pre_bullet_positions[i][1] = -1000;
        }
    }
    // render ebullet 
    for (int16_t i = 0; i < EBULLET_POOL; i++) {
        if (ebullet_pool[i].is_used) {
            fill_canvas_circle(m_canva, ebullet_pool[i].x, ebullet_pool[i].y,
                               10, ebullet_dsc->bg_color);
            pre_ebullet_positions[i][0] = ebullet_pool[i].x;
            pre_ebullet_positions[i][1] = ebullet_pool[i].y;
        } else {
            pre_ebullet_positions[i][0] = -1000;
            pre_ebullet_positions[i][1] = -1000;
        }
    }
    // render seek bullet
    for (int16_t i = 0; i < SBULLET_POOL; i++) {
        if (sbullet_pool[i].is_used) {
            draw_canvas_img(m_canva, sbullet_pool[i].x, sbullet_pool[i].y,
                            seek_dsc + sbullet_pool[i].direct);
            pre_seek[i][0] = align_img_x(sbullet_pool[i].x);
            pre_seek[i][1] = sbullet_pool[i].y;
        } else {
            pre_seek[i][0] = -1000;
            pre_seek[i][1] = -1000;
        }
    }
    pre_player_x = align_img_x(player->x);
    pre_player_y = player->y;
    dirty_batching = false;
    flush_dirty_areas(m_canva);
}
