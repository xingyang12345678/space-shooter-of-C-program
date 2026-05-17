#include "audio.h"
#include "SD_rw.h"
#include <string.h>

#define AUDIO_BUF_SIZE 1024

static uint16_t audio_buf[AUDIO_BUF_SIZE];
static uint32_t buf_index = 0;
static uint32_t buf_len = 0;

static FIL file;
static UINT br;

static uint8_t audio_play_flag = 0;
static uint8_t loop_mode = 1;
static uint8_t volume_percent = 50;

//保存当前播放的文件名
static char current_audio_file[256] = {0};

//I2S初始化
void i2s_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SPI1);

    gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_12 | GPIO_PIN_13);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12 | GPIO_PIN_13);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_12 | GPIO_PIN_13);

    gpio_af_set(GPIOC, GPIO_AF_5, GPIO_PIN_1);
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_1);

    spi_i2s_deinit(SPI1);

    i2s_psc_config(SPI1, I2S_AUDIOSAMPLE_44K, I2S_FRAMEFORMAT_DT16B_CH16B, I2S_MCKOUT_DISABLE);
    i2s_init(SPI1, I2S_MODE_MASTERTX, I2S_STD_PHILIPS, I2S_CKPL_LOW);
    i2s_enable(SPI1);
    spi_master_transfer_start(SPI1, SPI_TRANS_START);
    spi_i2s_interrupt_enable(SPI1, SPI_I2S_INT_TP);
    nvic_irq_enable(SPI1_IRQn, 1, 0);
}

void audio_set_volume(uint8_t vol)
{
    if (vol > 100) vol = 100;
    volume_percent = vol;
}

void audio_set_loop_mode(uint8_t loop)
{
    loop_mode = (loop != 0) ? 1 : 0;
}

uint8_t audio_is_playing(void)
{
    return audio_play_flag;
}

void audio_play(char *filename)
{
    FRESULT res;
    if (audio_play_flag) {
        f_close(&file);
    }

    // 保存文件名，用于后续恢复
    strncpy(current_audio_file, filename, sizeof(current_audio_file) - 1);
    current_audio_file[sizeof(current_audio_file) - 1] = '\0';

    volume_percent = SD_Read();

    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK)
        return;

    buf_index = 0;
    buf_len = 0;
    audio_play_flag = 1;
}

void audio_stop(void)
{
    audio_play_flag = 0;
    f_close(&file);
}

//获取当前文件名
const char* audio_get_current_filename(void)
{
    return current_audio_file;
}

static void audio_load_data(void)
{
    if (buf_index >= buf_len) {
        f_read(&file, audio_buf, AUDIO_BUF_SIZE * 2, &br);
        buf_len = br / 2;
        buf_index = 0;

        if (br == 0) {
            if (loop_mode) {
                f_lseek(&file, 0);
            } else {
                audio_stop();
            }
        }
    }
}

static int16_t apply_volume(int16_t sample, uint8_t vol)
{
    if (vol == 100) return sample;
    int32_t scaled = (int32_t)sample * vol / 100;
    if (scaled > 32767) scaled = 32767;
    if (scaled < -32768) scaled = -32768;
    return (int16_t)scaled;
}

void SPI1_IRQHandler(void)
{
    if (spi_i2s_interrupt_flag_get(SPI1, SPI_I2S_INT_FLAG_TP)) {
        if (audio_play_flag == 0) {
            spi_i2s_data_transmit(SPI1, 0);
            return;
        }

        audio_load_data();

        if (audio_play_flag == 0) {
            spi_i2s_data_transmit(SPI1, 0);
            return;
        }

        int16_t sample = (int16_t)audio_buf[buf_index++];
        sample = apply_volume(sample, volume_percent);
        spi_i2s_data_transmit(SPI1, (uint16_t)sample);
    }
}
