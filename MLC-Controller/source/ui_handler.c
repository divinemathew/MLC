/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include <string.h>

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "fsl_clock.h"
#include "fsl_uart.h"
#include "fsl_ftm.h"
#include "fsl_i2c.h"
#include "MK64F12.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/* Task priorities. */
#define MAX_INPUT_LENGTH 50
#define TOP_LINE 3
#define MAX_UP 0
#define MAX_DOWN 3
#define BACKSPACE 0x08
#define SEQUENCE 0x1B
#define UART_USED UART0
#define SLAVE_ADDRESS 0x22
#define MASTER_I2C I2C0
#define SLAVE_I2C I2C1
#define hello_task_PRIORITY (configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void task_1(void *pvParameters);
static void task_2(void *pvParameters);
void insert(char*, uint16_t, char);
void delete(char*, uint16_t);
void process_ui_input(char*, char);
void set_cursor(uint16_t, uint16_t);
void master_ui_task(void *);
void glow_led_task(void*);
void i2c_send_task(void*);
void i2c_write(char*, uint16_t);
void print_ui(void);
void update_display(void);
void print_status(TimerHandle_t);

static QueueHandle_t queue = NULL;
i2c_slave_handle_t g_s_handle;

char val[][60] = {"25", "0", "0", "0"};
int number[5];
int cursor_line = 0;
int cursor_pos;
int length;
int count;

uint16_t i2c_recieve_size = 4;
TickType_t last_wake;
TaskHandle_t xHandle, master_ui_handle;
TimerHandle_t timer1;

const char clear_end[5] = {0x1B, 0x5B, 0x4B};
const char clear[5] = {0x1B, 0x5B, 0x32, 0x4A};
const char coordinates[7] = {0x1B, 0x5B, 0x39, 0x3B, 0x39, 0x48};
const char up[5] = {0x1B, 0x5B, 'A'};
const char down[5] = {0x1B, 0x5B, 'B'};
const char right[5] = {0x1B, 0x5B, 'C'};
const char left[5] = {0x1B, 0x5B, 'D'};
const char head[] = "                  DAK Technologies Pvt Ltd\n\r";
const char config[][50] = {"Period: ", "Delay: ", "Increment: ", "Refresh Rate: "};

_Bool config_updated = false;
_Bool config_changed = false;
_Bool config_recieved = false;
_Bool line_changed = false;
_Bool master = false;

int slave_buff[20] = {25, 0, 0, 0};
char master_buff[20] = {1, 1, 1, 1};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
static void i2c1_slave_callback(I2C_Type *base, i2c_slave_transfer_t *xfer, void *userData)
{
    switch (xfer->event) {
    	case kI2C_SlaveReceiveEvent :
            /* Update information for received process */
            xfer->data     = slave_buff;
            xfer->dataSize = i2c_recieve_size;
			last_wake = xTaskGetTickCount();
            break;

    	case kI2C_SlaveCompletionEvent :
    		config_recieved = true;
    		master = false;
    }
}

int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    memset(&g_s_handle, 0, sizeof(g_s_handle));

    I2C_SlaveTransferCreateHandle(SLAVE_I2C, &g_s_handle, i2c1_slave_callback, NULL);

  //   queue = xQueueCreate(2, 1);
 //   xTaskCreate(task_1, "task1", configMINIMAL_STACK_SIZE + 100, NULL, 4, &master_ui_handle);
  //     xTaskCreate(task_2, "task2", configMINIMAL_STACK_SIZE + 100, NULL, 4, &xHandle);
    xTaskCreate(master_ui_task, "task1", configMINIMAL_STACK_SIZE + 100, NULL, 4, &master_ui_handle);
    xTaskCreate(glow_led_task, "task2", configMINIMAL_STACK_SIZE + 100, NULL, 4, &xHandle);
    xTaskCreate(i2c_send_task, "task4", configMINIMAL_STACK_SIZE + 100, NULL, 4, NULL);

    timer1 = xTimerCreate("AutoReload", 100000, pdTRUE, 0, print_status);

    xTimerStart(timer1, 0);

    GPIO_PortSet(GPIOE, 1u << 26);
	GPIO_PortSet(GPIOB, 1u << 21);
	GPIO_PortSet(GPIOB, 1u << 22);
	I2C_SlaveTransferNonBlocking(SLAVE_I2C, &g_s_handle, kI2C_SlaveCompletionEvent);


    vTaskStartScheduler();
    while (true) {
   // 	master_ui_task(NULL);
    }
}

