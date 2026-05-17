#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "gd32h7xx.h"
#include "audio.h"
#include "joystick.h"
#include "pause.h"
#include "key.h"
#include "display_manager.h"

int main()
{
		sys_init();
		
		rcu_periph_clock_enable(RCU_GPIOA);
		gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
		gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
		gpio_bit_reset(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
		
		lv_init();
		lv_port_disp_init();
		lv_port_indev_init();

    //SD卡初始化
    SD_Init();
		
    //is2初始化
    i2s_config(); 

    //PB2初始化
    PB2_Key_Init();
    
		//界面管理初始化
    display_manager_init();

    //加载主菜单
    display_switch_to_menu();
		 
		 
		while(1){
			
				delay_us(2000);
				lv_timer_handler();

				
				if(gaming){
					if (!display_game) {
							display_game = lv_obj_create(NULL);
							srand(lv_tick_get());
							}
					config.bullet_num = 7;
					battle_start();
				}
				gaming = 0;
				
				
        lv_timer_handler();
        delay_us(5);
		}
}
