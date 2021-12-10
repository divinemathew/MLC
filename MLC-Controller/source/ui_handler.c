/**
 * @file    MLC-Controller.c
 * @brief   Application entry point.
 */

#include "mlc_common.h"
#include "comm_handler.h"
#include <string.h>

/*******************************************
 * Const and Macro Defines
 *******************************************/
#define MAX_INPUT_LENGTH 5
#define CONFIG_ROW 8
#define CONFIG_COL 5

#define MAX_UP 0
#define MAX_DOWN 5
#define LINE_SPACE 2
#define STATUS_ROW_COL (15 + CONFIG_ROW, 8)
#define HEAD_1_ROW_COL (3, 30)
#define HEAD_2_ROW_COL (4, 30)

#define BACKSPACE 0x08
#define SEQUENCE 0x1B

/***********************************
 * Typedefs and Enum Declarations
 ***********************************/

// none

/***********************************
 * External Variable Declarations
 ***********************************/

// none

/***********************************
 * Const Declarations
 ***********************************/



const char up[5] = {0x1B, 0x5B, 'A'};
const char down[5] = {0x1B, 0x5B, 'B'};
const char right[5] = {0x1B, 0x5B, 'C'};
const char left[5] = {0x1B, 0x5B, 'D'};
const char clear_end[5] = {0x1B, 0x5B, 0x4B};
const char clear[5] = {0x1B, 0x5B, 0x32, 0x4A};
const char coordinates[7] = {0x1B, 0x5B, 0x39, 0x3B, 0x39, 0x48};


const char head_1[] = "DAK Technologies Pvt Ltd";
const char head_2[] = "Multicolor LED Controller";
const char config[][50] = {" 1. Start Color: ",
						   " 2. End Color: ",
						   " 3. Step Value: ",
						   " 4. Step Mode: ",
						   " 5. Change Rate: ",
						   " 6. Refresh Rate: "};

/***********************************
 * Public Variables
 ***********************************/

// none

/***********************************
 * Private Variables
 ***********************************/

_Bool line_changed = false;
_Bool master = false;

char val[][60] = {"25", "0", "0", "0", "0", "0"};
int number[5];
uint8_t cursor_line = 0;
uint8_t cursor_pos;
uint8_t value_length;
uint8_t count;

led_config_type configuration;

/***********************************
 * Private Prototypes
 ***********************************/
static void task_1(void *pvParameters);
static void task_2(void *pvParameters);
void insert(char*, uint16_t, char);
void delete(char*, uint16_t);
void process_ui_input(char*, char);
void change_line(void);
void set_cursor(uint16_t, uint16_t);
void run_master_ui(void);
void run_slave_ui(void);
void update_config_screen(void);
void print_ui_outline(void);

/***********************************
 * Public Functions
 ***********************************/
void print_ui_outline(void)
{
	PRINTF("%s", clear);
	set_cursor HEAD_1_ROW_COL;
	PRINTF("%s", head_1);
	set_cursor HEAD_2_ROW_COL;
	PRINTF("%s", head_2);
	for (int line = 0; line <= MAX_DOWN; line++) {
		set_cursor(CONFIG_ROW + (line * LINE_SPACE), CONFIG_COL);
		PRINTF("%s", config[line]);
		PRINTF("%s", val[line]);
	}
}

void update_status(TimerHandle_t xTimer)
{
	count += 234;
	set_cursor STATUS_ROW_COL;
	PRINTF("%s", clear_end);
	PRINTF("%d", count);
	set_cursor(CONFIG_ROW + cursor_line * LINE_SPACE, strlen(config[cursor_line]) + cursor_pos + CONFIG_COL);
}


void task_1(void *pvParameters)
{
    while (true) {
    	if (uxQueueSpacesAvailable(communication_queue)) {
    		GPIO_PortSet(GPIOB, 1u << 21); //off
    		char c = 'M';
    		BaseType_t status = xQueueSend(communication_queue, &c, 0);
    		c = 'i';
    		xQueueSend(communication_queue, &c, 0);
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
    	if (uxQueueMessagesWaiting(communication_queue)) {
    		xQueueReceive(communication_queue, &c, ( TickType_t ) 0x0);
    		PRINTF("%c", c);
    		PRINTF("%d\n\r", uxQueueMessagesWaiting(communication_queue));
    		GPIO_PortClear(GPIOB, 1u << 21); //on
    	}
    }
}
void ui_handler_task(void* board_is_master)
{
	if (true) {
		run_master_ui();
	} else {
		run_slave_ui();
	}
}

void run_master_ui(void)
{
	char key_pressed;

	print_ui_outline();
	change_line();
	for ( ; ; ) {
		key_pressed = GETCHAR();
		switch (key_pressed) {
			case SEQUENCE :
				key_pressed = GETCHAR();
				if (key_pressed == up[1]) {
					key_pressed = GETCHAR();
				}
				if (key_pressed == 'C' && cursor_pos < value_length) {
					PRINTF("%s", right);
					cursor_pos++;
				} else if (key_pressed == 'D' && cursor_pos > 0) {
					PRINTF("%s", left);
					cursor_pos--;
				} else if (key_pressed == 'A') {
					if (cursor_line > MAX_UP) {
						PRINTF("%s", up);
						cursor_line--;
						change_line();
					}
				} else if (key_pressed == 'B') {
					if (cursor_line < MAX_DOWN) {
						PRINTF("%s", down);
						cursor_line++;
						change_line();
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
	    		xQueueSend(communication_queue, &configuration, 0);
				taskYIELD();
				break;

			default :
				process_ui_input(val[cursor_line], key_pressed);
		}
	}
}


void run_slave_ui(void)
{

	print_ui_outline();
}

void update_config_screen(void)
{

}

void change_line(void)
{
	cursor_pos = strlen(val[cursor_line]);
	value_length = cursor_pos;
	set_cursor(CONFIG_ROW + (cursor_line * LINE_SPACE), strlen(config[cursor_line]) + value_length + CONFIG_COL);
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
				value_length--;
				if (value_length > cursor_pos) {
					char move_left_times[4];
					itoa((value_length - cursor_pos), move_left_times, 10);
					PRINTF("%c%c%s%c", left[0], left[1], move_left_times, left[2]);
				}
			}
			break;

		case '0' ... '9' :
			if (value_length < MAX_INPUT_LENGTH) {
				insert(str, cursor_pos, key_pressed);
				PRINTF("%s", str + cursor_pos);
				for (int i = 0; i < (value_length - cursor_pos); i++) {
					PRINTF("%s", left);
				}
				value_length++;
				cursor_pos++;
			} else {
				PRINTF("\a");
			}
	}
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

