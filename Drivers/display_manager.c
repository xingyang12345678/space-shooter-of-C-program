#include "display_manager.h"
lv_obj_t *display_menu = NULL;
lv_obj_t *display_play = NULL;
lv_obj_t *display_game = NULL;
static uint8_t *menu_bg_buffer = NULL;
static uint8_t *play_bg_buffer = NULL;
static lv_img_dsc_t menu_bg_dsc;
static lv_img_dsc_t play_bg_dsc;
static lv_obj_t *menu_bg_img = NULL;
static lv_obj_t *play_bg_img = NULL;

static lv_obj_t *setting_win = NULL;
static lv_obj_t *setting_mask = NULL;
static lv_obj_t *setting_label_value;
static lv_obj_t *setting_slider = NULL;
static char last_filename[256] = {0};
static uint8_t need_restore_audio_setting = 0;

static void free_menu_background(void);
static void free_play_background(void);
static void load_menu_background(void);
static void load_play_background(void);
static void create_menu_ui(void);
static void create_play_ui(void);
static void setting_slider_event_cb(lv_event_t *e);
static void setting_close_win_event_cb(lv_event_t *e);
static void setting_open_win_event_cb(lv_event_t *e);

static void free_menu_background(void) {
    if (menu_bg_buffer) {
        sdram_free(menu_bg_buffer);
        menu_bg_buffer = NULL;
    }
    if (menu_bg_img) {
        lv_obj_del(menu_bg_img);
        menu_bg_img = NULL;
    }
    memset(&menu_bg_dsc, 0, sizeof(menu_bg_dsc));
}

static void free_play_background(void) {
    if (play_bg_buffer) {
        sdram_free(play_bg_buffer);
        play_bg_buffer = NULL;
    }
    if (play_bg_img) {
        lv_obj_del(play_bg_img);
        play_bg_img = NULL;
    }
    memset(&play_bg_dsc, 0, sizeof(play_bg_dsc));
}

static void load_menu_background(void) {
    free_menu_background();

    const size_t img_size = 1024 * 600 * 3;
    const size_t file_size = img_size + 4;

    menu_bg_buffer = (uint8_t *)sdram_malloc(file_size);
    read_file_to_array("0:/menu.bin", menu_bg_buffer, file_size);

    menu_bg_dsc.header.always_zero = 0;
    menu_bg_dsc.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    menu_bg_dsc.header.w = 1024;
    menu_bg_dsc.header.h = 600;
    menu_bg_dsc.header.reserved = 0;
    menu_bg_dsc.data_size = img_size;
    menu_bg_dsc.data = menu_bg_buffer + 4;

    menu_bg_img = lv_img_create(display_menu);
    lv_img_set_src(menu_bg_img, &menu_bg_dsc);
    lv_obj_set_pos(menu_bg_img, 0, 0);
    lv_obj_set_size(menu_bg_img, 1024, 600);
}

static void load_play_background(void) {
    free_play_background();

    const size_t img_size = 1024 * 600 * 3;
    const size_t file_size = img_size + 4;

    play_bg_buffer = (uint8_t *)sdram_malloc(file_size);
    read_file_to_array("0:/play.bin", play_bg_buffer, file_size);

    play_bg_dsc.header.always_zero = 0;
    play_bg_dsc.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    play_bg_dsc.header.w = 1024;
    play_bg_dsc.header.h = 600;
    play_bg_dsc.header.reserved = 0;
    play_bg_dsc.data_size = img_size;
    play_bg_dsc.data = play_bg_buffer + 4;

    play_bg_img = lv_img_create(display_play);
    lv_img_set_src(play_bg_img, &play_bg_dsc);
    lv_obj_set_pos(play_bg_img, 0, 0);
    lv_obj_set_size(play_bg_img, 1024, 600);
}

static void setting_slider_event_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    lv_label_set_text_fmt(setting_label_value, "Value: %d", value);
}

static void setting_close_win_event_cb(lv_event_t *e) {
    (void)e;
    if (setting_mask) {
        if (setting_slider) {
            int32_t current_value = lv_slider_get_value(setting_slider);
            SD_Write(current_value);
        }
        if (need_restore_audio_setting && last_filename[0]) {
            audio_play(last_filename);
            need_restore_audio_setting = 0;
        }
        lv_obj_del(setting_mask);
        setting_mask = NULL;
        setting_win = NULL;
        setting_slider = NULL;
    }
}

