#include "battle.h"

int16_t is_in(int16_t x, int16_t y, int16_t w, int16_t h,
              int16_t b_x, int16_t b_y, int16_t b_w, int16_t b_h)
{
    if (b_x <= x && b_y <= y) {
        if (b_x + b_w < x + w && b_x + w > x && b_y + b_h > y && b_y + b_h < y + h)
            return 2;
    }
    if (b_x > x && b_y < y) {
        if (b_x < x + w && b_x > x && b_y + b_h > y && b_y + b_h < y + h)
            return 1;
    }
    if (b_x < x && b_y > y) {
        if (b_x + b_w > x && b_x + b_w < x + w && b_y > y && b_y < y + h)
            return 3;
    }
    if (b_x > x && b_y > y) {
        if (b_x > x && b_x < x + w && b_y > y && b_y < y + h)
            return 4;
    }
    return 0;
}

uint8_t seek_direct(int16_t a, int16_t b, int16_t x, int16_t y)
{
    int16_t d_x = x - a;
    int16_t d_y = y - b;
    if (d_x == 0 && d_y == 0)
        return 0;
    if (d_x == 0 && d_y > 0)
        return 0;
    if (d_x < 0 && d_y > 0 && -d_x <= d_y)
        return 1;
    if (d_x < 0 && d_y > 0 && -d_x > d_y)
        return 2;
    if (d_x < 0 && d_y == 0)
        return 3;
    if (d_x < 0 && d_y < 0 && -d_x >= -d_y)
        return 4;
    if (d_x < 0 && d_y < 0 && -d_x < -d_y)
        return 5;
    if (d_x == 0 && d_y < 0)
        return 6;
    if (d_x > 0 && d_y < 0 && d_x <= -d_y)
        return 7;
    if (d_x > 0 && d_y < 0 && d_x > -d_y)
        return 8;
    if (d_x > 0 && d_y == 0)
        return 9;
    if (d_x > 0 && d_y > 0 && d_x >= d_y)
        return 10;
    if (d_x > 0 && d_y > 0 && d_x < d_y)
        return 11;
    return 0;
}

static void apply_hit(struct Enemy *e, int16_t dmg, struct Bullet *ebullet_pool)
{
    if (e->type == E_SHIELD && e->shield_hp > 0) {
        e->shield_hp -= dmg;
        if (e->shield_hp <= 0) {
            e->shield_hp = 0;
            int16_t cx = e->x + E1_W / 2;
            int16_t cy = e->y + E1_H / 2;
            int16_t burst_v[SHIELD_BURST][2] = {
                { -20, 0 }, { 20, 0 }, { 0, -20 }, { 0, 20 },
                { -14, -14 }, { 14, -14 }, { -14, 14 }, { 14, 14 }
            };
            for (int i = 0; i < SHIELD_BURST; i++)
                create_bullet_v(ebullet_pool, EBULLET_POOL, B_NORM, cx, cy,
                                burst_v[i][0], burst_v[i][1]);
        }
    } else {
        e->health -= dmg;
    }
}

