#include "battle.h"
#include "key.h"
//read stick input
int16_t read_stick(uint16_t x)
{
    if (x >= 1448 && x <= 2648)
        return 0;
    return ((int16_t)x) - 2048;
}
//update normal bullets
void update_bullet_norm(struct Bullet *bullet_pool)
{
    for (int16_t i = 0; i < BULLET_POOL; i++) {
        if (bullet_pool[i].is_used) {
            bullet_pool[i].x += bullet_pool[i].v_x;
            bullet_pool[i].y += bullet_pool[i].v_y;
            if (bullet_pool[i].x < -20 || bullet_pool[i].x > 1100
                || bullet_pool[i].y < -20 || bullet_pool[i].y > 620)
                bullet_pool[i].is_used = 0;
        }
    }
}
//update enemy bullets
void update_ebullet_norm(struct Bullet *bullet_pool)
{
    for (int16_t i = 0; i < EBULLET_POOL; i++) {
        if (bullet_pool[i].is_used) {
            bullet_pool[i].x += bullet_pool[i].v_x;
            bullet_pool[i].y += bullet_pool[i].v_y;
            if (bullet_pool[i].x < -20 || bullet_pool[i].x > 1100
                || bullet_pool[i].y < -20 || bullet_pool[i].y > 620)
                bullet_pool[i].is_used = 0;
        }
    }
}
//update seek bullets:mamnge all the v and pos of seek bullets
void update_sbullet(struct Bullet *sbullet_pool, struct Enemy **seek_objects,
                    struct Enemy *enemy_pool)
{
    for (int i = 0; i < SBULLET_POOL; i++) {
        int16_t s_x = 1;
        int16_t s_y = 1;
        if (sbullet_pool[i].is_used) {
            if (sbullet_pool[i].x < -100 || sbullet_pool[i].x > 1300
                || sbullet_pool[i].y < -100 || sbullet_pool[i].y > 700) {
                sbullet_pool[i].is_used = 0;
                continue;
            }
            if (seek_objects[i] == 0 || seek_objects[i]->is_used == 0) {
                seek_objects[i] = &safe_e;
                for (int j = 0; j < ENEMY_POOL; j++) {
                    if (enemy_pool[j].is_used) {
                        seek_objects[i] = enemy_pool + j;
                        break;
                    }
                }
            }
            sbullet_pool[i].direct
                = seek_direct(seek_objects[i]->x, seek_objects[i]->y, sbullet_pool[i].x,
                              sbullet_pool[i].y);
            int32_t dx = (int32_t)(seek_objects[i]->x - sbullet_pool[i].x);
            int32_t dy = (int32_t)(seek_objects[i]->y - sbullet_pool[i].y);
            int32_t d_x = dx * dx;
            int32_t d_y = dy * dy;
            int32_t dist_sq = d_x + d_y;
            if (dist_sq == 0)
                dist_sq = 1;
            if (dx > 0)
                s_x = 1;
            else
                s_x = -1;
            if (dy > 0)
                s_y = 1;
            else
                s_y = -1;
            sbullet_pool[i].x
                += s_x * ((int16_t)(d_x * 25 / dist_sq));
            sbullet_pool[i].y
                += s_y * ((int16_t)(d_y * 25 / dist_sq));
        }
    }
}
//limit and restore mana
void update_mana(void)
{
    if (player_mana < config.max_mana)
        player_mana += config.mana_regen;
    if (player_mana > config.max_mana)
        player_mana = config.max_mana;
}


//movement,nomal attack(gap),skills,game over,knockback,
void update_player(struct Player *player, struct Bullet *bullet_pool,
                   struct Bullet *sbullet_pool, struct Enemy **seek_objects,
                   struct Enemy *enemy_pool, struct Mine *mine_pool)
{
    if (key_value == KEY_S2) {
        bullet_pattern ^= 1;
    }
	if (player->health < 0) is_over = 1;
    if (player_shield_timer > 0)
        player_shield_timer--;
    if (player->v_x * control_x < 0)
        player->v_x = 0;
    if (player->v_y * control_y < 0)
        player->v_y = 0;
    player->v_x += 5 * control_x / 2048;
    player->v_y += 5 * control_y / 2048;
    int16_t new_x = player->x + player->v_x;
    int16_t new_y = player->y + player->v_y;
    if (new_x > 0 && new_x < 900)
        player->x = new_x;
    if (new_y > 0 && new_y < 540)
        player->y = new_y;
    player_attack_timer++;
    if (player_attack_timer % config.attack_gap == 0) {
        uint8_t pattern = bullet_pattern;
        for (int i = 0; i < config.bullet_num[pattern]; i++) {
            create_bullet_v(bullet_pool, BULLET_POOL, B_NORM, player->x + P_W,
                            player->y + P_H / 2 + config.bullet_distirbution[pattern][i],
                            config.bullet_v[pattern][i][0], config.bullet_v[pattern][i][1]);
        }
    }
    
