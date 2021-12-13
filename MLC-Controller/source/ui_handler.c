/**
 * @file    MLC-Controller.c
 * @brief   Application entry point.
 */

#include "mlc_common.h"
#include <string.h>

/*******************************************
 * Const and Macro Defines
 *******************************************/
#define STATUS_UPDATE_TICKS 100
#define MAX_INPUT_LENGTH 5
#define CONFIG_VALUE_LENGTH 13
#define LINE_SPACE 2

#define CONFIG_ROW 18
#define CONFIG_COL 5
#define CONFIG_ROW_COL (CONFIG_ROW, CONFIG_COL)
#define STATUS_ROW 10
#define STATUS_COL 5
#define STATUS_ROW_COL (STATUS_ROW, STATUS_COL)
#define HEAD_1_ROW_COL (3, 30)
#define HEAD_2_ROW_COL (4, 30)

#define MAX_UP 0
#define MAX_DOWN 5
#define MODE_LINE 3
#define STATUS_COUNT 3
#define BACKSPACE 0x08
#define SEQUENCE 0x1B


const char up[5] = {0x1B, 0x5B, 'A'};
const char down[5] = {0x1B, 0x5B, 'B'};
const char right[5] = {0x1B, 0x5B, 'C'};
const char left[5] = {0x1B, 0x5B, 'D'};
const char clear_end[5] = {0x1B, 0x5B, 0x4B};
const char clear[5] = {0x1B, 0x5B, 0x32, 0x4A};
const char coordinates[7] = {0x1B, 0x5B, 0x39, 0x3B, 0x39, 0x48};


const char title_1[] = "DAK Technologies Pvt Ltd";
const char title_2[] = "Multicolor LED Controller";
const char master_name[] = "MASTER";
const char slave_name[] = "SLAVE";
const char status_title[] = "Status";
const char config_title[] = "Configuration";


const char status_name[][20] = {"Connection: ",
							    "Current Color: ",
							    "Current Position: ",
							    "Status 4"
							    };

const char status_const_str[][2][25] = {{"Slave not connected", "Slave Connected"},
									    {"Master not connected", "Master Connected"}
									   };

const char config_name[][20] = {"1. Start Color:  ",
						   	   	"2. End Color:    ",
								"3. Step Value:   ",
								"4. Step Mode:    ",
								"5. Change Rate:  ",
								"6. Refresh Rate: "
						 	    };

const char step_mode_name[][CONFIG_VALUE_LENGTH] = {"Auto Up",
													"Auto Down",
													"Auto Up-Down",
													"Manual"
													};

/***********************************
 * Typedefs and Enum Declarations
 ***********************************/
typedef enum {
	AUTO_UP,
	AUTO_DOWN,
	AUTO_UP_DOWN,
	MANUAL
} step_mode_enum;

typedef enum {
	NONE,
	UP,
	DOWN,
	LEFT,
	RIGHT
} arrow_key_type;
/***********************************
 * External Variable Declarations
 ***********************************/

// none

/***********************************
 * Const Declarations
 ***********************************/

// none

/***********************************
 * Public Variables
 ***********************************/

// none

/***********************************
 * Private Variables
 ***********************************/

static QueueHandle_t communication_queue;
static QueueHandle_t slave_status_queue;

_Bool master_mode = true;
_Bool device_connected = false;

_Bool anime_frame_1 = true;
char config_value_str[][CONFIG_VALUE_LENGTH] = {"(0, 0, 0)",
												"(7, 7, 3)",
												"1",
												"Auto_Up",
												"1",
												"1"};
int number[5];
uint8_t cursor_line = 0;
uint8_t cursor_pos;
uint8_t value_length;
uint8_t count;

led_config_type configuration;
TimerHandle_t status_timer;

/***********************************
 * Private Prototypes
 ***********************************/
static void task_1(void *pvParameters);
static void task_2(void *pvParameters);
void insert(char* str, uint16_t position, char input);
void delete(char* str, uint16_t position);

