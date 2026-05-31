#include "battle.h"

//create enemy
struct Enemy *create_enemy(struct Enemy *enemy_pool, enum Enemy_types type,
                           int16_t x, int16_t y)
{
    for (int16_t i = 0; i < ENEMY_POOL; i++) {
        if (enemy_pool[i].is_used == 0) {
            enemy_pool[i].is_used      = 1;
            enemy_pool[i].x            = x;
            enemy_pool[i].y            = y;
            enemy_pool[i].type         = type;
            enemy_pool[i].frame        = 1;
            enemy_pool[i].v_x          = 0;
            enemy_pool[i].v_y          = 0;
            enemy_pool[i].shield_hp    = 0;
            enemy_pool[i].spawn_timer  = 0;
            switch (type) {
            case E_SHIELD:
                enemy_pool[i].health    = 100;
                enemy_pool[i].shield_hp = SHIELD_HP;
                break;
            case E_RAM:
                enemy_pool[i].health = 60;
                enemy_pool[i].v_x    = -25;
                enemy_pool[i].v_y    = 0;
                break;
            case E_CARRIER:
                enemy_pool[i].health      = 250;
                enemy_pool[i].spawn_timer = 1;
                break;
            case E_MINION:
                enemy_pool[i].health = 30;
                break;
            case E_BOSS1:
                enemy_pool[i].health = 2500;
                break;
            case E_BOSS2:
                enemy_pool[i].health = 2800;
                break;
            case E_BOSS3:
                enemy_pool[i].health = 2000;
                break;
            case E_BOSS4:
                enemy_pool[i].health = 2200;
                break;
            default:
                enemy_pool[i].health = 100;
                break;
            }
            enemy_pool[i].max_health = enemy_pool[i].health;
            enemy_num += 1;
            return &enemy_pool[i];
        }
    }
    return 0;
}
//create bullets with v 
struct Bullet *create_bullet_v(struct Bullet *bullet_pool, int16_t pool_size,
                               enum Bullet_types type,
                               int16_t x, int16_t y, int16_t vx, int16_t vy)
{
    for (int16_t i = 0; i < pool_size; i++) {
        if (bullet_pool[i].is_used == 0) {
            bullet_pool[i].is_used = 1;
            bullet_pool[i].x       = x;
            bullet_pool[i].y       = y;
            bullet_pool[i].v_x     = vx;
            bullet_pool[i].v_y     = vy;
            bullet_pool[i].type    = type;
            bullet_pool[i].direct  = 3;
            return &bullet_pool[i];
        }
    }
    return 0;
}
//create bullets with v = -30, 0
struct Bullet *create_bullet(struct Bullet *bullet_pool, int16_t pool_size,
                             enum Bullet_types type, int16_t x, int16_t y)
{
    return create_bullet_v(bullet_pool, pool_size, type, x, y, -30, 0);
}
//create seek bullets
struct Bullet *create_sbullet(struct Bullet *sbullet_pool, struct Enemy **seek_objects,
                              struct Enemy *enemey_pool, int16_t x, int16_t y)
{
    for (int i = 0; i < SBULLET_POOL; i++) {
        if (sbullet_pool[i].is_used == 0) {
            sbullet_pool[i].is_used = 1;
            sbullet_pool[i].x       = x;
            sbullet_pool[i].y       = y;
            sbullet_pool[i].type    = B_SEEK;
            seek_objects[i]         = 0;
            for (int j = 0; j < ENEMY_POOL; j++) {
                if (enemey_pool[j].is_used) {
                    seek_objects[i] = &enemey_pool[j];
                    break;
                }
            }
            if (seek_objects[i] == 0)
                seek_objects[i] = &safe_e;
            return sbullet_pool + i;
        }
    }
    return 0;
}
//create mine
struct Mine *create_mine(struct Mine *mine_pool, int16_t x, int16_t y)
{
    for (int i = 0; i < MINE_POOL; i++) {
        if (!mine_pool[i].is_used) {
            mine_pool[i].is_used = 1;
            mine_pool[i].x = x;
            mine_pool[i].y = y;
            return &mine_pool[i];
        }
    }
    return 0;
}

