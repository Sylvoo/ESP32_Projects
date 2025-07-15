#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "hal/ledc_types.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include "driver/ledc.h"
#include "soc/clk_tree_defs.h"

void servoRotate(void * arg)
{
	int duty = 1638; 
	int step = 14;
	int total_cycles = 117; // bcs 14*117=1638
	bool pos_direction = true;
	int iteration_time = 10; // ms
	
	ledc_timer_config_t timer_config = {
		.duty_resolution = LEDC_TIMER_15_BIT,
		.freq_hz = 50,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.timer_num = LEDC_TIMER_0,
		.clk_cfg = LEDC_AUTO_CLK
	};
	
	ledc_timer_config(&timer_config); // aplikujemy timer config
	
	ledc_channel_config_t ledc_conf = {
		.channel = LEDC_CHANNEL_0,
		.duty = duty,
		.gpio_num = 25,
		.intr_type = LEDC_INTR_DISABLE,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.timer_sel = LEDC_TIMER_0 // musi byc ten sam co w timer_conifg 
	};
	
	ledc_channel_config(&ledc_conf); // aplikujemy channel config 
	
	int i;
	while(1)
	{
		for (i=0;i<total_cycles;i++)
		{
			pos_direction ? (duty += step) : (duty -= step);
			ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
			ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
			
			vTaskDelay(iteration_time/portTICK_PERIOD_MS);
		}
		
		pos_direction = !pos_direction;
	}
}

void app_main()
{
	xTaskCreate(&servoRotate, "servo_rotating", 2048, NULL, 5, NULL);
	
}