    if (key_value == KEY_S3 && player_mana >= config.shield_cost) {
        player_mana -= config.shield_cost;
        player_shield_timer = config.shield_duration;
    }
    if (key_value == KEY_S4 && player_mana >= config.mine_cost) {
        if (create_mine(mine_pool, player->x + P_W / 2 - MINE_W / 2,
                        player->y + P_H / 2 - MINE_H / 2) != 0)
            player_mana -= config.mine_cost;
    }
    if (key_value == KEY_S5 && player_mana >= config.seek_cost) {
        if (create_sbullet(sbullet_pool, seek_objects, enemy_pool, player->x + P_W,
                           player->y + P_H / 2) != 0)
            player_mana -= config.seek_cost;
    }
    if (control_x == 0 && control_y == 0 && player_knockback_timer == 0) {
        player->v_x /= 2;
        player->v_y /= 2;
    }
    if (player_knockback_timer > 0)
        player_knockback_timer--;

    int16_t min_v_x = player_knockback_timer > 0 ? -60 : -30;
    if (player->v_x > 30)
        player->v_x = 30;
    if (player->v_x < min_v_x)
        player->v_x = min_v_x;
    if (player->v_y > 30)
        player->v_y = 30;
    if (player->v_y < -30)
        player->v_y = -30;
}

static void update_enemy_norm(struct Enemy *e, struct Bullet *ebullet_pool)
{
    if (rand() % 2)
        e->v_x += rand() % 10;
    else
        e->v_x -= rand() % 10;
    if (rand() % 2)
        e->v_y += rand() % 8;
    else
        e->v_y -= rand() % 10;
    if (e->x < 300)  e->v_x = 20;
    if (e->x > 1000) e->v_x = -20;
    if (e->y < 10)   e->v_y = 10;
    if (e->y > 550)  e->v_y = -10;
    e->x += e->v_x;
    e->y += e->v_y;
    e->frame += 1;
    if (e->frame % 10 == 0) {
        create_bullet(ebullet_pool, EBULLET_POOL, B_NORM, e->x, e->y + E_H / 2);
        e->frame = 1;
    }
}

static void update_enemy_shield(struct Enemy *e, struct Bullet *ebullet_pool)
{
    if (rand() % 2)
        e->v_x += rand() % 12;
    else
        e->v_x -= rand() % 12;
    if (rand() % 2)
        e->v_y += rand() % 6;
    else
        e->v_y -= rand() % 6;
    if (e->x < 300)  e->v_x = 15;
    if (e->x > 1000) e->v_x = -15;
    if (e->y < 10)   e->v_y = 8;
    if (e->y > 550)  e->v_y = -8;
    e->x += e->v_x;
    e->y += e->v_y;
    e->frame += 1;
    if (e->frame % 12 == 0) {
        create_bullet(ebullet_pool, EBULLET_POOL, B_NORM, e->x, e->y + E1_H / 2);
        e->frame = 1;
    }
}

static void update_enemy_ram(struct Enemy *e)
{
    e->x += e->v_x;
    e->y += e->v_y;
    if (e->x < -E2_W) {
        e->is_used = 0;
        enemy_num -= 1;
    }
}

