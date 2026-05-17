#ifndef __KEY_H
#define __KEY_H

#include "gd32h7xx.h"
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
void key_init(void);
uint8_t key_read(void);

#define KEY_S2   1
#define KEY_S3   2
#define KEY_S4   3
#define KEY_S5   4

#endif
