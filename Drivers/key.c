#include "key.h"

void key_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);

    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_13);

    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_5);
}

uint8_t key_read(void)
{
    static uint32_t last_tick = 0;
		static uint8_t last_key;
		uint8_t new_key = 0;
    if(gpio_input_bit_get(GPIOC, GPIO_PIN_13) == RESET)
    {
				new_key = KEY_S2;
    }

    if(gpio_input_bit_get(GPIOD, GPIO_PIN_3) == RESET)
    {
				new_key = KEY_S3;
    }

    if(gpio_input_bit_get(GPIOD, GPIO_PIN_4) == RESET)
    {
				new_key = KEY_S4;
    }

    if(gpio_input_bit_get(GPIOD, GPIO_PIN_5) == RESET)
    {
				new_key = KEY_S5;
    }
		if(lv_tick_get() - last_tick < 800 && new_key ==last_key)
				new_key = 0;
		last_key = new_key;
		last_tick = lv_tick_get();
    return new_key;
}