void process_ui_input(char* str, char key_pressed, arrow_key_type arrow_pressed);
void read_color_input(char* str, char key_pressed, arrow_key_type arrow_pressed);
void read_number_input(char* str, char key_pressed, arrow_key_type arrow_pressed);
void read_mode_input(char* str, char key_pressed, arrow_key_type arrow_pressed);

void change_line(void);
void set_cursor(uint16_t row, uint16_t col);
void run_master_ui(void);
void run_slave_ui(void);
void update_config_screen(void);
void draw_ui(void);
void update_status_periodic(TimerHandle_t xTimer);
void draw_dotted_square(uint16_t length, uint16_t breadth);
void draw_square(uint16_t length, uint16_t breadth);
void move_cursor_left(uint16_t no_of_times);
void clear_next(uint8_t length);

/***********************************
 * Public Functions
 ***********************************/
void ui_handler_task(void* board_is_master)
{

	communication_queue = get_queue_handle(COMMUNICATION_QUEUE);
    status_timer = xTimerCreate("AutoReload", STATUS_UPDATE_TICKS, pdTRUE, 0, update_status_periodic);
    xTimerStart(status_timer, 0);

    /* Clear screen and print UI */
	PRINTF("%s", clear);
	if (master_mode) {
		run_master_ui();
	} else {
		run_slave_ui();
	}
}

void draw_ui(void)
{
	/* Draw title */
	set_cursor HEAD_1_ROW_COL;
	PRINTF("%s%s%s%s", left, left, up, up);
	draw_square(strlen(title_2) + 2, 4);
	PRINTF("%s ", down);
	PRINTF("%s", title_1);
	set_cursor HEAD_2_ROW_COL;
	PRINTF("%s", title_2);
	PRINTF("%s ", down);
    move_cursor_left(17);
    if (master_mode) {
    	PRINTF("%s", master_name);
    } else {
    	PRINTF("%s", slave_name);
    }

	/* Draw status */
	for (uint16_t line = 0; line < STATUS_COUNT; line++) {
		set_cursor(STATUS_ROW + (line * LINE_SPACE), STATUS_COL);
		PRINTF("%s", status_name[line]);
		PRINTF("%s", clear_end);
	}

	/* Draw configuration table */
	for (uint16_t line = 0; line <= MAX_DOWN; line++) {
		set_cursor(CONFIG_ROW + (line * LINE_SPACE), CONFIG_COL);
		PRINTF("%s", config_name[line]);
		if (line != MODE_LINE) {
			PRINTF("%s", config_value_str[line]);
		}
		PRINTF("%s", clear_end);
	}
	set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + CONFIG_COL + 2);
	PRINTF("%s", config_value_str[MODE_LINE]);

	/* Draw hind */
}

void update_status_periodic(TimerHandle_t xTimer)
{

	if (xQueueReceive(slave_status_queue, &device_connected, 0)) {
	/* Redraw connection status */
	set_cursor(STATUS_ROW + (0 * LINE_SPACE), strlen(status_name[0]) + STATUS_COL);
	if (master_mode) {
		if (device_connected) {
			PRINTF("%s", status_const_str[0][1]);
			clear_next(5);
		} else {
			PRINTF("%s", status_const_str[0][0]);
			clear_next(5);
		}
	} else {
		if (device_connected) {
			PRINTF("%s", status_const_str[1][1]);
			clear_next(5);
		} else {
			PRINTF("%s", status_const_str[1][0]);
			clear_next(5);
		}
	}

	}

	/* Redraw current color */
	set_cursor(STATUS_ROW + (1 * LINE_SPACE), strlen(status_name[1]) + STATUS_COL);
	PRINTF("%s", "(0, 0, 0)");

	/* Redraw pattern position */
	set_cursor(STATUS_ROW + (2 * LINE_SPACE), strlen(status_name[2]) + STATUS_COL);
	PRINTF("%s", "[##########          ]");

	/* Next frame of animation */
	set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + CONFIG_COL);
	if (anime_frame_1) {
		PRINTF("< ");
		set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + strlen(step_mode_name[0]) + CONFIG_COL + 2);
		PRINTF(" >");
		anime_frame_1 = false;
	} else {
		PRINTF(" <");
		set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + strlen(step_mode_name[0]) + CONFIG_COL + 2);
		PRINTF("> ");
		anime_frame_1 = true;
	}

	/* Reset cursor */
	if (master_mode) {
		set_cursor(CONFIG_ROW + (cursor_line * LINE_SPACE), strlen(config_name[cursor_line]) + cursor_pos + CONFIG_COL);
	} else {
		set_cursor(100, 1);
	}
}

