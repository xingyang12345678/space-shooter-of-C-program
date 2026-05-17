#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "lvgl.h"
#include "drivers.h"
#include "SD_rw.h"
#include "audio.h"
#include "key.h"
#include "joystick.h"
#include "pause.h"
#include <string.h>
#include "battle.h"
extern lv_obj_t *display_menu;
extern lv_obj_t *display_play;
extern lv_obj_t *display_game;
void display_manager_init(void);
void display_manager_deinit(void);
void display_switch_to_menu(void);
void display_switch_to_play(void);
void display_switch_to_game(void);
#endif