void collision_detect(struct Player *player, struct Enemy *enemy_pool,
                      struct Bullet *bullet_pool, struct Bullet *ebullet_pool,
                      struct Bullet *sbullet_pool, struct Mine *mine_pool)
{
    int16_t ew, eh;
    // seek bullet vs enemy 
    for (int16_t i = 0; i < SBULLET_POOL; i++) {
        if (sbullet_pool[i].is_used) {
            for (int16_t j = 0; j < ENEMY_POOL; j++) {
                if (enemy_pool[j].is_used) {
                    ew = enemy_w(&enemy_pool[j]);
                    eh = enemy_h(&enemy_pool[j]);
                    if (is_in(enemy_pool[j].x, enemy_pool[j].y, ew, eh,
                              sbullet_pool[i].x, sbullet_pool[i].y, 50, 50)) {
                        sbullet_pool[i].is_used = 0;
                        apply_hit(&enemy_pool[j], config.seek_damage, ebullet_pool);
                        if (enemy_pool[j].health <= 0) {
                            player_mana += config.mana_kill_bonus;
                            player_score += 1;
                        }
                        break;
                    }
                }
            }
        }
    }
    // mine vs enemy: enemy touches mine  
    for (int16_t i = 0; i < MINE_POOL; i++) {
        if (!mine_pool[i].is_used)
            continue;
        for (int16_t j = 0; j < ENEMY_POOL; j++) {
            if (!enemy_pool[j].is_used)
                continue;
            ew = enemy_w(&enemy_pool[j]);
            eh = enemy_h(&enemy_pool[j]);
            if (is_in(enemy_pool[j].x, enemy_pool[j].y, ew, eh,
                      mine_pool[i].x, mine_pool[i].y, MINE_W, MINE_H)) {
                mine_pool[i].is_used = 0;
                enemy_pool[j].health -= config.mine_damage;
                if (enemy_pool[j].health <= 0) {
                    player_mana += config.mana_kill_bonus;
                    player_score += 1;
                }
                break;
            }
        }
    }
    // ebullet vs player 
    for (int16_t i = 0; i < EBULLET_POOL; i++) {
        if (ebullet_pool[i].is_used) {
            if (is_in(player->x, player->y, P_W, P_H, ebullet_pool[i].x,
                      ebullet_pool[i].y, 10, 10)) {
                ebullet_pool[i].is_used = 0;
                if (player_shield_timer <= 0)
                    player->health -= 10;
								else player_shield_timer -= 20;
            }
        }
    }
    // bullet vs enemy 
    for (int16_t i = 0; i < BULLET_POOL; i++) {
        if (bullet_pool[i].is_used) {
            for (int16_t j = 0; j < ENEMY_POOL; j++) {
                if (enemy_pool[j].is_used) {
                    ew = enemy_w(&enemy_pool[j]);
                    eh = enemy_h(&enemy_pool[j]);
                    if (is_in(enemy_pool[j].x, enemy_pool[j].y, ew, eh,
                              bullet_pool[i].x, bullet_pool[i].y, 10, 10)) {
                        bullet_pool[i].is_used = 0;
                        apply_hit(&enemy_pool[j], config.bullet_damage, ebullet_pool);
                        if (enemy_pool[j].health <= 0) {
                            player_mana += config.mana_kill_bonus;
                            player_score += 1;
                        }
                    }
                }
            }
        }
    }
    // enemy vs enemy collision 
    for (int16_t i = 0; i < ENEMY_POOL - 1; i++) {
        int16_t col_sort;
        if (enemy_pool[i].is_used) {
            int16_t wi = enemy_w(&enemy_pool[i]);
            int16_t hi = enemy_h(&enemy_pool[i]);
            for (int16_t j = i + 1; j < ENEMY_POOL; j++) {
                if (enemy_pool[j].is_used) {
                    int16_t wj = enemy_w(&enemy_pool[j]);
                    int16_t hj = enemy_h(&enemy_pool[j]);
                    col_sort = is_in(enemy_pool[i].x, enemy_pool[i].y, wi, hi,
                                     enemy_pool[j].x, enemy_pool[j].y, wj, hj);
                    if (col_sort) {
                        if (enemy_pool[i].type != E_RAM) {
                            switch (col_sort) {
                            case 1: enemy_pool[i].v_y = 5;  enemy_pool[i].v_x = -5; break;
                            case 2: enemy_pool[i].v_y = 5;  enemy_pool[i].v_x = 5;  break;
                            case 3: enemy_pool[i].v_y = -5; enemy_pool[i].v_x = 5;  break;
                            case 4: enemy_pool[i].v_y = -5; enemy_pool[i].v_x = -5; break;
                            default: break;
                            }
                        }
                        if (enemy_pool[j].type != E_RAM) {
                            switch (col_sort) {
                            case 1: enemy_pool[j].v_y = -5; enemy_pool[j].v_x = 5;  break;
                            case 2: enemy_pool[j].v_y = -5; enemy_pool[j].v_x = -5; break;
                            case 3: enemy_pool[j].v_y = 5;  enemy_pool[j].v_x = -5; break;
                            case 4: enemy_pool[j].v_y = 5;  enemy_pool[j].v_x = 5;  break;
                            default: break;
                            }
                        }
                    }
                }
            }
        }
    }
    // enemy vs player collision  
    for (int16_t i = 0; i < ENEMY_POOL; i++) {
        int16_t col_sort;
        if (enemy_pool[i].is_used) {
            ew = enemy_w(&enemy_pool[i]);
            eh = enemy_h(&enemy_pool[i]);
            col_sort = is_in(enemy_pool[i].x, enemy_pool[i].y, ew, eh,
                             player->x, player->y, P_W, P_H);
            if (col_sort) {
                if (player_shield_timer <= 0) {
                    if (enemy_pool[i].type == E_RAM)
                        player->health -= 50;
                    else{
                        player->health -= 20;
										}
                }
								else {
                    if (enemy_pool[i].type == E_RAM){
                        player->health -= 20;
												player_shield_timer-=50;
										}
                    else{
                        player->health -= 10;
												player_shield_timer-=50;
										}									
								}
                if (enemy_pool[i].type == E_RAM) {
                    enemy_pool[i].health = 0;
                } else {
                    enemy_pool[i].health -= 20;
                }
                if (enemy_pool[i].health <= 0) {
                    player_mana += config.mana_kill_bonus;
                    player_score += 1;
                }
                switch (col_sort) {
                case 1:
                    player->v_y = -5; player->v_x = 5;
                    break;
                case 2:
                    player->v_y = -5; player->v_x = -5;
                    break;
                case 3:
                    player->v_y = 5; player->v_x = -5;
                    break;
                case 4:
                    player->v_y = 5; player->v_x = 5;
                    break;
                default:
                    break;
                }
                if (enemy_pool[i].type != E_RAM) {
                    switch (col_sort) {
                    case 1: enemy_pool[i].v_y = 5;  enemy_pool[i].v_x = -5; break;
                    case 2: enemy_pool[i].v_y = 5;  enemy_pool[i].v_x = 5;  break;
                    case 3: enemy_pool[i].v_y = -5; enemy_pool[i].v_x = 5;  break;
                    case 4: enemy_pool[i].v_y = -5; enemy_pool[i].v_x = -5; break;
                    default: break;
                    }
                }
            }
        }
    }
}
