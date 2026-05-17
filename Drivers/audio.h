#ifndef __AUDIO_H
#define __AUDIO_H

#include "gd32h7xx.h"
#include "ff.h"

void i2s_config(void);
void audio_play(char *filename);
void audio_stop(void);
void audio_set_volume(uint8_t vol);
void audio_set_loop_mode(uint8_t loop);
uint8_t audio_is_playing(void);

//获取当前播放的文件名（用于恢复播放）
const char* audio_get_current_filename(void);

#endif
