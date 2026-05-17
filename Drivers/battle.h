#ifndef FORMAT_BATTLE_H
#define FORMAT_BATTLE_H

#include "drivers.h"
#include "joystick.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include <stdlib.h>

#define ENEMY_POOL   20
#define BULLET_POOL  60
#define EBULLET_POOL 120
#define SBULLET_POOL 10
#define P_W          96
#define P_H          33
#define P_BUF_SIZE   (P_W * P_H * 2 + 4)
#define E_W          64
#define E_H          26
#define E_BUF_SIZE   (E_W * E_H * 2 + 4)

#define E1_W         120
#define E1_H         60
#define E1_BUF_SIZE  (E1_W * E1_H * 2 + 4)

#define E2_W         120
#define E2_H         60
#define E2_BUF_SIZE  (E2_W * E2_H * 2 + 4)

#define E3_W         120
#define E3_H         60
#define E3_BUF_SIZE  (E3_W * E3_H * 2 + 4)

#define E3M_W        40
#define E3M_H        22
#define E3M_BUF_SIZE (E3M_W * E3M_H * 2 + 4)

#define SHIELD_HP    80
#define SHIELD_BURST 8
#define MINE_POOL    10
#define MINE_W       80
#define MINE_H       80
#define MINE_BUF_SIZE (MINE_W * MINE_H * 2 + 4)
#define PS_W         96
#define PS_H         33
#define PS_BUF_SIZE  (PS_W * PS_H * 2 + 4)

/* boss sprites: name suffix is pixel size */
#define BOSS1_W      300
#define BOSS1_H      151
#define BOSS1_BUF_SIZE (BOSS1_W * BOSS1_H * 2 + 4)
#define BOSS2_W      300
#define BOSS2_H      206
#define BOSS2_BUF_SIZE (BOSS2_W * BOSS2_H * 2 + 4)
#define BOSS3_W      300
#define BOSS3_H      164
#define BOSS3_BUF_SIZE (BOSS3_W * BOSS3_H * 2 + 4)
#define BOSS4_W      300
#define BOSS4_H      210
#define BOSS4_BUF_SIZE (BOSS4_W * BOSS4_H * 2 + 4)

#define BOSS4_CHARGES 3

#define DISPATCH_ROWS 12
#define DISPATCH_COLS 6
#define DISPATCH_LEN  (DISPATCH_ROWS * DISPATCH_COLS)

struct BattleConfig{
	uint16_t bullet_num;
	uint16_t bullet_damage;
	uint16_t bullet_distirbution[7];
	int16_t bullet_v[7][2];
	uint16_t attack_gap;
	uint16_t seek_damage;
	uint16_t seek_cost;
	uint16_t shield_cost;
	uint16_t mine_cost;
	uint16_t mine_damage;
	uint16_t max_mana;
	uint16_t mana_regen;
	uint16_t mana_kill_bonus;
	uint16_t shield_duration;
	uint16_t player_hp;
	int16_t enemy_dispatch[DISPATCH_LEN];
};
enum Enemy_types {
    E_NORM,
    E_SHIELD,
    E_RAM,
    E_CARRIER,
    E_MINION,
    E_BOSS1,
    E_BOSS2,
    E_BOSS3,
    E_BOSS4
};

enum Bullet_types {
    B_NORM,
    B_SEEK
};

struct Player {
    int16_t x;
    int16_t y;
    int16_t v_x;
    int16_t v_y;
    int16_t health;
};

struct Enemy {
    int16_t           x;
    int16_t           y;
    int16_t           v_x;
    int16_t           v_y;
    int16_t           health;
    int16_t           frame;
    enum Enemy_types  type;
    bool              is_used;
    int16_t           shield_hp;
    int16_t           spawn_timer;
    int16_t           max_health;
};

struct Bullet {
    int16_t            x;
    int16_t            y;
    int16_t            v_x;
    int16_t            v_y;
    enum Bullet_types  type;
    bool               is_used;
    uint8_t            direct;
};

struct Mine {
    int16_t x;
    int16_t y;
    bool    is_used;
};

extern struct Enemy safe_e;

extern struct BattleConfig config;

extern uint8_t is_over;

extern int16_t pre_player_x;
extern int16_t pre_player_y;
extern int16_t pre_bullet_positions[BULLET_POOL][2];
extern int16_t pre_ebullet_positions[EBULLET_POOL][2];
extern int16_t pre_enemy[ENEMY_POOL][2];
extern int16_t pre_seek[SBULLET_POOL][2];
extern int16_t pre_mine[MINE_POOL][2];

extern int16_t control_x;
extern int16_t control_y;
extern uint8_t key_value;

extern int16_t enemy_num;
extern int16_t dispatch_cursor;
extern int16_t player_mana;
extern int16_t player_shield_timer;
extern int32_t player_score;
extern int32_t round_timer;
extern struct Enemy *current_boss;
extern lv_obj_t *display_game;
extern bool gaming;

void init_pre_bullet(void);
void init_pre_ebullet(void);
void init_pre_enemy(void);
void init_pre_seek(void);
void init_pre_mine(void);

void clear_area(lv_obj_t *canvas, int16_t x, int16_t y, int16_t w, int16_t h);
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
            lv_img_dsc_t *boss3_dsc, lv_img_dsc_t *boss4_dsc);

int16_t read_stick(uint16_t x);

void collision_detect(struct Player *player, struct Enemy *enemy_pool,
                      struct Bullet *bullet_pool, struct Bullet *ebullet_pool,
                      struct Bullet *sbullet_pool, struct Mine *mine_pool);

void create_mine(struct Mine *mine_pool, int16_t x, int16_t y);
void update_mana(void);

int16_t is_in(int16_t x, int16_t y, int16_t w, int16_t h,
              int16_t b_x, int16_t b_y, int16_t b_w, int16_t b_h);

uint8_t seek_direct(int16_t a, int16_t b, int16_t x, int16_t y);

void update_bullet_seek(struct Bullet *bullet);

void update_bullet_norm(struct Bullet *bullet_pool);
void update_ebullet_norm(struct Bullet *bullet_pool);

struct Enemy *create_enemy(struct Enemy *enemy_pool, enum Enemy_types type,
                           int16_t x, int16_t y);
struct Bullet *create_bullet(struct Bullet *bullet_pool, int16_t pool_size,
                             enum Bullet_types type, int16_t x, int16_t y);
struct Bullet *create_bullet_v(struct Bullet *bullet_pool, int16_t pool_size,
                               enum Bullet_types type,
                               int16_t x, int16_t y, int16_t vx, int16_t vy);
struct Bullet *create_sbullet(struct Bullet *sbullet_pool,
                              struct Enemy **seek_objects, struct Enemy *enemey_pool,
                              int16_t x, int16_t y);

void update_sbullet(struct Bullet *sbullet_pool, struct Enemy **seek_objects,
                    struct Enemy *enemy_pool);

void update_player(struct Player *player, struct Bullet *bullet_pool,
                   struct Bullet *sbullet_pool, struct Enemy **seek_objects,
                   struct Enemy *enemy_pool, struct Mine *mine_pool);

void update_enemy(struct Enemy *enemy, struct Bullet *ebullet_pool);

void enemy_dispatcher( struct Enemy *enemy_pool);

int16_t enemy_w(struct Enemy *e);
int16_t enemy_h(struct Enemy *e);

uint8_t battle_start();

#endif



