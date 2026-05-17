#ifndef TOUCH_H
#define TOUCH_H

#include "gd32h7xx.h"




typedef enum
{
    ATK_RGBLCD_TOUCH_IIC_ADDR_14 = 0x14,    /* 0x14 */
    ATK_RGBLCD_TOUCH_IIC_ADDR_5D = 0x5D,    /* 0x5D */
    ATK_RGBLCD_TOUCH_IIC_ADDR_38 = 0x38,    /* 0x38 */
} atk_rgblcd_touch_iic_addr_t;

/* 触摸点坐标数据结构 */
typedef struct
{
    uint16_t x;     /* 触摸点X坐标 */
    uint16_t y;     /* 触摸点Y坐标 */
    uint16_t size;  /* 触摸点大小 */
} lcd_touch_point_t;

/* 错误代码 */
#define ATK_RGBLCD_TOUCH_EOK                    0   /* 没有错误 */
#define ATK_RGBLCD_TOUCH_ERROR                  1   /* 错误 */

/* 操作函数 */
uint8_t atk_rgblcd_touch_init(void);                    /* RGB LCD模块触摸初始化 */
uint8_t atk_rgblcd_touch_scan(lcd_touch_point_t *point, uint8_t cnt);    /* RGB LCD模块触摸扫描 */

#endif