void task_1(void *pvParameters)
{
    while (true) {
//    //	if (uxQueueSpacesAvailable(communication_queue)) {
//    		GPIO_PortSet(GPIOB, 1u << 21); //off
//    		char c = 'M';
//    	//	BaseType_t status = xQueueSend(communication_queue, &c, 0);
//    		c = 'i';
//    //		xQueueSend(communication_queue, &c, 0);
//    		if (status == pdPASS) {
//    			//PRINTF("PASS");
//    		}
//    	}
    }
}

void task_2(void *pvParameters)
{
	char c;
    while (true) {
//    	if (uxQueueMessagesWaiting(communication_queue)) {
//    		xQueueReceive(communication_queue, &c, ( TickType_t ) 0x0);
//    		PRINTF("%c", c);
//    		PRINTF("%d\n\r", uxQueueMessagesWaiting(communication_queue));
//    		GPIO_PortClear(GPIOB, 1u << 21); //on
//    	}
    }
}

void run_master_ui(void)
{
	char key_pressed;

	draw_ui();
	change_line();
	for ( ; ; ) {
		key_pressed = GETCHAR();
		switch (key_pressed) {
			case SEQUENCE :
				key_pressed = GETCHAR();
				if (key_pressed == up[1]) {
					key_pressed = GETCHAR();
				}
				if (key_pressed == 'C') {
					process_ui_input(config_value_str[cursor_line], key_pressed, RIGHT);

				} else if (key_pressed == 'D') {
					process_ui_input(config_value_str[cursor_line], key_pressed, LEFT);

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

	    		xQueueSend(communication_queue, &configuration, 0);
				taskYIELD();
				break;

			default :
				process_ui_input(config_value_str[cursor_line], key_pressed, NONE);
		}
	}
}


void run_slave_ui(void)
{

	draw_ui();
}

void update_config_screen(void)
{

}

void change_line(void)
{
	switch (cursor_line) {
		case 0 :
		case 1 :
			cursor_pos = 1;
			value_length = 9;
			set_cursor(CONFIG_ROW + (cursor_line * LINE_SPACE), strlen(config_name[cursor_line]) + 1 + CONFIG_COL);
			break;

		case 2 :
		case 4 :
		case 5 :
			cursor_pos = strlen(config_value_str[cursor_line]);
			value_length = cursor_pos;
			set_cursor(CONFIG_ROW + (cursor_line * LINE_SPACE), strlen(config_name[cursor_line]) + value_length + CONFIG_COL);
			break;
      
		case 3 :
			cursor_pos = strlen(config_value_str[cursor_line]) + 4;
			value_length = cursor_pos;
			set_cursor(CONFIG_ROW + (cursor_line * LINE_SPACE), strlen(config_name[cursor_line]) + value_length + CONFIG_COL);
	}
}

void process_ui_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (cursor_line) {
		case 0 :
		case 1 :
			read_color_input(str, key_pressed, arrow_pressed);
			break;

		case 2 :
		case 4 :
		case 5 :
			read_number_input(str, key_pressed, arrow_pressed);
			break;

		case 3 :
			read_mode_input(str, key_pressed, arrow_pressed);
	}
}
void read_color_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (arrow_pressed) {
		case LEFT :
			if (key_pressed == 'D' && cursor_pos > 1) {
				PRINTF("%s", left);
				PRINTF("%s", left);
				PRINTF("%s", left);
				cursor_pos -= 3;
			}
			break;

		case RIGHT :
			if (cursor_pos < (value_length - 2)) {
				PRINTF("%s", right);
				PRINTF("%s", right);
				PRINTF("%s", right);
				cursor_pos += 3;
			}
			break;

		default :;
	}

	switch (key_pressed) {
		case '0' ... '9' :
			delete(str, cursor_pos);
			insert(str, cursor_pos, key_pressed);
			PRINTF("%s", str + cursor_pos);
			move_cursor_left(value_length - cursor_pos);
	}
}