static void setting_open_win_event_cb(lv_event_t *e) {
    (void)e;
    if (setting_win != NULL) return;

    if (audio_is_playing()) {
        const char *cur = audio_get_current_filename();
        if (cur && cur[0]) {
            strncpy(last_filename, cur, sizeof(last_filename)-1);
            last_filename[sizeof(last_filename)-1] = '\0';
            need_restore_audio_setting = 1;
        }
        audio_stop();
    } else {
        need_restore_audio_setting = 0;
    }

    int32_t saved_value = SD_Read();

    setting_mask = lv_obj_create(display_menu);
    lv_obj_set_size(setting_mask, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(setting_mask, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(setting_mask, LV_OPA_50, 0);

    setting_win = lv_win_create(setting_mask, 40);
    lv_obj_set_size(setting_win, 600, 400);
    lv_obj_center(setting_win);

    lv_obj_t *header = lv_win_get_header(setting_win);
    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "Setting");
    lv_obj_set_width(title, LV_PCT(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t *cont = lv_win_get_content(setting_win);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 20, 0);

    setting_slider = lv_slider_create(cont);
    lv_obj_set_width(setting_slider, 300);
    lv_slider_set_range(setting_slider, 0, 100);
    lv_slider_set_value(setting_slider, saved_value, LV_ANIM_OFF);
    lv_obj_add_event_cb(setting_slider, setting_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    setting_label_value = lv_label_create(cont);
    lv_label_set_text_fmt(setting_label_value, "Value: %d", saved_value);

    lv_obj_t *back_btn = lv_btn_create(cont);
    lv_obj_set_size(back_btn, 120, 45);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, setting_close_win_event_cb, LV_EVENT_CLICKED, NULL);
}

static void open_play_event_cb(lv_event_t *e) {
    (void)e;
    display_switch_to_play();
}

static void create_menu_ui(void) {
    lv_obj_t *play_btn = lv_btn_create(display_menu);
    lv_obj_set_size(play_btn, 200, 80);
    lv_obj_set_pos(play_btn, 162, 400);
    lv_obj_add_event_cb(play_btn, open_play_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *play_label = lv_label_create(play_btn);
    lv_label_set_text(play_label, "Play");
    lv_obj_center(play_label);

    lv_obj_t *setting_btn = lv_btn_create(display_menu);
    lv_obj_set_size(setting_btn, 200, 80);
    lv_obj_set_pos(setting_btn, 662, 400);
    lv_obj_add_event_cb(setting_btn, setting_open_win_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *setting_label = lv_label_create(setting_btn);
    lv_label_set_text(setting_label, "Setting");
    lv_obj_center(setting_label);
}

static void start_btn_event_cb(lv_event_t *e) 
{ 
    (void)e;
		gaming = true;
}


static void custom_btn_event_cb(lv_event_t *e) { (void)e; }
static void history_btn_event_cb(lv_event_t *e) { (void)e; }

static void create_play_ui(void) {
    lv_obj_t *start_btn = lv_btn_create(display_play);
    lv_obj_set_size(start_btn, 200, 80);
    lv_obj_set_pos(start_btn, 162, 400);
    lv_obj_add_event_cb(start_btn, start_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *start_label = lv_label_create(start_btn);
    lv_label_set_text(start_label, "Start");
    lv_obj_center(start_label);

    lv_obj_t *custom_btn = lv_btn_create(display_play);
    lv_obj_set_size(custom_btn, 200, 80);
    lv_obj_set_pos(custom_btn, 412, 400);
    lv_obj_add_event_cb(custom_btn, custom_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *custom_label = lv_label_create(custom_btn);
    lv_label_set_text(custom_label, "Custom");
    lv_obj_center(custom_label);

    lv_obj_t *history_btn = lv_btn_create(display_play);
    lv_obj_set_size(history_btn, 200, 80);
    lv_obj_set_pos(history_btn, 662, 400);
    lv_obj_add_event_cb(history_btn, history_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *history_label = lv_label_create(history_btn);
    lv_label_set_text(history_label, "History");
    lv_obj_center(history_label);
}

void display_manager_init(void) {
    display_menu = lv_obj_create(NULL);
    display_play = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(display_menu, lv_color_black(), 0);
    lv_obj_set_style_bg_color(display_play, lv_color_black(), 0);

    load_menu_background();
    load_play_background();

    create_menu_ui();
    create_play_ui();
}

void display_manager_deinit(void) {
    free_menu_background();
    free_play_background();
    if (display_menu) {
        lv_obj_del(display_menu);
        display_menu = NULL;
    }
    if (display_play) {
        lv_obj_del(display_play);
        display_play = NULL;
    }
}

void display_switch_to_menu(void) {
    if (display_menu) lv_scr_load(display_menu);
	  audio_set_loop_mode(1);
	  audio_play("0:/menu.pcm");
}

void display_switch_to_play(void) {
    if (display_play) lv_scr_load(display_play);
	  audio_stop();
}

void display_switch_to_game(void) {
    if (display_game) lv_scr_load(display_game);
	  audio_set_loop_mode(1);
    audio_play("0:/menu.pcm");
}
