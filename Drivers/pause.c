#include "pause.h"
#include "audio.h"
#include <string.h>

static lv_obj_t * win = NULL;
static lv_obj_t * mask = NULL;
static lv_obj_t * label_value;
static lv_obj_t * slider_obj = NULL;
static int32_t slider_value = 50;
static uint8_t pause_flag = 0;
static uint8_t return_to_level_select_flag = 0;

static uint8_t pause_open_request = 0;
static char last_filename[256] = {0};
static uint8_t need_restore_audio = 0;

static void close_pause_window(uint8_t save)
{
    if (save) {
        SD_Write(slider_value);
    }
    if (need_restore_audio && last_filename[0]) {
        audio_play(last_filename);
        need_restore_audio = 0;
    }
    pause_flag = 0;
    if (mask) {
        lv_obj_del(mask);
        mask = NULL;
        win = NULL;
        slider_obj = NULL;
        label_value = NULL;
    }
}

static void continue_btn_event_cb(lv_event_t * e)
{
    (void)e;
    close_pause_window(1);
}

static void return_btn_event_cb(lv_event_t * e)
{
    (void)e;
    return_to_level_select_flag = 1;
    close_pause_window(1);
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    slider_value = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label_value, "Volume: %d", slider_value);
}

//暂停窗口
static void open_pause(void)
{
    if (lv_scr_act() != display_game) return;
    if (win != NULL) return;
    if (audio_is_playing()) {
        const char* cur = audio_get_current_filename();
        if (cur && cur[0]) {
            strncpy(last_filename, cur, sizeof(last_filename)-1);
            last_filename[sizeof(last_filename)-1] = '\0';
            need_restore_audio = 1;
        }
        audio_stop();
    } else {
        need_restore_audio = 0;
    }

    pause_flag = 1;
    mask = lv_obj_create(display_game);
    lv_obj_set_size(mask, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(mask, 0, 0);
    lv_obj_set_style_bg_color(mask, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(mask, LV_OPA_50, 0);
    lv_obj_add_flag(mask, LV_OBJ_FLAG_CLICKABLE);

    win = lv_win_create(mask, 40);
    lv_obj_set_size(win, 600, 400);
    lv_obj_center(win);

    lv_obj_t * header = lv_win_get_header(win);
    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "PAUSE");
    lv_obj_set_width(title, LV_PCT(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t * cont = lv_win_get_content(win);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 20, 0);

    slider_obj = lv_slider_create(cont);
    lv_obj_set_width(slider_obj, 300);
    lv_slider_set_range(slider_obj, 0, 100);
    slider_value = SD_Read();
    lv_slider_set_value(slider_obj, slider_value, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_obj, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    label_value = lv_label_create(cont);
    lv_label_set_text_fmt(label_value, "Volume: %d", slider_value);

    lv_obj_t * btn_cont = lv_obj_create(cont);
    lv_obj_set_size(btn_cont, LV_PCT(80), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn_cont, 20, 0);

    lv_obj_t * continue_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(continue_btn, 120, 50);
    lv_obj_t * continue_label = lv_label_create(continue_btn);
    lv_label_set_text(continue_label, "Continue");
    lv_obj_center(continue_label);
    lv_obj_add_event_cb(continue_btn, continue_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * return_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(return_btn, 120, 50);
    lv_obj_t * return_label = lv_label_create(return_btn);
    lv_label_set_text(return_label, "Return");
    lv_obj_center(return_label);
    lv_obj_add_event_cb(return_btn, return_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

void PB2_Key_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SYSCFG);
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_2);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN2);
    exti_init(EXTI_2, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_2);
    nvic_irq_enable(EXTI2_IRQn, 2, 0);
}

//中断设置标志位
void EXTI2_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_2) != RESET) {
        pause_open_request = 1;
        exti_interrupt_flag_clear(EXTI_2);
    }
}

uint8_t Pause_IsActive(void)
{
    return pause_flag;
}

uint8_t Pause_ShouldReturnToMenu(void)
{
    return return_to_level_select_flag;
}

void Pause_ClearReturnFlag(void)
{
    return_to_level_select_flag = 0;
}

void Pause_HandleRequest(void)
{
    if (pause_open_request) {
        pause_open_request = 0;
        open_pause();
    }
}

void switch_to_level_select(void)
{
    display_switch_to_play();
}
