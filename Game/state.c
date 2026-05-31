#include "battle.h"

struct Enemy safe_e = { 100, 100, 0, 0, 100, 0, E_NORM, 1, 0, 0, 100 };

int16_t pre_player_x = -100;
int16_t pre_player_y = -100;
int16_t pre_bullet_positions[BULLET_POOL][2];
int16_t pre_ebullet_positions[EBULLET_POOL][2];
int16_t pre_enemy[ENEMY_POOL][2];
int16_t pre_seek[SBULLET_POOL][2];
int16_t pre_mine[MINE_POOL][2];
int16_t control_x   = 0;
int16_t control_y   = 0;
int16_t player_knockback_timer = 0;
int16_t player_attack_timer = 0;
int16_t enemy_num         = 0;
int16_t dispatch_cursor   = 0;
int16_t player_mana       = 0;
int16_t player_shield_timer = 0;
int32_t player_score      = 0;
int32_t round_timer       = 0;
struct Enemy *current_boss = 0;
extern lv_obj_t *display_game;
uint8_t key_value;
uint8_t bullet_pattern = BULLET_PATTERN_STRAIGHT;
struct BattleConfig config ={
	{5, 5}, // bullet count: straight, spread
 10, //damage
 {
    {0,-12,12,-24,24,-36,36},
    {0,-12,12,-24,24,-36,36},
 },
	{
    {{30,0},{30,0},{30,0},{30,0},{30,0},{30,0},{30,0}},
    {{30,0},{30,0},{30,0},{25,10},{25,-10},{20,15},{20,-15}},
  },
	5,   //attack gap
	50,  //seek damage
	150,   // seek_cost 
	200,   // shield_cost 
	120,   // mine_cost 
	200,  // mine_damage 
	500,  // max_mana 
	2,    // mana_regen
	50,   // mana_kill_bonus 
	100,   // shield_duration 
	1500,   //player hp
	12,    // wave_count
	{
    // norm shield ram carrier  min_frames     boss 
        0,    0,     0,   0,        80,         0,
        0,    0,     0,   0,        80,         0,
        0,    0,     0,   0,        20,         1,   
        0,    0,     0,   0,        100,        2,
        0,    0,     4,   0,        100,        3,
        0,    0,     0,   0,        20,         0,   
        2,    1,     4,   0,        100,        0,
        3,    1,     2,   1,        100,        0,
        0,    0,     0,   0,        20,         0,  
        3,    1,     4,   0,        100,        0,
        1,    2,     3,   1,        100,        0,
        0,    0,     0,   0,        20,         4, 
 }
 //the max wave_count is 60
};
bool gaming = false;
uint8_t is_over = false;
