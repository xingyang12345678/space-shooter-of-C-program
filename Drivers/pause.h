#ifndef __PAUSE_H
#define __PAUSE_H

#include "gd32h7xx.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "SD_rw.h"
#include <stdio.h>
#include "display_manager.h"
extern FATFS  fs;
extern lv_obj_t* display_play;
extern lv_obj_t* display_menu;
extern lv_obj_t* display_game;

void PB2_Key_Init(void);
uint8_t Pause_IsActive(void);
uint8_t Pause_GetMaxRenderFps(void);
uint16_t Pause_GetGameSpeedPercent(void);
uint8_t Pause_ShouldReturnToMenu(void);
void Pause_ClearReturnFlag(void);
void Pause_HandleRequest(void);

void switch_to_level_select(void);

#endif