void print_status(TimerHandle_t xTimer)
{
	set_cursor(8, 1);
	PRINTF("%s", clear_end);
	PRINTF("%d", count);
	set_cursor(TOP_LINE + cursor_line, strlen(config[cursor_line]) + cursor_pos + 1);
}

void task_1(void *pvParameters)
{
    while (true) {
    	if (uxQueueSpacesAvailable(queue)) {
    		GPIO_PortSet(GPIOB, 1u << 21); //off
    		char c = 'M';
    		BaseType_t status = xQueueSend(queue, &c, 0);
    		c = 'i';
    		xQueueSend(queue, &c, 0);
    		if (status == pdPASS) {
    			//PRINTF("PASS");
    		}
    	}
    }
}

void task_2(void *pvParameters)
{
	char c;
    while (true) {
    	if (uxQueueMessagesWaiting(queue)) {
    		xQueueReceive(queue, &c, ( TickType_t ) 0x0);
    		PRINTF("%c", c);
    		PRINTF("%d\n\r", uxQueueMessagesWaiting(queue));
    		GPIO_PortClear(GPIOB, 1u << 21); //on
    	}
    }
}
void master_ui_task(void* pvParameters)
{
	char key_pressed;
	print_ui();
	set_cursor(TOP_LINE + cursor_line, strlen(config[cursor_line]) + strlen(val[cursor_line]) + 1);
	cursor_pos = strlen(val[cursor_line]);
	length = cursor_pos;

	for ( ; ; ) {
		key_pressed = GETCHAR();
		switch (key_pressed) {
			case SEQUENCE :
				key_pressed = GETCHAR();
				if (key_pressed == up[1]) {
					key_pressed = GETCHAR();
				}
				if (key_pressed == 'C' && cursor_pos < length) {
					PRINTF("%s", right);
					cursor_pos++;
				} else if (key_pressed == 'D' && cursor_pos > MAX_UP) {
					PRINTF("%s", left);
					cursor_pos--;
				} else if (key_pressed == 'A') {
					if (cursor_line > 0) {
						PRINTF("%s", up);
						cursor_line--;
						line_changed = true;
					}
				} else if (key_pressed == 'B') {
					if (cursor_line < MAX_DOWN) {
						PRINTF("%s", down);
						cursor_line++;
						line_changed = true;
					}
				}
				break;

			case 's' :
			case 'S' :
				break;

			case 'p' :
			case 'P' :
				break;

			case '\r' :
				number[0] = atoi(val[0]);
				number[1] = atoi(val[1]);
				number[2] = atoi(val[2]);
				master_buff[0] = (char)number[0];
				master_buff[1] = (char)number[1];
				master_buff[2] = (char)number[2];
				config_changed = true;
				master = true;
				last_wake = xTaskGetTickCount();
				taskYIELD();
				break;

			default :
				process_ui_input(val[cursor_line], key_pressed);
		}
		if (line_changed) {
			set_cursor(TOP_LINE + cursor_line, strlen(config[cursor_line]) + strlen(val[cursor_line]) + 1);
			cursor_pos = strlen(val[cursor_line]);
			length = cursor_pos;
			line_changed = false;
		}
	}

}

void process_ui_input(char* str, char key_pressed)
{
	switch (key_pressed) {
		case BACKSPACE :
			if (cursor_pos > 0) {
				cursor_pos--;
				delete(str, cursor_pos);
				PRINTF("%s", left);
				PRINTF("%s", clear_end);
				PRINTF("%s", str + cursor_pos);
				length--;
				if (length > cursor_pos) {
					char move_left_times[4];
					itoa((length - cursor_pos), move_left_times, 10);
					PRINTF("%c%c%s%c", left[0], left[1], move_left_times, left[2]);
				}
			}
			break;

		case '-' :
		case '0' ... '9' :
			if (cursor_pos < MAX_INPUT_LENGTH) {
				insert(str, cursor_pos, key_pressed);
				PRINTF("%s", str + cursor_pos);
				for (int i = 0; i < (length - cursor_pos); i++) {
					PRINTF("%s", left);
				}
				length++;
				cursor_pos++;
			}
			break;
		}
}
void print_ui(void)
{
	PRINTF("%s", clear);
	PRINTF("%s", head);
	PRINTF("\n\r%s", config[0]);
	PRINTF("%s", val[0]);
	PRINTF("\n\r%s", config[1]);
	PRINTF("%s", val[1]);
	PRINTF("\n\r%s", config[2]);
	PRINTF("%s", val[2]);
	PRINTF("\n\r%s", config[3]);
	PRINTF("%s", val[3]);
}