static void update_enemy_carrier(struct Enemy *e, struct Enemy *enemy_pool,
                                  struct Bullet *ebullet_pool)
{
    if (rand() % 2)
        e->v_x += rand() % 5;
    else
        e->v_x -= rand() % 5;
    if (rand() % 2)
        e->v_y += rand() % 4;
    else
        e->v_y -= rand() % 4;
    if (e->x < 400)  e->v_x = 10;
    if (e->x > 900)  e->v_x = -10;
    if (e->y < 10)   e->v_y = 8;
    if (e->y > 500)  e->v_y = -8;
    e->x += e->v_x;
    e->y += e->v_y;
    e->frame += 1;
    e->spawn_timer += 1;
    if (e->spawn_timer >= 40) {
        create_enemy(enemy_pool, E_MINION, e->x + E3_W / 2, e->y + E3_H);
        e->spawn_timer = 1;
    }
    if (e->frame % 15 == 0) {
        create_bullet(ebullet_pool, EBULLET_POOL, B_NORM, e->x, e->y + E3_H / 2);
        e->frame = 1;
    }
}

static void create_aimed_ebullet(struct Bullet *ebullet_pool,
                                 int16_t sx, int16_t sy,
                                 int16_t tx, int16_t ty,
                                 int16_t speed)
{
    int32_t dx = tx - sx;
    int32_t dy = ty - sy;
    int32_t adx = dx < 0 ? -dx : dx;
    int32_t ady = dy < 0 ? -dy : dy;
    int16_t vx;
    int16_t vy;

    if (adx == 0 && ady == 0) {
        vx = -speed;
        vy = 0;
    } else if (adx >= ady) {
        vx = dx < 0 ? -speed : speed;
        vy = (int16_t)(dy * speed / adx);
    } else {
        vx = (int16_t)(dx * speed / ady);
        vy = dy < 0 ? -speed : speed;
    }

    create_bullet_v(ebullet_pool, EBULLET_POOL, B_NORM, sx, sy, vx, vy);
}

static void update_enemy_minion(struct Enemy *e, struct Bullet *ebullet_pool,
                                struct Player *player)
{
    if (rand() % 2)
        e->v_x += rand() % 8;
    else
        e->v_x -= rand() % 8;
    if (rand() % 2)
        e->v_y += rand() % 10;
    else
        e->v_y -= rand() % 10;
    if (e->x < 200)  e->v_x = 20;
    if (e->x > 1000) e->v_x = -20;
    if (e->y < 10)   e->v_y = 12;
    if (e->y > 560)  e->v_y = -12;
    e->x += e->v_x;
    e->y += e->v_y;
    e->frame += 1;
    if (e->frame % 14 == 0) {
        create_aimed_ebullet(ebullet_pool,
                             e->x, e->y + E3M_H / 2,
                             player->x + P_W / 2, player->y + P_H / 2,
                             30);
        e->frame = 1;
    }
}

static void update_boss1(struct Enemy *e, struct Enemy *enemy_pool,
                         struct Bullet *ebullet_pool)
{
    if (e->frame % 30 == 0) {
        e->v_x = (rand() % 11) - 5;
        e->v_y = (rand() % 11) - 5;
    }
    if (e->x < 600) e->v_x = 10;
    if (e->x > 900) e->v_x = -10;
    if (e->y < 20)  e->v_y = 10;
    if (e->y > 380) e->v_y = -10;
    e->x += e->v_x;
    e->y += e->v_y;
    e->frame += 1;

    if (e->frame % 5 == 0 && (rand() % 3) != 0) {
        int16_t lanes = 3 + rand() % 2;
        for (int i = 0; i < lanes; i++) {
            int16_t y = e->y + 20 + (BOSS1_H - 40) * i / (lanes - 1 ? lanes - 1 : 1);
            int16_t off = -(rand() % 120);
            create_bullet_v(ebullet_pool, EBULLET_POOL, B_NORM,
                            e->x + off, y, -34, 0);
        }
    }

    e->spawn_timer += 1;
    if (e->spawn_timer >= 50 && (rand() % 2) == 0 && enemy_num < 12) {
        create_enemy(enemy_pool, E_NORM, e->x + BOSS1_W / 2, e->y + BOSS1_H / 2);
        e->spawn_timer = 0;
    }
}


static void update_boss2(struct Enemy *e, struct Enemy *enemy_pool,
                         struct Bullet *ebullet_pool)
{
    if (e->frame % 25 == 0) {
        e->v_x = (rand() % 9) - 4;
        e->v_y = (rand() % 9) - 4;
    }
    if (e->x < 600) e->v_x = 10;
    if (e->x > 900) e->v_x = -10;
    if (e->y < 10)  e->v_y = 10;
    if (e->y > 340) e->v_y = -10;
    e->x += e->v_x;
    e->y += e->v_y;
    e->frame += 1;


