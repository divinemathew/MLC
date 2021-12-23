/**
 * @file    MLC-Controller.c
 * @brief   Application entry point.
 */

#include "mlc_ui_lib.h"
#include <string.h>
#include "semphr.h"

/*******************************************
 * Const and Macro Defines
 *******************************************/


/***********************************
 * Typedefs and Enum Declarations
 ***********************************/

/***********************************
 * External Variable Declarations
 ***********************************/
extern TaskHandle_t ui_handler_handle;

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
static SemaphoreHandle_t console;
static QueueHandle_t communication_queue;
static QueueHandle_t device_status_queue;
static QueueHandle_t pattern_status_queue;

static _Bool master_mode = true;
static _Bool device_connected = true;
static _Bool config_edited = true;
static _Bool anime_frame_1 = true;
static char current_color_str[10] = "(0, 0, 0)";
static char config_value_str[][CONFIG_VALUE_LENGTH] = {"(0, 0, 0)",
													   "(7, 7, 3)",
													   "1",
													   "1.Auto_Up",
													   "1",
													   "1",
													   "1"
													   };
static step_mode_enum step_mode = AUTO_UP;
static uint8_t cursor_line = 0;
static uint8_t cursor_pos;
static uint8_t value_length;

static led_config_type configuration;

/***********************************
 * Private Prototypes
 ***********************************/


void process_ui_input(char* str, char key_pressed, arrow_key_type arrow_pressed);
void read_color_input(char* str, char key_pressed, arrow_key_type arrow_pressed);
void read_number_input(char* str, char key_pressed, arrow_key_type arrow_pressed);
void read_mode_input(char* str, char key_pressed, arrow_key_type arrow_pressed);

void change_line(void);
void run_master_ui(void);
void run_slave_ui(void);
void print_config_values(void);
void draw_ui(void);
void update_status_task(void* pvParameter);
uint8_t parse_color(char* color_str);
void color_to_str(uint8_t color, char* str_out);
void print_pattern_state(uint8_t control);
_Bool config_is_valid(void);
void print_connection_status(void);
void reset_cursor(void);


/***********************************
 * Public Functions
 ***********************************/