void set_cursor(uint16_t row, uint16_t col)
{
	char row_str[4], col_str[4];
	itoa(row, row_str, 10);
	itoa(col, col_str, 10);
	PRINTF("%c%c%s%c%s%c", coordinates[0], coordinates[1], row_str, coordinates[3], col_str, coordinates[5]);
}

void insert(char* str, uint16_t position, char input)
{
	int16_t length = strlen(str) - 1;
	str[length + 2] = '\0';
	for (; length >= position; length--) {
		str[length + 1] = str[length];
	}
	str[position] = input;
}

void delete(char* str, uint16_t position)
{
	int16_t length = strlen(str) - 1;
	for (; position < length; position++) {
		str[position] = str[position + 1];
	}
	str[position] = '\0';
}

void glow_led_task(void *pvParameters)
{
	GPIO_PortSet(GPIOB, 1U << 22);
	GPIO_PortSet(GPIOE, 1U << 26);
	GPIO_PortSet(GPIOB, 1U << 21);
	vTaskDelayUntil(&last_wake, 10);
	while (true) {
		if (config_updated) {
			config_updated = false;
			if (slave_buff[3] != 0) {
				vTaskDelayUntil(&last_wake, slave_buff[3]);
			}
		}
		if (slave_buff[0] != 0) {
			GPIO_PortClear(GPIOB, 1U << 22);
			vTaskDelayUntil(&last_wake, slave_buff[0]);
			GPIO_PortSet(GPIOB, 1U << 22);
			GPIO_PortClear(GPIOE, 1U << 26);
			vTaskDelayUntil(&last_wake, slave_buff[0]);
			GPIO_PortSet(GPIOE, 1U << 26);
			GPIO_PortClear(GPIOB, 1U << 21);
			vTaskDelayUntil(&last_wake, slave_buff[0]);
			GPIO_PortSet(GPIOB, 1U << 21);
			vTaskDelayUntil(&last_wake, slave_buff[0]);
		}
	}
}

void i2c_send_task(void *pvParameters)
{
	while (true) {
		count++;
		if (config_recieved) {
			config_recieved = false;
			vTaskSuspend(master_ui_handle);
			vTaskDelete(xHandle);
			xTaskCreate(glow_led_task, "task2", configMINIMAL_STACK_SIZE + 100, NULL, 4, &xHandle);
			master_buff[0] = slave_buff[0];
			master_buff[1] = slave_buff[1];
			master_buff[2] = slave_buff[2];
			slave_buff[3] = (slave_buff[3] + (signed char)slave_buff[1])*(1 + ((signed char)slave_buff[2] / 127));
			master_buff[3] = slave_buff[3];

			i2c_write(master_buff, i2c_recieve_size);
			config_updated = true;

		} else if (config_changed) {
			slave_buff[0] = number[0];
			slave_buff[1] = number[1];
			slave_buff[2] = number[2];
			slave_buff[3] = 0;

			vTaskDelete(xHandle);
			xTaskCreate(glow_led_task, "task2", configMINIMAL_STACK_SIZE + 100, NULL, 4, &xHandle);
			i2c_write(master_buff, i2c_recieve_size);
			config_changed = false;
			config_updated = true;
		} else {
			taskYIELD();
		}
	}
}

void i2c_write(char* data, uint16_t size)
{
    i2c_master_transfer_t masterXfer;
	masterXfer.slaveAddress   = SLAVE_ADDRESS;
	masterXfer.direction      = kI2C_Write;
	masterXfer.subaddress     = 0x0B;
	masterXfer.subaddressSize = 0;
	masterXfer.data           = data;
	masterXfer.dataSize       = size;
	masterXfer.flags          = kI2C_TransferDefaultFlag;
    I2C_MasterTransferBlocking(MASTER_I2C, &masterXfer);
}

void update_display(void)
{

}