    if (e->frame % 2 == 0 && (rand() % 2)) {
        int16_t vx = -22 - (rand() % 8);
        int16_t vy = (rand() % 21) - 10;
        create_bullet_v(ebullet_pool, EBULLET_POOL, B_NORM,
                        e->x, e->y + BOSS2_H / 2, vx, vy);
    }

    e->spawn_timer += 1;
    if (e->spawn_timer >= 24 && (rand() % 3) != 0 && enemy_num < 12) {
        int16_t y = e->y + (rand() % BOSS2_H);
        create_enemy(enemy_pool, E_MINION, e->x + BOSS2_W / 2, y);
        e->spawn_timer = 0;
    }
}


static void update_boss3(struct Enemy *e, struct Enemy *enemy_pool,
                         struct Bullet *ebullet_pool)
{
    if (e->frame % 28 == 0) {
        e->v_x = (rand() % 9) - 4;
        e->v_y = (rand() % 9) - 4;
    }
    if (e->x < 600) e->v_x = 10;
    if (e->x > 900) e->v_x = -10;
    if (e->y < 20)  e->v_y = 10;
    if (e->y > 380) e->v_y = -10;
    e->x += e->v_x;
    e->y += e->v_y;
    e->frame += 1;


    if (e->frame % 5 == 0) {
        int16_t cy = e->y + BOSS3_H / 2;
        int16_t spreads[8][2] = {
            {-22, -18}, {-23, -13}, {-24, -8}, {-26, -3},
            {-26,   3}, {-24,   8}, {-23, 13}, {-22, 18}
        };
        for (int i = 0; i < 8; i++)
            create_bullet_v(ebullet_pool, EBULLET_POOL, B_NORM,
                            e->x, cy, spreads[i][0], spreads[i][1]);
    }

    e->spawn_timer += 1;
    if (e->spawn_timer >= 10 && (rand() % 3) == 0 && enemy_num < 12) {
        int16_t y1 = 100 + (rand() % 100);   
        int16_t y2 = 300 + (rand() % 100);
        int16_t y3 = 400 + (rand() % 100);
        create_enemy(enemy_pool, E_RAM, e->x, y1);
        create_enemy(enemy_pool, E_RAM, e->x, y2);
        create_enemy(enemy_pool, E_RAM, e->x, y3);
        e->spawn_timer = 0;
    }
}

static void update_boss4(struct Enemy *e, struct Enemy *enemy_pool,
                         struct Bullet *ebullet_pool, struct Player *player)
{
    e->frame += 1;
    if (e->spawn_timer > 0) {
        e->v_x = -55;
        e->v_y = 0;
        e->x += e->v_x;
        if (e->x < -BOSS4_W) {
            e->spawn_timer -= 1;
            if (e->spawn_timer > 0) {
                e->x = 1100;
                e->y = 30 + (rand() % (520 - BOSS4_H));
            } else {
                e->spawn_timer = -60;
                e->x = 750 + rand() % 150;
                e->y = 50 + rand() % 200;
            }
        }
        return;
    }
    if (e->spawn_timer < 0) {
        e->spawn_timer += 1;
    }

    if (e->frame % 25 == 0) {
        e->v_x = (rand() % 9) - 4;
        e->v_y = (rand() % 9) - 4;
    }
    if (e->x < 600) e->v_x = 10;
    if (e->x > 900) e->v_x = -10;
    if (e->y < 20)  e->v_y = 10;
    if (e->y > 350) e->v_y = -10;
    e->x += e->v_x;
    e->y += e->v_y;

    if (e->frame % 2 == 0 && (rand() % 2)) {
        create_aimed_ebullet(ebullet_pool,
                             e->x, e->y + BOSS4_H / 2,
                             player->x + P_W / 2, player->y + P_H / 2,
                             28);
    }
    if (e->spawn_timer == 0 && e->frame > 40 && (rand() % 90) == 0) {
        e->spawn_timer = BOSS4_CHARGES;
    }
}

