#include "joystick.h"

//ADC初始化
void joystick_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_ADC2);
    adc_deinit(ADC2);
    adc_clock_config(ADC2, ADC_CLK_SYNC_HCLK_DIV16);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE,
                  GPIO_PIN_2 | GPIO_PIN_3);
    adc_resolution_config(ADC2, ADC_RESOLUTION_12B);
    adc_data_alignment_config(ADC2, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC2, ADC_REGULAR_CHANNEL, 1);
    adc_special_function_config(ADC2, ADC_SCAN_MODE, DISABLE);
    adc_special_function_config(ADC2, ADC_CONTINUOUS_MODE, DISABLE);
    adc_regular_channel_config(ADC2, 0, JS_X_CHANNEL, SQX_SMP(247));
    adc_enable(ADC2);
    delay_us(10);
    adc_calibration_enable(ADC2);
}

static uint16_t joystick_read_channel(uint32_t channel)
{
    adc_regular_channel_config(ADC2, 0, channel, SQX_SMP(247));

    adc_flag_clear(ADC2, ADC_FLAG_EOC);

    adc_software_trigger_enable(ADC2, ADC_REGULAR_CHANNEL);

    while(adc_flag_get(ADC2, ADC_FLAG_EOC) == RESET);

    return adc_regular_data_read(ADC2);
}

uint16_t joystick_read_x(void)
{
    return joystick_read_channel(JS_X_CHANNEL);
}

uint16_t joystick_read_y(void)
{
    return joystick_read_channel(JS_Y_CHANNEL);
}

void joystick_read_xy(uint16_t *x, uint16_t *y)
{
    *x = joystick_read_x();
    *y = joystick_read_y();
}
