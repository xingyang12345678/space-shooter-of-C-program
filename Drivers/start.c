#include "battle.h"
#include <stdio.h>
#include "key.h"
#include "pause.h"
#include "display_manager.h"
uint8_t battle_start()
{
		dispatch_cursor = 0;
    uint8_t *enemy_buffer   = sdram_malloc(E_BUF_SIZE);
    uint8_t *player_buffer  = sdram_malloc(P_BUF_SIZE);
    uint8_t *player_shield_buffer = sdram_malloc(PS_BUF_SIZE);
    uint8_t *seek_buffer    = sdram_malloc((50 * 50 * 2 + 4) * 12);
    uint8_t *mine_buffer    = sdram_malloc(MINE_BUF_SIZE);
    uint8_t *e1_shield_buf  = sdram_malloc(E1_BUF_SIZE);
    uint8_t *e1_plain_buf   = sdram_malloc(E1_BUF_SIZE);
    uint8_t *e2_ram_buf     = sdram_malloc(E2_BUF_SIZE);
    uint8_t *e3_carrier_buf = sdram_malloc(E3_BUF_SIZE);
    uint8_t *e3_minion_buf  = sdram_malloc(E3M_BUF_SIZE);
    uint8_t *boss1_buf      = sdram_malloc(BOSS1_BUF_SIZE);
    uint8_t *boss2_buf      = sdram_malloc(BOSS2_BUF_SIZE);
    uint8_t *boss3_buf      = sdram_malloc(BOSS3_BUF_SIZE);
    uint8_t *boss4_buf      = sdram_malloc(BOSS4_BUF_SIZE);
    enemy_num = 0;
    player_mana = config.max_mana;
    player_shield_timer = 0;
    player_score = 0;
    round_timer = 0;
    current_boss = 0;
    void *buf = sdram_malloc(1024 * 600 * 2);

    lv_color_t red   = lv_color_make(255, 10, 10);
    lv_color_t blue  = lv_color_make(10, 10, 255);
    lv_color_t green = lv_color_make(10, 255, 10);
    lv_draw_rect_dsc_t circle_dsc;
    lv_draw_rect_dsc_init(&circle_dsc);
    circle_dsc.bg_color  = blue;
    circle_dsc.bg_opa    = LV_OPA_COVER;
    circle_dsc.radius    = LV_RADIUS_CIRCLE;

    lv_draw_rect_dsc_t ebullet_dsc;
    lv_draw_rect_dsc_init(&ebullet_dsc);
    ebullet_dsc.bg_color = green;
    ebullet_dsc.bg_opa   = LV_OPA_COVER;
    ebullet_dsc.radius   = LV_RADIUS_CIRCLE;

    read_file_to_array("0:/player.bin", player_buffer, P_BUF_SIZE);
    lv_img_dsc_t player_struct;
    player_struct.header.always_zero = 0;
    player_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    player_struct.header.w           = P_W;
    player_struct.header.h           = P_H;
    player_struct.header.reserved    = 0;
    player_struct.data_size          = P_W * P_H * 2;
    player_struct.data               = player_buffer + 4;

    read_file_to_array("0:/player_plane_shielded.bin", player_shield_buffer, PS_BUF_SIZE);
    lv_img_dsc_t player_shield_struct;
    player_shield_struct.header.always_zero = 0;
    player_shield_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    player_shield_struct.header.w           = PS_W;
    player_shield_struct.header.h           = PS_H;
    player_shield_struct.header.reserved    = 0;
    player_shield_struct.data_size          = PS_W * PS_H * 2;
    player_shield_struct.data               = player_shield_buffer + 4;

    read_file_to_array("0:/air_mine.bin", mine_buffer, MINE_BUF_SIZE);
    lv_img_dsc_t mine_struct;
    mine_struct.header.always_zero = 0;
    mine_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    mine_struct.header.w           = MINE_W;
    mine_struct.header.h           = MINE_H;
    mine_struct.header.reserved    = 0;
    mine_struct.data_size          = MINE_W * MINE_H * 2;
    mine_struct.data               = mine_buffer + 4;

    read_file_to_array("0:/enemy.bin", enemy_buffer, E_BUF_SIZE);
    lv_img_dsc_t enemy_struct;
    enemy_struct.header.always_zero = 0;
    enemy_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    enemy_struct.header.w           = E_W;
    enemy_struct.header.h           = E_H;
    enemy_struct.header.reserved    = 0;
    enemy_struct.data_size          = E_W * E_H * 2;
    enemy_struct.data               = enemy_buffer + 4;

    read_file_to_array("0:/enemy1_shield.bin", e1_shield_buf, E1_BUF_SIZE);
    lv_img_dsc_t e1_shield_struct;
    e1_shield_struct.header.always_zero = 0;
    e1_shield_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    e1_shield_struct.header.w           = E1_W;
    e1_shield_struct.header.h           = E1_H;
    e1_shield_struct.header.reserved    = 0;
    e1_shield_struct.data_size          = E1_W * E1_H * 2;
    e1_shield_struct.data               = e1_shield_buf + 4;

    read_file_to_array("0:/enemy1_plain.bin", e1_plain_buf, E1_BUF_SIZE);
    lv_img_dsc_t e1_plain_struct;
    e1_plain_struct.header.always_zero = 0;
    e1_plain_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    e1_plain_struct.header.w           = E1_W;
    e1_plain_struct.header.h           = E1_H;
    e1_plain_struct.header.reserved    = 0;
    e1_plain_struct.data_size          = E1_W * E1_H * 2;
    e1_plain_struct.data               = e1_plain_buf + 4;

    read_file_to_array("0:/enemy2_ram.bin", e2_ram_buf, E2_BUF_SIZE);
    lv_img_dsc_t e2_ram_struct;
    e2_ram_struct.header.always_zero = 0;
    e2_ram_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    e2_ram_struct.header.w           = E2_W;
    e2_ram_struct.header.h           = E2_H;
    e2_ram_struct.header.reserved    = 0;
    e2_ram_struct.data_size          = E2_W * E2_H * 2;
    e2_ram_struct.data               = e2_ram_buf + 4;

    read_file_to_array("0:/enemy3_carrier.bin", e3_carrier_buf, E3_BUF_SIZE);
    lv_img_dsc_t e3_carrier_struct;
    e3_carrier_struct.header.always_zero = 0;
    e3_carrier_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    e3_carrier_struct.header.w           = E3_W;
    e3_carrier_struct.header.h           = E3_H;
    e3_carrier_struct.header.reserved    = 0;
    e3_carrier_struct.data_size          = E3_W * E3_H * 2;
    e3_carrier_struct.data               = e3_carrier_buf + 4;

    read_file_to_array("0:/enemy3_minion.bin", e3_minion_buf, E3M_BUF_SIZE);
    lv_img_dsc_t e3_minion_struct;
    e3_minion_struct.header.always_zero = 0;
    e3_minion_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    e3_minion_struct.header.w           = E3M_W;
    e3_minion_struct.header.h           = E3M_H;
    e3_minion_struct.header.reserved    = 0;
    e3_minion_struct.data_size          = E3M_W * E3M_H * 2;
    e3_minion_struct.data               = e3_minion_buf + 4;

    read_file_to_array("0:/boss_01_cruiser_300x151.bin", boss1_buf, BOSS1_BUF_SIZE);
    lv_img_dsc_t boss1_struct;
    boss1_struct.header.always_zero = 0;
    boss1_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    boss1_struct.header.w           = BOSS1_W;
    boss1_struct.header.h           = BOSS1_H;
    boss1_struct.header.reserved    = 0;
    boss1_struct.data_size          = BOSS1_W * BOSS1_H * 2;
    boss1_struct.data               = boss1_buf + 4;

    read_file_to_array("0:/boss_02_hive_300x206.bin", boss2_buf, BOSS2_BUF_SIZE);
    lv_img_dsc_t boss2_struct;
    boss2_struct.header.always_zero = 0;
    boss2_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    boss2_struct.header.w           = BOSS2_W;
    boss2_struct.header.h           = BOSS2_H;
    boss2_struct.header.reserved    = 0;
    boss2_struct.data_size          = BOSS2_W * BOSS2_H * 2;
    boss2_struct.data               = boss2_buf + 4;

    read_file_to_array("0:/boss_03_dragon_300x164.bin", boss3_buf, BOSS3_BUF_SIZE);
    lv_img_dsc_t boss3_struct;
    boss3_struct.header.always_zero = 0;
    boss3_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    boss3_struct.header.w           = BOSS3_W;
    boss3_struct.header.h           = BOSS3_H;
    boss3_struct.header.reserved    = 0;
    boss3_struct.data_size          = BOSS3_W * BOSS3_H * 2;
    boss3_struct.data               = boss3_buf + 4;

    read_file_to_array("0:/boss_04_prism_300x210.bin", boss4_buf, BOSS4_BUF_SIZE);
    lv_img_dsc_t boss4_struct;
    boss4_struct.header.always_zero = 0;
    boss4_struct.header.cf          = LV_IMG_CF_TRUE_COLOR;
    boss4_struct.header.w           = BOSS4_W;
    boss4_struct.header.h           = BOSS4_H;
    boss4_struct.header.reserved    = 0;
    boss4_struct.data_size          = BOSS4_W * BOSS4_H * 2;
    boss4_struct.data               = boss4_buf + 4;

    lv_img_dsc_t seek_struct[12];
    for (int i = 0; i < 12; i++) {
        char name[24];
        sprintf(name, "0:/missile_%d.bin", i);
        read_file_to_array(name, seek_buffer + (50 * 50 * 2 + 4) * i,
                           50 * 50 * 2 + 4);
        seek_struct[i].header.always_zero = 0;
        seek_struct[i].header.cf          = LV_IMG_CF_TRUE_COLOR;
        seek_struct[i].header.w           = 50;
        seek_struct[i].header.h           = 50;
        seek_struct[i].header.reserved    = 0;
        seek_struct[i].data_size          = 50 * 50 * 2;
        seek_struct[i].data               = seek_buffer + (50 * 50 * 2 + 4) * i + 4;
    }

    lv_obj_t *m_canva = lv_canvas_create(display_game);
    lv_obj_set_width(m_canva, 1024);
    lv_obj_set_height(m_canva, 600);
    lv_obj_set_pos(m_canva, 0, 0);

    lv_canvas_set_buffer(m_canva, buf, 1024, 600, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(m_canva, lv_color_black(), LV_OPA_COVER);

    struct Enemy  enemy_pool[ENEMY_POOL];
    struct Bullet bullet_pool[BULLET_POOL];
    struct Bullet ebullet_pool[EBULLET_POOL];
    struct Bullet sbullet_pool[SBULLET_POOL];
    struct Mine   mine_pool[MINE_POOL];
    struct Enemy *seek_objects[SBULLET_POOL];
    for (int16_t i = 0; i < ENEMY_POOL; i++)
        enemy_pool[i].is_used = 0;
    for (int16_t i = 0; i < BULLET_POOL; i++)
        bullet_pool[i].is_used = 0;
    for (int16_t i = 0; i < EBULLET_POOL; i++)
        ebullet_pool[i].is_used = 0;
    for (int16_t i = 0; i < SBULLET_POOL; i++)
        sbullet_pool[i].is_used = 0;
    for (int16_t i = 0; i < MINE_POOL; i++)
        mine_pool[i].is_used = 0;
    for (int16_t i = 0; i < SBULLET_POOL; i++)
        seek_objects[i] = 0;
    struct Player player;
    player.x      = 200;
    player.y      = 300;
    player.v_x    = 0;
    player.v_y    = 0;
    player.health = 200;
    static uint32_t last_update = 0;
    uint32_t        now         = lv_tick_get();
    init_pre_bullet();
    init_pre_ebullet();
    init_pre_enemy();
    init_pre_seek();
    init_pre_mine();
    lv_obj_t *btn = lv_btn_create(display_game);
    lv_obj_set_pos(btn, 0, 0);
    lv_obj_set_width(btn, 80);
    lv_obj_set_height(btn, 30);
    lv_obj_t *score_label = lv_label_create(btn);
    lv_obj_set_style_text_color(score_label, lv_color_white(), 0);

    lv_obj_t *boss_btn = lv_btn_create(display_game);
    lv_obj_set_pos(boss_btn, 780, 0);
    lv_obj_set_width(boss_btn, 240);
    lv_obj_set_height(boss_btn, 30);
    lv_obj_t *boss_hp_label = lv_label_create(boss_btn);
    lv_obj_set_style_text_color(boss_hp_label, lv_color_white(), 0);
    lv_obj_add_flag(boss_btn, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *health_btn = lv_btn_create(display_game);
    lv_obj_set_pos(health_btn, 90, 0);
    lv_obj_set_width(health_btn, 120);
    lv_obj_set_height(health_btn, 30);
    lv_obj_t *health_label = lv_label_create(health_btn);
    lv_obj_set_style_text_color(health_label, lv_color_white(), 0);

    lv_obj_t *enemy_btn = lv_btn_create(display_game);
    lv_obj_set_pos(enemy_btn, 220, 0);
    lv_obj_set_width(enemy_btn, 120);
    lv_obj_set_height(enemy_btn, 30);
    lv_obj_t *enemy_label = lv_label_create(enemy_btn);
    lv_obj_set_style_text_color(enemy_label, lv_color_white(), 0);

    lv_obj_t *mana_btn = lv_btn_create(display_game);
    lv_obj_set_pos(mana_btn, 350, 0);
    lv_obj_set_width(mana_btn, 120);
    lv_obj_set_height(mana_btn, 30);
    lv_obj_t *mana_label = lv_label_create(mana_btn);
    lv_obj_set_style_text_color(mana_label, lv_color_white(), 0);
    joystick_init();
    display_switch_to_game();
		key_init();
    is_over = 0;
    while (1) {
        Pause_HandleRequest();
        if (Pause_IsActive()) {
            lv_timer_handler();
            delay_us(5000);
            continue;
        }
				key_value = key_read();
        control_x = read_stick(joystick_read_x());
        control_y = read_stick(joystick_read_y());
        now       = lv_tick_get();
        if (now - last_update > 80) {
					  last_update = now;
            lv_label_set_text_fmt(score_label, "score:%d", (int)player_score);
            lv_label_set_text_fmt(health_label, "health:%d", player.health);
            lv_label_set_text_fmt(enemy_label, "enemy:%d", enemy_num);
            lv_label_set_text_fmt(mana_label, "mana:%d", player_mana);
            if (current_boss != 0 && current_boss->is_used) {
                lv_label_set_text_fmt(boss_hp_label, "BOSS HP: %d/%d",
                                      current_boss->health,
                                      current_boss->max_health);
                lv_obj_clear_flag(boss_btn, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(boss_btn, LV_OBJ_FLAG_HIDDEN);
            }
            update_mana();
            enemy_dispatcher( enemy_pool);
            collision_detect(&player, enemy_pool, bullet_pool, ebullet_pool,
                           sbullet_pool, mine_pool);
            update_player(&player, bullet_pool, sbullet_pool, seek_objects, enemy_pool,
                          mine_pool);
            update_sbullet(sbullet_pool, seek_objects, enemy_pool);
            update_enemy(enemy_pool, ebullet_pool);
            update_bullet_norm(bullet_pool);
            update_ebullet_norm(ebullet_pool);
            render(m_canva, &player, enemy_pool, bullet_pool, ebullet_pool, sbullet_pool,
                   mine_pool, &player_struct, &player_shield_struct,
                   &circle_dsc, &ebullet_dsc, &enemy_struct, seek_struct,
                   &mine_struct,
                   &e1_shield_struct, &e1_plain_struct, &e2_ram_struct,
                   &e3_carrier_struct, &e3_minion_struct,
                   &boss1_struct, &boss2_struct,
                   &boss3_struct, &boss4_struct);
						if(is_over)	break;
        }
        if (Pause_ShouldReturnToMenu()) {
            Pause_ClearReturnFlag();
            break;
        }

        lv_timer_handler();
        delay_us(10);
    }
		switch_to_level_select();
    sdram_free(buf);
		sdram_free(enemy_buffer);
		sdram_free(player_buffer);
		sdram_free(player_shield_buffer);
		sdram_free(seek_buffer);
		sdram_free(mine_buffer);
		sdram_free(e1_shield_buf);
		sdram_free(e1_plain_buf);
		sdram_free(e2_ram_buf);
		sdram_free(e3_carrier_buf);
		sdram_free(e3_minion_buf);
		sdram_free(boss1_buf);
		sdram_free(boss2_buf);
		sdram_free(boss3_buf);
		sdram_free(boss4_buf);
		lv_obj_clean(display_game);
		display_game = NULL;
		return is_over;
}