void update_enemy(struct Enemy *enemy, struct Bullet *ebullet_pool,
                  struct Player *player)
{
    for (int16_t i = 0; i < ENEMY_POOL; i++) {
        if (!enemy[i].is_used)
            continue;
        if (enemy[i].health <= 0) {
            enemy[i].is_used = 0;
            enemy_num -= 1;
            if (current_boss == &enemy[i])
                current_boss = 0;
            continue;
        }
        switch (enemy[i].type) {
        case E_SHIELD:
            update_enemy_shield(&enemy[i], ebullet_pool);
            break;
        case E_RAM:
            update_enemy_ram(&enemy[i]);
            break;
        case E_CARRIER:
            update_enemy_carrier(&enemy[i], enemy, ebullet_pool);
            break;
        case E_MINION:
            update_enemy_minion(&enemy[i], ebullet_pool, player);
            break;
        case E_BOSS1:
            update_boss1(&enemy[i], enemy, ebullet_pool);
            break;
        case E_BOSS2:
            update_boss2(&enemy[i], enemy, ebullet_pool);
            break;
        case E_BOSS3:
            update_boss3(&enemy[i], enemy, ebullet_pool);
            break;
        case E_BOSS4:
            update_boss4(&enemy[i], enemy, ebullet_pool, player);
            break;
        default:
            update_enemy_norm(&enemy[i], ebullet_pool);
            break;
        }
    }
}


void enemy_dispatcher(struct Enemy *enemy_pool)
{
    //the y pos of ram lanes
    static const int16_t ram_lanes[RAM_LANE_COUNT] = { 60, 140, 220, 300, 380, 460 };
    uint16_t dispatch_rows = config.wave_count;

    if (dispatch_rows == 0) {
        is_over = 2;
        return;
    }
    if (dispatch_rows > DISPATCH_ROWS)
        dispatch_rows = DISPATCH_ROWS;

    uint16_t dispatch_end = dispatch_rows * DISPATCH_COLS;
    if (dispatch_cursor >= dispatch_end) {
        round_timer = 0;
        is_over = 2;
        return;
    }
    int16_t *slot = config.enemy_dispatch + dispatch_cursor;
    int16_t boss_type   = slot[5];
    int16_t min_frames  = slot[4];

    if (round_timer == 0) {
        for (int i = 0; i < slot[0]; i++)
            create_enemy(enemy_pool, E_NORM, 1400, 100 + rand() % 400);
        for (int i = 0; i < slot[1]; i++)
            create_enemy(enemy_pool, E_SHIELD, 1400, 100 + rand() % 400);
        int16_t lane_order[RAM_LANE_COUNT];
        for (int i = 0; i < RAM_LANE_COUNT; i++)
            lane_order[i] = i;
        for (int i = 0; i < slot[2] && i < RAM_LANE_COUNT; i++) {
            int16_t pick = i + rand() % (RAM_LANE_COUNT - i);
            int16_t lane = lane_order[pick];
            lane_order[pick] = lane_order[i];
            lane_order[i] = lane;
            create_enemy(enemy_pool, E_RAM, 1200, ram_lanes[lane]);
        }
        for (int i = 0; i < slot[3]; i++)
            create_enemy(enemy_pool, E_CARRIER, 1400, 200 + rand() % 200);
        if (boss_type >= 1 && boss_type <= 4) {
            enum Enemy_types bt = E_BOSS1;
            int16_t bw = BOSS1_W, bh = BOSS1_H;
            switch (boss_type) {
            case 1: bt = E_BOSS1; bw = BOSS1_W; bh = BOSS1_H; break;
            case 2: bt = E_BOSS2; bw = BOSS2_W; bh = BOSS2_H; break;
            case 3: bt = E_BOSS3; bw = BOSS3_W; bh = BOSS3_H; break;
            case 4: bt = E_BOSS4; bw = BOSS4_W; bh = BOSS4_H; break;
            }
            int16_t by = 50 + (rand() % (520 - bh > 50 ? 520 - bh : 50));
            current_boss = create_enemy(enemy_pool, bt, 1100, by);
        }
        round_timer = 1;  
        return;
    }

    round_timer += 1;
    bool boss_alive = (boss_type >= 1 && boss_type <= 4) && current_boss != 0;
    if (round_timer >= min_frames && !boss_alive && enemy_num <= 2) {
        dispatch_cursor += DISPATCH_COLS;
        round_timer = 0;
        if (dispatch_cursor >= dispatch_end)
            is_over = 2;
    }
}