void ui_handler_task(void* board_is_master)
{
	/* initialize queues and status timer */
	communication_queue = get_queue_handle(COMMUNICATION_QUEUE);
	device_status_queue = get_queue_handle(DEVICE_STATUS_QUEUE);
	pattern_status_queue = get_queue_handle(PATTERN_STATUS_QUEUE);
	console = xSemaphoreCreateBinary();
	xSemaphoreGive(console);
    xTaskCreate(update_status_task, "UTask", configMINIMAL_STACK_SIZE + 100, NULL, 4, NULL);

    master_mode = *((_Bool*)board_is_master);
    configuration.start_color[0] = parse_color(config_value_str[START_COLOR]);
	configuration.stop_color[0] = parse_color(config_value_str[STOP_COLOR]);
	configuration.step_value = atoi(config_value_str[STEP_VALUE]);
	configuration.step_mode = (uint8_t)step_mode;
	configuration.no_of_cycles = atoi(config_value_str[NUMBER_OF_CYCLES]);
	configuration.color_change_rate = atoi(config_value_str[COLOR_CHANGE_RATE]);
	configuration.refresh_rate = atoi(config_value_str[REFRESH_RATE]);
    configuration.control_mode = NOP;

    /* Clear screen and print UI */
	PRINTF("%s", clear);
	if (master_mode) {
		run_master_ui();
	} else {
		run_slave_ui();
	}
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void draw_ui(void)
{
	/* Draw title */
	PRINTF("%s", hide_cursor);
	set_cursor HEAD_1_ROW_COL;
	move_cursor_left(TITLE_SQUARE_SIZE + 1);
	PRINTF("%s%s", up, up);
	draw_square(strlen(title_2) + TITLE_SQUARE_SIZE * 2, 4);
	set_cursor HEAD_1_ROW_COL;
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
    set_cursor(STATUS_ROW - (LINE_SPACE + 2), STATUS_COL - 1);
    draw_dotted_square(SMALL_SQUARE_SIZE, 1);
    PRINTF("       %s", status_title);
	for (uint16_t line = 0; line < STATUS_COUNT; line++) {
		set_cursor(STATUS_ROW + (line * LINE_SPACE), STATUS_COL);
		PRINTF("%s", status_name[line]);
		PRINTF("%s", clear_end);
	}
	set_cursor(STATUS_ROW + (0 * LINE_SPACE), strlen(status_name[0]) + STATUS_COL);
	PRINTF("%s", "Waiting...");
	set_cursor(STATUS_ROW + (1 * LINE_SPACE), strlen(status_name[1]) + STATUS_COL);
	PRINTF("%s", "(0, 0, 0)");
	set_cursor(STATUS_ROW + (2 * LINE_SPACE), strlen(status_name[2]) + STATUS_COL);
	PRINTF("%s", "[          ]");
	set_cursor PATTERN_STATE_ROW_COL;
	PRINTF("%s%s", left, up);
	draw_dotted_square(SMALL_SQUARE_SIZE, 1);
	PRINTF("%s", pattern_state_const[0]);

	/* Draw configuration table */
	set_cursor(CONFIG_ROW - (LINE_SPACE + 2), CONFIG_COL - 1);
	draw_dotted_square(SMALL_SQUARE_SIZE, 1);
	PRINTF("    %s", config_title);
	print_config_values();

	/* Draw hind */
	if (master_mode) {
		set_cursor(HINT_ROW - (7 * LINE_SPACE), HINT_COL - 1);
		draw_dotted_square(SMALL_SQUARE_SIZE, 10);
		set_cursor(HINT_ROW - (6 * LINE_SPACE), HINT_COL);
		PRINTF("ENTER - Apply changes");
		set_cursor(HINT_ROW - (5 * LINE_SPACE), HINT_COL);
		PRINTF("S - Stop");
		set_cursor(HINT_ROW - (4 * LINE_SPACE), HINT_COL);
		PRINTF("P - Pause");
		set_cursor(HINT_ROW - (3 * LINE_SPACE), HINT_COL);
		PRINTF("A/D - Prev/Next color");
		set_cursor(HINT_ROW - (1 * LINE_SPACE), HINT_COL);
		PRINTF("DESCRIPTION");
		set_cursor(HINT_ROW + (2 * LINE_SPACE), HINT_COL);
		PRINTF("RANGE");
	}

	PRINTF("%s", show_cursor);
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void update_status_task(void* pvParameter)
{
	while(1) {
		xSemaphoreTake(console, 100);
		uint8_t current_color = 0;
		int16_t row, col;

		print_connection_status();

		/* Redraw current color */
		PRINTF("%s", hide_cursor);
		if (xQueueReceive(pattern_status_queue, &current_color, 0)) {
			set_cursor(STATUS_ROW + (1 * LINE_SPACE), strlen(status_name[1]) + STATUS_COL);
			color_to_str(current_color, current_color_str);
			PRINTF("%s", current_color_str);

			/* Redraw pattern position */
			set_cursor(STATUS_ROW + (2 * LINE_SPACE), strlen(status_name[2]) + STATUS_COL);
			PRINTF("%s", "[##################   ]");
		}

		/* Next frame of animation */
		set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[MODE_LINE]) + CONFIG_COL);
		if (anime_frame_1 && master_mode) {
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
		} else if (master_mode){
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

		reset_cursor();
		xSemaphoreGive(console);
		vTaskDelay(STATUS_UPDATE_TICKS);
	}
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void reset_cursor(void)
{
	uint16_t row, col;

	if (master_mode) {
		row = CONFIG_ROW + (cursor_line * LINE_SPACE);
		col = strlen(config_name[cursor_line]) + cursor_pos + CONFIG_COL;
		set_cursor(row, col);
		PRINTF("%s", show_cursor);
	}
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void run_master_ui(void)
{
	char key_pressed;
	_Bool pattern_paused = false;
	_Bool pattern_running = false;

	/* print master home screen */
	xSemaphoreTake(console, 0);
	draw_ui();
	change_line();
	for ( ; ; ) {
		xSemaphoreGive(console);
		key_pressed = GETCHAR();
		xSemaphoreTake(console, 100);
		switch (key_pressed) {

			/* if read an escape sequence, check which escape sequence is recieved */
			case '\e' :
				key_pressed = GETCHAR();
				if (key_pressed == '[') {
					key_pressed = GETCHAR();
				}
				if (key_pressed == 'C') {
					process_ui_input(config_value_str[cursor_line], key_pressed, RIGHT_KEY);

				} else if (key_pressed == 'D') {
					process_ui_input(config_value_str[cursor_line], key_pressed, LEFT_KEY);

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
				/* send STOP command */
				configuration.control_mode = STOP;
				xQueueSend(communication_queue, &configuration, QUEUE_SEND_WAIT);
				print_pattern_state(configuration.control_mode);
				pattern_running = false;
				break;

			case 'p' :
			case 'P' :
				/* send toggled PAUSE/RESUME command */
				if (pattern_paused && pattern_running) {
					configuration.control_mode = RESUME;
					pattern_paused = false;
				} else if (pattern_running) {
					configuration.control_mode = PAUSE;
					pattern_paused = true;
				}
				xQueueSend(communication_queue, &configuration, QUEUE_SEND_WAIT);
				print_pattern_state(configuration.control_mode);
				break;

			case '\r' :
				/* send configuration and send START command */
				if (config_is_valid()) {
					if (config_edited) {
						configuration.control_mode = NOP;
						xQueueSend(communication_queue, &configuration, QUEUE_SEND_WAIT);
						config_edited = false;
					}
					configuration.control_mode = START;
					xQueueSend(communication_queue, &configuration, QUEUE_SEND_WAIT);
					pattern_running = true;
					print_pattern_state(configuration.control_mode);
					taskYIELD();
				} else {
					set_cursor PATTERN_STATE_ROW_COL;
					PRINTF("%s", pattern_state_const[4]);
				}
				break;

			case 'A' :
			case 'a' :
				/* send DOWN command */
				configuration.control_mode = DOWN;
				xQueueSend(communication_queue, &configuration, QUEUE_SEND_WAIT);
				break;

			case 'D' :
			case 'd' :
				/* send DOWN command */
				configuration.control_mode = UP;
				xQueueSend(communication_queue, &configuration, QUEUE_SEND_WAIT);
				break;

			default :
				process_ui_input(config_value_str[cursor_line], key_pressed, NO_KEY_PRESSED);
		}
	}
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
_Bool config_is_valid(void)
{
	_Bool config_valid = true;

	/* parse structure data from string */
	configuration.start_color[0] = parse_color(config_value_str[START_COLOR]);
	configuration.stop_color[0] = parse_color(config_value_str[STOP_COLOR]);
	configuration.step_value = atoi(config_value_str[STEP_VALUE]);
	configuration.step_mode = (uint8_t)step_mode;
	configuration.no_of_cycles = atoi(config_value_str[NUMBER_OF_CYCLES]);
	configuration.color_change_rate = atoi(config_value_str[COLOR_CHANGE_RATE]);
	configuration.refresh_rate = atoi(config_value_str[REFRESH_RATE]);

	/* check if color is greater than minimum value */
	config_valid &= config_value_str[START_COLOR][1] >= '0';
	config_valid &= config_value_str[START_COLOR][4] >= '0';
	config_valid &= config_value_str[START_COLOR][7] >= '0';
	config_valid &= config_value_str[STOP_COLOR][1] >= '0';
	config_valid &= config_value_str[STOP_COLOR][4] >= '0';
	config_valid &= config_value_str[STOP_COLOR][7] >= '0';

	/* check if color is less than maximum value */
	config_valid &= config_value_str[START_COLOR][1] <= '7';
	config_valid &= config_value_str[START_COLOR][4] <= '7';
	config_valid &= config_value_str[START_COLOR][7] <= '3';
	config_valid &= config_value_str[STOP_COLOR][1] <= '7';
	config_valid &= config_value_str[STOP_COLOR][4] <= '7';
	config_valid &= config_value_str[STOP_COLOR][7] <= '3';

	/* check if values are greater than minimum */
	config_valid &= configuration.step_value >= MIN_STEP_VALUE;
	config_valid &= configuration.no_of_cycles >= MIN_NO_OF_CYCLES;
	config_valid &= configuration.color_change_rate >= MIN_CHANGE_RATE;
	config_valid &= configuration.refresh_rate >= MIN_CHANGE_RATE;

	/* check if values are less than maximum */
	config_valid &= configuration.step_value <= MAX_STEP_VALUE;
	config_valid &= configuration.no_of_cycles <= MAX_NO_OF_CYCLES;
	config_valid &= configuration.color_change_rate <= MAX_CHANGE_RATE;
	config_valid &= configuration.refresh_rate <= MAX_CHANGE_RATE;

	return config_valid;
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void run_slave_ui(void)
{
	xSemaphoreTake(console, 100);
	draw_ui();
	xSemaphoreGive(console);
	for (; ;) {
		if (xQueueReceive(communication_queue, &configuration, 0)) {
			if (configuration.control_mode == NOP) {

				/* decode configurations to ASCII and print */
				color_to_str(configuration.start_color[0], config_value_str[START_COLOR]);
				color_to_str(configuration.stop_color[0], config_value_str[STOP_COLOR]);
				itoa(configuration.step_value, config_value_str[STEP_VALUE], 10);
				if (configuration.step_mode <= MAX_MODE_VALUE) {
					strcpy(config_value_str[STEP_MODE], step_mode_name[configuration.step_mode]);
					step_mode = configuration.step_mode;
				}
				itoa(configuration.no_of_cycles, config_value_str[NUMBER_OF_CYCLES], 10);
				itoa(configuration.color_change_rate, config_value_str[COLOR_CHANGE_RATE], 10);
				itoa(configuration.refresh_rate, config_value_str[REFRESH_RATE], 10);
				xSemaphoreTake(console, 100);
				print_config_values();
				xSemaphoreGive(console);
			} else {

				/* display control received */
				xSemaphoreTake(console, 100);
				print_pattern_state(configuration.control_mode);
				xSemaphoreGive(console);
			}
		} else {
			taskYIELD();
		}
	}
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void print_pattern_state(uint8_t control)
{
	uint8_t mode = 0;

	PRINTF("%s", hide_cursor);
	set_cursor PATTERN_STATE_ROW_COL;
	if (configuration.step_mode == MANUAL) {
		mode = 3;
	} else {
		switch (control) {
			case START :
			case RESUME :
				mode = 1;
				break;

			case PAUSE :
				mode = 2;
				break;
		}
	}
	PRINTF("%s", pattern_state_const[mode]);

	/* Redraw connection status */
	print_connection_status();
	reset_cursor();
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void print_connection_status(void)
{
	if (xQueueReceive(device_status_queue, &device_connected, 0)) {
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
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void print_config_values(void)
{
	/* Draw configuration values */
	for (uint16_t line = 0; line <= MAX_DOWN; line++) {
		set_cursor(CONFIG_ROW + (line * LINE_SPACE), CONFIG_COL);
		PRINTF("%s", config_name[line]);
		if (line != MODE_LINE) {
			PRINTF("%s", config_value_str[line]);
			clear_next(MAX_INPUT_LENGTH);
		}
	}
	set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[MODE_LINE]) + CONFIG_COL + 2);
	PRINTF("%s", config_value_str[MODE_LINE]);
	clear_next(8);
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
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
		case NUMBER_OF_CYCLES :
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

	set_cursor(HINT_ROW - (0 * LINE_SPACE), HINT_COL);
	PRINTF("%s", config_description[cursor_line][0]);
	set_cursor(HINT_ROW + (3 * LINE_SPACE), HINT_COL);
	PRINTF("%s", config_description[cursor_line][1]);

	reset_cursor();
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void process_ui_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (cursor_line) {
		case START_COLOR :
		case STOP_COLOR :
			read_color_input(str, key_pressed, arrow_pressed);
			break;

		case STEP_VALUE:
		case NUMBER_OF_CYCLES :
		case COLOR_CHANGE_RATE :
		case REFRESH_RATE :
			read_number_input(str, key_pressed, arrow_pressed);
			break;

		case STEP_MODE :
			read_mode_input(str, key_pressed, arrow_pressed);
	}
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void read_color_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (key_pressed) {
		case '0' ... '9' :
			delete(str, cursor_pos);
			insert(str, cursor_pos, key_pressed);
			PRINTF("%s", str + cursor_pos);
			move_cursor_left(value_length - cursor_pos);
			read_color_input(str, '\0', RIGHT_KEY);
			config_edited = true;
			break;

		case BACKSPACE :
			delete(str, cursor_pos);
			insert(str, cursor_pos, '0');
			PRINTF("%s", str + cursor_pos);
			move_cursor_left(value_length - cursor_pos);
			read_color_input(str, '\0', LEFT_KEY);
			config_edited = true;
			break;
	}

	switch (arrow_pressed) {
		case LEFT_KEY :
			if (cursor_pos > 1) {
				PRINTF("%s%s%s", left, left, left);
				cursor_pos -= 3;
			}
			break;

		case RIGHT_KEY :
			if (cursor_pos < (value_length - 2)) {
				PRINTF("%s%s%s", right, right, right);
				cursor_pos += 3;
			}
			break;

		default :;
	}


}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void read_mode_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (arrow_pressed) {
		case LEFT_KEY :
			if (step_mode > AUTO_UP) {
				step_mode--;
				set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + CONFIG_COL + 2);
				PRINTF("%s", step_mode_name[step_mode]);
				clear_next(8);
				change_line();
				config_edited = true;
			}
			break;

		case RIGHT_KEY :
			if (step_mode < MANUAL) {
				step_mode++;
				set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + CONFIG_COL + 2);
				PRINTF("%s", step_mode_name[step_mode]);
				clear_next(8);
				change_line();
				config_edited = true;
			}
			break;

		default :;
	}
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void read_number_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (arrow_pressed) {
		case LEFT_KEY :
			if (cursor_pos > 0) {
				PRINTF("%s", left);
				cursor_pos--;
			}
			break;

		case RIGHT_KEY :
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
				clear_next(MAX_INPUT_LENGTH);
				value_length--;
				move_cursor_left(value_length - cursor_pos);
				config_edited = true;
			}
			break;

		case '0' ... '9' :
			if (value_length < MAX_INPUT_LENGTH) {
				insert(str, cursor_pos, key_pressed);
				PRINTF("%s", str + cursor_pos);
				move_cursor_left(value_length - cursor_pos);
				value_length++;
				cursor_pos++;
				config_edited = true;
			} else {
				PRINTF("\a");
			}
	}
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
uint8_t parse_color(char* color_str)
{
	uint8_t result = 0;
	char color[2] = "0";

	color[0] = color_str[RED_OFFSET];
	result |= (atoi(color) & 0x07) << 5;
	color[0] = color_str[GREEN_OFFSET];
	result |= (atoi(color) & 0x07) << 2;
	color[0] = color_str[BLUE_OFFSET];
	result |= atoi(color) & 0x03;

	return result;
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void color_to_str(uint8_t color, char* str_out)
{
	char number_str[2];

	itoa((color & 0b11100000) >> 5, number_str, 10);
	str_out[RED_OFFSET] = number_str[0];
	itoa((color & 0b00011100) >> 2, number_str, 10);
	str_out[GREEN_OFFSET] = number_str[0];
	itoa((color & 0b00000011) >> 0, number_str, 10);
	str_out[BLUE_OFFSET] = number_str[0];
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
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

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void move_cursor_left(uint16_t no_of_times)
{
	char no_of_times_str[4];

	if (no_of_times > 0) {
		itoa(no_of_times, no_of_times_str, 10);
		PRINTF("%c%c%s%c", left[0], left[1], no_of_times_str, left[2]);
	}
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void insert(char* str, uint16_t position, char input)
{
	int16_t length = strlen(str) - 1;
	str[length + 2] = '\0';
	for (; length >= position; length--) {
		str[length + 1] = str[length];
	}
	str[position] = input;
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void delete(char* str, uint16_t position)
{
	int16_t length = strlen(str) - 1;
	for (; position < length; position++) {
		str[position] = str[position + 1];
	}
	str[position] = '\0';
}

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
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

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
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

/**
* @brief Lists a.
*
* Li.
*
* @param The
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void clear_next(uint8_t length)
{
	for (uint8_t len = 0; len < length; len++) {
		PRINTF(" ");
	}
	move_cursor_left(length);
}
