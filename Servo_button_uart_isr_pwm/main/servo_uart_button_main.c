#include <stdint.h>
#include <stdio.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_attr.h"
#include "esp_eth_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "hal/gpio_types.h"
#include "hal/ledc_types.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include "driver/ledc.h"
#include "soc/clk_tree_defs.h"

#define SERVO_GPIO 25
#define BUTTON_GPIO 4
#define UART_PORT UART_NUM_0
#define BUF_SIZE 1024

QueueHandle_t uart_queue;
volatile bool paused = false;
int duty = 2457; // srodek czyli 1.5ms na 50hz czyli 20ms 

void servo_set_duty(int value)
{
	ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, value);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

void IRAM_ATTR button_isr_handler(void* arg)
{
	paused = !paused;
}

void uart(void* arg)
{
	uint8_t data[BUF_SIZE];
	while(1)
	{
		int len = uart_read_bytes(UART_PORT, data, BUF_SIZE, pdMS_TO_TICKS(100));
		if(len>0) 
		{
			for (int i = 0; i<len; i++)
			{
				xQueueSend(uart_queue, &data[i] , portMAX_DELAY);
			}
		}
	}
}

void servo(void * arg)
{
	uint8_t cmd;
	while(1)
	{
		if(xQueueReceive(uart_queue, &cmd, portMAX_DELAY))
		{
			if (paused) continue;
			if ((cmd == 'l' || cmd == 'L') && duty < 3276) duty += 100; // w lewo i maksymalnie do 2ms wypelnienia
			else if ((cmd == 'h' || cmd == 'H') && duty > 1638) duty -= 100; // w prawo 
			
			servo_set_duty(duty);
			printf("DUTY: %d\n", duty);	
		}
	}
}

void app_main()
{
	uart_config_t uart_conft = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(UART_PORT, &uart_conft);
	uart_driver_install(UART_PORT, BUF_SIZE*2,0 ,0 ,NULL, 0);
	
	ledc_timer_config_t timer = {
		.duty_resolution = LEDC_TIMER_15_BIT,
		.freq_hz = 50,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.timer_num = LEDC_TIMER_0,
		.clk_cfg = LEDC_AUTO_CLK
	};
	ledc_timer_config(&timer);
	
	ledc_channel_config_t channel = {
		.channel = LEDC_CHANNEL_0,
		.duty = duty,
		.gpio_num = SERVO_GPIO,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.hpoint = 0,
		.timer_sel = LEDC_TIMER_0
	};
	ledc_channel_config(&channel);
	servo_set_duty(duty);
	vTaskDelay(pdMS_TO_TICKS(50));
	
	gpio_config_t g_conf = {
		.intr_type = GPIO_INTR_POSEDGE,
		.mode = GPIO_MODE_INPUT,
		.pin_bit_mask = (1ULL << BUTTON_GPIO),
		.pull_down_en = 0,
		.pull_up_en = 1 
	};
	gpio_config(&g_conf);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);
	
	uart_queue = xQueueCreate(10, sizeof(uint8_t));
	xTaskCreate(&uart, "komunikacja_uart", 2048, NULL, 10, NULL);
	xTaskCreate(&servo, "servo_task", 2048, NULL, 10, NULL);
}



