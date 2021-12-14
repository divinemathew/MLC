/**
 * @file    MLC-Controller.c
 * @brief   Application entry point.
 */

#include "mlc_common.h"
#include <string.h>

/*******************************************
 * Const and Macro Defines
 *******************************************/
#define STATUS_UPDATE_TICKS 25
#define MAX_INPUT_LENGTH 5
#define CONFIG_VALUE_LENGTH 15
#define LINE_SPACE 2

#define CONFIG_ROW 22
#define CONFIG_COL 5
#define CONFIG_ROW_COL (CONFIG_ROW, CONFIG_COL)
#define STATUS_ROW 12
#define STATUS_COL 5
#define STATUS_ROW_COL (STATUS_ROW, STATUS_COL)
#define HEAD_1_ROW_COL (3, 30)
#define HEAD_2_ROW_COL (4, 30)
#define ERROR_ROW_COL (36, 5)

#define MIN_STEP_VALUE 1
#define MAX_STEP_VALUE 100
#define MIN_NO_OF_CYCLES 1
#define MAX_NO_OF_CYCLES 100
#define MIN_CHANGE_RATE 1
#define MAX_CHANGE_RATE 100
#define MIN_REFRESH_RATE 1
#define MAX_REFRESH_RATE 100

#define MAX_UP 0
#define MAX_DOWN 6
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
								"5. Step Mode:   ",
								"4. No. of cycles ",
								"6. Change Rate:  ",
								"7. Refresh Rate: "
						 	    };

const char step_mode_name[][CONFIG_VALUE_LENGTH] = {"1.Auto Up",
													"2.Auto Down",
													"3.Auto Up-Down",
													"4.Manual"
													};

/***********************************
 * Typedefs and Enum Declarations
 ***********************************/
typedef enum {
	AUTO_UP = 0,
	AUTO_DOWN = 1,
	AUTO_UP_DOWN = 2,
	MANUAL = 3
} step_mode_enum;

typedef enum {
	NONE,
	UP,
	DOWN,
	LEFT,
	RIGHT
} arrow_key_type;

typedef enum {
	START_COLOR = 0,
	STOP_COLOR = 1,
	STEP_VALUE = 2,
	STEP_MODE = 3,
	NO_OF_CYCLES = 4,
	COLOR_CHANGE_RATE = 5,
	REFRESH_RATE = 6,
	COLOR_SCHEME = 7,
	CONTROL_MODE = 8
} config_name_enum;

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
_Bool device_connected = true;
_Bool anime_frame_1 = true;
char config_value_str[][CONFIG_VALUE_LENGTH] = {"(0, 0, 0)",
												"(7, 7, 3)",
												"1",
												"1.Auto_Up",
												"1",
												"1",
												"1"};
int number[5];
step_mode_enum step_mode = AUTO_UP;
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
uint8_t parse_color(char* color_str);
void validate_and_send(void);

/***********************************
 * Public Functions
 ***********************************/
void ui_handler_task(void* board_is_master)
{
	communication_queue = get_queue_handle(COMMUNICATION_QUEUE);
	slave_status_queue = get_queue_handle(SLAVE_STATUS_QUEUE);
    status_timer = xTimerCreate("Status_timer", STATUS_UPDATE_TICKS, pdTRUE, 0, update_status_periodic);
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
    set_cursor(STATUS_ROW - (LINE_SPACE + 1), STATUS_COL - 1);
    draw_dotted_square(21, 1);
    PRINTF("       %s", status_title);
	for (uint16_t line = 0; line < STATUS_COUNT; line++) {
		set_cursor(STATUS_ROW + (line * LINE_SPACE), STATUS_COL);
		PRINTF("%s", status_name[line]);
		PRINTF("%s", clear_end);
	}
	set_cursor(STATUS_ROW + (0 * LINE_SPACE), strlen(status_name[0]) + STATUS_COL);
	PRINTF("%s", "Waiting...");

	/* Draw configuration table */
	set_cursor(CONFIG_ROW - (LINE_SPACE + 1), CONFIG_COL - 1);
	    draw_dotted_square(20, 1);
	    PRINTF("    %s", config_title);
	for (uint16_t line = 0; line <= MAX_DOWN; line++) {
		set_cursor(CONFIG_ROW + (line * LINE_SPACE), CONFIG_COL);
		PRINTF("%s", config_name[line]);
		if (line != MODE_LINE) {
			PRINTF("%s", config_value_str[line]);
		}
		PRINTF("%s", clear_end);
	}
	set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + CONFIG_COL + 1);
	PRINTF("%s", config_value_str[MODE_LINE]);

	/* Draw hind */
}

