#ifndef __SD_RW_H
#define __SD_RW_H
#include "ff.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void SD_Init(void);
void SD_Write(int32_t value);
int32_t SD_Read(void);
#endif
