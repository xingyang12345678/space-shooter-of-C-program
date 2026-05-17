#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "gd32h7xx.h"
#include "gd32h7xx_adc.h"


#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"

#define JS_X_CHANNEL ADC_CHANNEL_1
#define JS_Y_CHANNEL ADC_CHANNEL_0

void joystick_init(void);

uint16_t joystick_read_x(void);
uint16_t joystick_read_y(void);

void joystick_read_xy(uint16_t *x, uint16_t *y);




extern lv_obj_t* display_play;
#endif