void update_status_periodic(TimerHandle_t xTimer)
{
	int16_t row, col;

	/* Redraw connection status */
	if (xQueueReceive(slave_status_queue, &device_connected, 0)) {
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
	PRINTF("%s", "[#####     ]");

	/* Next frame of animation */
	set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[MODE_LINE]) + CONFIG_COL);
	if (anime_frame_1) {
		if (step_mode > AUTO_UP) {
			PRINTF("< ");
		} else {
			PRINTF("  ");
		}
		row = CONFIG_ROW + (MODE_LINE * LINE_SPACE);
		col = strlen(config_name[MODE_LINE]) + strlen(step_mode_name[step_mode]) + CONFIG_COL + 2;
		set_cursor(row, col);
		if (step_mode < MANUAL) {
			PRINTF(" >");
		} else {
			PRINTF("  ");
		}
		anime_frame_1 = false;
	} else {
		if (step_mode > AUTO_UP) {
			PRINTF(" <");
		} else {
			PRINTF("  ");
		}
		row = CONFIG_ROW + (MODE_LINE * LINE_SPACE);
		col = strlen(config_name[MODE_LINE]) + strlen(step_mode_name[step_mode]) + CONFIG_COL + 2;
		set_cursor(row, col);
		if (step_mode < MANUAL) {
			PRINTF("> ");
		} else {
			PRINTF("  ");
		}
		anime_frame_1 = true;
	}

	/* Reset cursor */
	if (master_mode) {
		row = CONFIG_ROW + (cursor_line * LINE_SPACE);
		col = strlen(config_name[cursor_line]) + cursor_pos + CONFIG_COL;
		set_cursor(row, col);
	} else {
		set_cursor(1, 1);
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
				validate_and_send();
				taskYIELD();
				break;

			default :
				process_ui_input(config_value_str[cursor_line], key_pressed, NONE);
		}
	}
}

