#include "battle.h"
static int16_t pre_enemy_w[ENEMY_POOL];
static int16_t pre_enemy_h[ENEMY_POOL];

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
    lv_draw_rect_dsc_t bg_dsc;
    lv_draw_rect_dsc_init(&bg_dsc);
    bg_dsc.bg_color = lv_color_black();
    bg_dsc.bg_opa   = LV_OPA_COVER;
    lv_canvas_draw_rect(canvas, x, y, w, h, &bg_dsc);
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
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);
    img_dsc.opa = LV_OPA_COVER;
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
            lv_canvas_draw_img(m_canva, mine_pool[i].x, mine_pool[i].y, mine_dsc,
                               &img_dsc);
            pre_mine[i][0] = mine_pool[i].x;
            pre_mine[i][1] = mine_pool[i].y;
        } else {
            pre_mine[i][0] = -1000;
            pre_mine[i][1] = -1000;
        }
    }
    // render player
    lv_img_dsc_t *p_dsc = (player_shield_timer > 0) ? player_shield_dsc : player_dsc;
    lv_canvas_draw_img(m_canva, player->x, player->y, p_dsc, &img_dsc);
    // render enemy
    for (int16_t i = 0; i < ENEMY_POOL; i++) {
        if (enemy_pool[i].is_used) {
            lv_img_dsc_t *dsc = pick_enemy_dsc(&enemy_pool[i], enemy_dsc,
                                                e1_shield_dsc, e1_plain_dsc,
                                                e2_ram_dsc, e3_carrier_dsc,
                                                e3_minion_dsc,
                                                boss1_dsc, boss2_dsc,
                                                boss3_dsc, boss4_dsc);
            lv_canvas_draw_img(m_canva, enemy_pool[i].x, enemy_pool[i].y, dsc,
                               &img_dsc);
            pre_enemy[i][0] = enemy_pool[i].x;
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
            lv_canvas_draw_rect(m_canva, bullet_pool[i].x, bullet_pool[i].y, 10, 10,
                                circle_dsc);
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
            lv_canvas_draw_rect(m_canva, ebullet_pool[i].x, ebullet_pool[i].y, 10, 10,
                                ebullet_dsc);
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
            lv_canvas_draw_img(m_canva, sbullet_pool[i].x, sbullet_pool[i].y,
                               seek_dsc + sbullet_pool[i].direct, &img_dsc);
            pre_seek[i][0] = sbullet_pool[i].x;
            pre_seek[i][1] = sbullet_pool[i].y;
        } else {
            pre_seek[i][0] = -1000;
            pre_seek[i][1] = -1000;
        }
    }
    pre_player_x = player->x;
    pre_player_y = player->y;
}