void read_mode_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{

}

void read_number_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (arrow_pressed) {
		case LEFT :
			if (key_pressed == 'D' && cursor_pos > 0) {
				PRINTF("%s", left);
				cursor_pos--;
			}
			break;

		case RIGHT :
			if (cursor_pos < value_length) {
				PRINTF("%s", right);
				cursor_pos++;
			}
			break;

		default :;
	}

	switch (key_pressed) {
		case BACKSPACE :
			if (cursor_pos > 0) {
				cursor_pos--;
				delete(str, cursor_pos);
				PRINTF("%s", left);
				PRINTF("%s", str + cursor_pos);
				PRINTF("%s", clear_end);
				value_length--;
				move_cursor_left(value_length - cursor_pos);
			}
			break;

		case '0' ... '9' :
			if (value_length < MAX_INPUT_LENGTH) {
				insert(str, cursor_pos, key_pressed);
				PRINTF("%s", str + cursor_pos);
				move_cursor_left(value_length - cursor_pos);
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
	PRINTF("%c%c", coordinates[0], coordinates[1]);
	PRINTF("%s", row_str);
	PRINTF("%c", coordinates[3]);
	PRINTF("%s", col_str);
	PRINTF("%c", coordinates[5]);
}

void move_cursor_left(uint16_t no_of_times)
{
	char no_of_times_str[4];

	if (no_of_times > 0) {
		itoa(no_of_times, no_of_times_str, 10);
		PRINTF("%c%c%s%c", left[0], left[1], no_of_times_str, left[2]);
	}
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

void draw_dotted_square(uint16_t length, uint16_t breadth)
{
	/* Print line to right */
	PRINTF("+");
	for (uint16_t len = 0; len < length; len++) {
		PRINTF("-");
	}
	PRINTF("+");

	/* Print down */
	for (uint16_t height = 0; height < breadth; height++) {
		PRINTF("%s%s|", left, down);
	}
	PRINTF("%s", down);

	/* Move left and print to right */
	move_cursor_left(length + 2);
	PRINTF("+");
	for (uint16_t len = 0; len < length; len++) {
		PRINTF("-");
	}
	PRINTF("+");
	move_cursor_left(length + 1);

	/* Print up */
	for (uint16_t height = 0; height < breadth; height++) {
		PRINTF("%s%s|", left, up);
	}
}

void draw_square(uint16_t length, uint16_t breadth)
{
	/* Print line to right */
	PRINTF(" ");
	for (uint16_t len = 0; len < length; len++) {
		PRINTF("_");
	}
	PRINTF(" ");

	/* Print down */
	for (uint16_t height = 0; height < breadth; height++) {
		PRINTF("%s%s|", left, down);
	}
	PRINTF("%s", down);

	/* Move left and print to right */
	move_cursor_left(length + 2);
	PRINTF("|");
	for (uint16_t len = 0; len < length; len++) {
		PRINTF("_");
	}
	PRINTF("|");
	move_cursor_left(length + 1);

	/* Print up */
	for (uint16_t height = 0; height < breadth; height++) {
		PRINTF("%s%s|", left, up);
	}
}

void clear_next(uint8_t length)
{
	for (uint8_t len = 0; len < length; len++) {
		PRINTF(" ");
	}
	move_cursor_left(length);
}