void validate_and_send(void)
{
	_Bool config_valid = true;

	configuration.start_color[0] = parse_color(config_value_str[START_COLOR]);
	configuration.stop_color[0] = parse_color(config_value_str[STOP_COLOR]);
	configuration.step_value = atoi(config_value_str[STEP_VALUE]);
	configuration.step_mode = (uint8_t)step_mode;
	configuration.no_of_cycles = atoi(config_value_str[NO_OF_CYCLES]);
	configuration.color_change_rate = atoi(config_value_str[COLOR_CHANGE_RATE]);
	configuration.refresh_rate = atoi(config_value_str[REFRESH_RATE]);

	config_valid &= config_value_str[START_COLOR][1] >= '0';
	config_valid &= config_value_str[START_COLOR][4] >= '0';
	config_valid &= config_value_str[START_COLOR][7] >= '0';
	config_valid &= config_value_str[STOP_COLOR][1] >= '0';
	config_valid &= config_value_str[STOP_COLOR][4] >= '0';
	config_valid &= config_value_str[STOP_COLOR][7] >= '0';

	config_valid &= config_value_str[START_COLOR][1] <= '7';
	config_valid &= config_value_str[START_COLOR][4] <= '7';
	config_valid &= config_value_str[START_COLOR][7] <= '3';
	config_valid &= config_value_str[STOP_COLOR][1] <= '7';
	config_valid &= config_value_str[STOP_COLOR][4] <= '7';
	config_valid &= config_value_str[STOP_COLOR][7] <= '3';

	config_valid &= configuration.step_value >= MIN_STEP_VALUE;
	config_valid &= configuration.no_of_cycles >= MIN_NO_OF_CYCLES;
	config_valid &= configuration.color_change_rate >= MIN_CHANGE_RATE;
	config_valid &= configuration.refresh_rate >= MIN_CHANGE_RATE;

	config_valid &= configuration.step_value <= MAX_STEP_VALUE;
	config_valid &= configuration.no_of_cycles <= MAX_NO_OF_CYCLES;
	config_valid &= configuration.color_change_rate <= MAX_CHANGE_RATE;
	config_valid &= configuration.refresh_rate <= MAX_CHANGE_RATE;

	if (config_valid) {
		xQueueSend(communication_queue, &configuration, 0);
		set_cursor ERROR_ROW_COL;
		PRINTF("%s", clear_end);
	} else {
		set_cursor ERROR_ROW_COL;
		PRINTF("Invalid Configuration");
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

		/* color data type */
		case START_COLOR :
		case STOP_COLOR :
			cursor_pos = 1;
			value_length = 9;
			set_cursor(CONFIG_ROW + (cursor_line * LINE_SPACE), strlen(config_name[cursor_line]) + 1 + CONFIG_COL);
			break;

		/* number data type */
		case STEP_VALUE:
		case NO_OF_CYCLES :
		case COLOR_CHANGE_RATE :
		case REFRESH_RATE :
			cursor_pos = strlen(config_value_str[cursor_line]);
			value_length = cursor_pos;
			set_cursor(CONFIG_ROW + (cursor_line * LINE_SPACE), strlen(config_name[cursor_line]) + value_length + CONFIG_COL);
			break;

		/* change mode type input */
		case STEP_MODE :
			cursor_pos = strlen(step_mode_name[step_mode]) + 4;
			value_length = cursor_pos;
			set_cursor(CONFIG_ROW + (cursor_line * LINE_SPACE), strlen(config_name[cursor_line]) + value_length + CONFIG_COL);
	}
}

void process_ui_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (cursor_line) {
		case START_COLOR :
		case STOP_COLOR :
			read_color_input(str, key_pressed, arrow_pressed);
			break;

		case STEP_VALUE:
		case NO_OF_CYCLES :
		case COLOR_CHANGE_RATE :
		case REFRESH_RATE :
			read_number_input(str, key_pressed, arrow_pressed);
			break;

		case STEP_MODE :
			read_mode_input(str, key_pressed, arrow_pressed);
	}
}

void read_color_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (arrow_pressed) {
		case LEFT :
			if (cursor_pos > 1) {
				PRINTF("%s%s%s", left, left, left);
				cursor_pos -= 3;
			}
			break;

		case RIGHT :
			if (cursor_pos < (value_length - 2)) {
				PRINTF("%s%s%s", right, right, right);
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
		read_color_input(str, '\0', RIGHT);
	}
}

void read_mode_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (arrow_pressed) {
		case LEFT :
			if (step_mode > AUTO_UP) {
				step_mode--;
				set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + CONFIG_COL + 2);
				PRINTF("%s", step_mode_name[step_mode]);
				clear_next(8);
				change_line();
			}
			break;

		case RIGHT :
			if (step_mode < MANUAL) {
				step_mode++;
				set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + CONFIG_COL + 2);
				PRINTF("%s", step_mode_name[step_mode]);
				clear_next(8);
				change_line();
			}
			break;

		default :;
	}
}

void read_number_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (arrow_pressed) {
		case LEFT :
			if (cursor_pos > 0) {
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

uint8_t parse_color(char* color_str)
{
	uint8_t result = 0;
	char blue[2], green[2], red[2];

	blue[0] = color_str[7];
	blue[1] = '\0';
	green[0] = color_str[4];
	green[1] = '\0';
	red[0] = color_str[1];
	red[1] = '\0';

	result |= atoi(blue) & 0x03;
	result |= (atoi(green) & 0x07) << 2;
	result |= (atoi(red) & 0x07) << 5;
	return result;
}
