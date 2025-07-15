/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "led_strip.h"
#include "portmacro.h"
#include "sdkconfig.h"

void app_main(void)
{
	// LED = GPIO02 ESP32
	// Button GPIO04
	gpio_reset_pin(2);
	gpio_reset_pin(4);
	gpio_reset_pin(22);
	
	gpio_set_direction(2, GPIO_MODE_OUTPUT);
	gpio_set_direction(22, GPIO_MODE_OUTPUT);
	gpio_set_direction(4, GPIO_MODE_INPUT);
	
	gpio_pullup_en(4);
	
	while(1)
	{
		if(gpio_get_level(4) == 0)
		{
			gpio_set_level(2, 1); // turn on
			gpio_set_level(22, 1); 
		}
		else 
		{
			gpio_set_level(2,0); // turn off
			gpio_set_level(22, 0); 
		}
	}
	
}
