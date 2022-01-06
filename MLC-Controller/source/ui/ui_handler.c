/**
 * @file ui_handler.c
 * @brief Multi-color LED controller UI block
 *
 * This program prints a user interface on a serial monitor and
 * validates user inputs.
 *
 * @note
 *
 * Revision History:
 *	- 091221 ATG : Creation Date
 */

#include "mlc_ui_lib.h"
#include <string.h>
#include "semphr.h"

/*******************************************
 * Const and Macro Defines
 *******************************************/
// none

/***********************************
 * Typedefs and Enum Declarations
 ***********************************/
// none

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
static _Bool device_connected = false;
static _Bool config_edited = true;
static _Bool changes_cancelled = false;
static _Bool anime_frame_1 = true;
static _Bool pattern_running = false;
static char config_value_str[][CONFIG_VALUE_LENGTH] = {"(0, 0, 0)",
													   "(7, 7, 3)",
													   "(1, 1, 1)",
													   "1.Auto_Up",
													   "10",
													   "1",
													   "100"
													   };
static step_mode_enum step_mode = AUTO_UP;
static uint8_t current_color = 0;
static uint8_t cursor_line = 0;
static uint8_t cursor_pos;
static uint8_t value_length;
static uint8_t width = 10;

static led_config_type configuration;

/***********************************
 * Private Prototypes
 ***********************************/


void process_ui_input(char* str, char key_pressed, arrow_key_type arrow_pressed);
void read_color_input(char* str, char key_pressed, arrow_key_type arrow_pressed);
void read_number_input(char* str, char key_pressed, arrow_key_type arrow_pressed);
void read_mode_input(char* str, char key_pressed, arrow_key_type arrow_pressed);

void change_cursor_line(uint8_t line);
void print_line_error(uint8_t line);
void run_master_ui(void);
void run_slave_ui(void);
void print_configurations(void);
void draw_color_slider(uint8_t color);
void draw_current_color(uint8_t color);
void draw_ui(void);
void update_status_task(void* pvParameter);
uint8_t parse_color(char* color_str);
void color_to_str(uint8_t color, char* str_out);
void print_pattern_state(uint8_t control);
_Bool encode_config(void);
void print_connection_status(_Bool status);
void reset_cursor(void);
void animate_arrow(void);
void decode_config(void);

/***********************************
 * Public Functions
 ***********************************/

/**
* @brief Main task of UI handler.
*
* Initialize configuration and start master or slave UI.
*
* @param board_is_master	  Task parameter to check MLC mode.
*
* @note
*
* Revision History:
* - 201221 KAR: Creation Date
*/
void ui_handler_task(void* board_is_master)
{
	/* initialize queues and create status task */
	communication_queue = get_queue_handle(COMMUNICATION_QUEUE);
	device_status_queue = get_queue_handle(DEVICE_STATUS_QUEUE);
	pattern_status_queue = get_queue_handle(PATTERN_STATUS_QUEUE);
    xTaskCreate(update_status_task, "Animation task", configMINIMAL_STACK_SIZE + 100, NULL, 4, NULL);

    /* create semaphore to use console */
	console = xSemaphoreCreateBinary();
	xSemaphoreGive(console);

	/* initialize configuration with defaults */
    master_mode = *((_Bool*)board_is_master);
    encode_config();
    configuration.control_mode = NOP;

    /* Clear screen and start UI */
    while (1) {
		PRINTF("%s", clear);
		if (master_mode) {
			run_master_ui();
		} else {
			run_slave_ui();
		}
	}
}

/**
* @brief Prints UI screen outline.
*
* Redraws all static objects on screen.
*
* @param
* @return
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void draw_ui(void)
{
	PRINTF("%s", clear);
	PRINTF("%s", hide_cursor);

	/* Draw title */
	set_cursor (TITLE_ROW, TITLE_COL);
	draw_square(TITLE_SQUARE_SIZE, 4);
	set_cursor (TITLE_ROW + 2, TITLE_COL + (TITLE_SQUARE_SIZE / 2));
	move_cursor_left(strlen(title_1) / 2);
	PRINTF("%s", title_1);

	set_cursor (TITLE_ROW + 3, TITLE_COL + (TITLE_SQUARE_SIZE / 2));
	move_cursor_left(strlen(title_2) / 2);
	PRINTF("%s", title_2);

	set_cursor (TITLE_ROW + 4, TITLE_COL + (TITLE_SQUARE_SIZE / 2));
    if (master_mode) {
    	move_cursor_left(strlen(master_name) / 2);
    	PRINTF("%s", master_name);
    } else {
    	move_cursor_left(strlen(slave_name) / 2);
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
	draw_current_color(current_color);
	draw_color_slider(current_color);

	/* Draw pattern state */
	set_cursor PATTERN_STATE_ROW_COL;
	PRINTF("%s%s", left, up);
	draw_dotted_square(SMALL_SQUARE_SIZE, 1);
	PRINTF("%s", pattern_state_const[0]);

	/* Draw configuration table */
	set_cursor(CONFIG_ROW - (LINE_SPACE + 2), CONFIG_COL - 1);
	draw_dotted_square(SMALL_SQUARE_SIZE, 1);
	PRINTF("    %s", config_title);

	/* Draw hind box */
	if (master_mode) {
		set_cursor(HINT_ROW - (8 * LINE_SPACE), HINT_COL - 1);
		draw_dotted_square(SMALL_SQUARE_SIZE, 11);
		set_cursor(HINT_ROW - (7 * LINE_SPACE), HINT_COL);
		PRINTF("ENTER - Apply changes");
		set_cursor(HINT_ROW - (6 * LINE_SPACE), HINT_COL);
		PRINTF("S - Stop");
		set_cursor(HINT_ROW - (5 * LINE_SPACE), HINT_COL);
		PRINTF("P - Pause");
		set_cursor(HINT_ROW - (4 * LINE_SPACE), HINT_COL);
		PRINTF("A/D - Prev/Next color");
		set_cursor(HINT_ROW - (3 * LINE_SPACE), HINT_COL);
		PRINTF("C - Cancel changes");
		set_cursor(HINT_ROW - (1 * LINE_SPACE), HINT_COL);
		PRINTF("DESCRIPTION");
		set_cursor(HINT_ROW + (2 * LINE_SPACE), HINT_COL);
		PRINTF("RANGE");
	}

	reset_cursor();
}

/**
* @brief Pattern status update task.
*
* Periodically checks for color change and manages UI animations.
* Updates connection status and current color on UI.
* Manages arrow animation and color position slider animation.
*
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void update_status_task(void* pvParameter)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	int16_t pattern_state;
	uint8_t delay_count = 0;
#if ENABLE_FAST_COLOR_REFRESH == 0
	_Bool color_changed = false;
#endif

	while(1) {
		xSemaphoreTake(console, CONSOLE_SEMAPHORE_WAIT);

		/* Increment animation delay counter.
		 * Check and print connection status */
		delay_count++;
		if (xQueueReceive(device_status_queue, &device_connected, 0)) {
			print_connection_status(device_connected);
		}

		/* Read and print current color from pattern executer */
		if (xQueueReceive(pattern_status_queue, &pattern_state, 0) == pdPASS) {
#if ENABLE_FAST_COLOR_REFRESH == 0
			color_changed = true;
#endif
			/* if pattern has not stopped, display current color.
			 * Else display pattern stopped */
			if (pattern_state != -1) {
				current_color = (uint8_t)pattern_state;
#if ENABLE_FAST_COLOR_REFRESH == 0
				if (configuration.step_mode == MANUAL) {
#endif
					draw_current_color(current_color);
#if ENABLE_FAST_COLOR_REFRESH == 0
				}
#endif
				draw_color_slider(current_color);
			} else {

				/* print pattern stopped */
				configuration.control_mode = STOP;
				print_pattern_state(configuration.control_mode);
				pattern_running = false;
			}
		}

		/* Next frame of arrow animation and redraw current color */
		if (delay_count >= ANIMATE_TICKS) {
			delay_count = 0;
			if (master_mode) {
				animate_arrow();
			}
			changes_cancelled = false;
#if ENABLE_FAST_COLOR_REFRESH == 0
			if (color_changed) {
				draw_current_color(current_color);
				color_changed = false;
			}
#endif
			reset_cursor();
		}

		xSemaphoreGive(console);
		vTaskDelayUntil(&xLastWakeTime, STATUS_UPDATE_TICKS);
	}
}

/**
* @brief Refresh color slider.
*
* Redraws color slider.
*
* @param color color to display.
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void draw_color_slider(uint8_t color)
{
	char pattern_position_str[POSITION_STR_LEN + 4] = PATTERN_POS_STR;

	PRINTF("%s", hide_cursor);
	set_cursor(STATUS_ROW + (2 * LINE_SPACE), strlen(status_name[2]) + STATUS_COL);
	if (color < 255) {
		pattern_position_str[((color * POSITION_STR_LEN) / 255) + 1] = PATTERN_POS_CHAR;
	} else {
		pattern_position_str[POSITION_STR_LEN] = PATTERN_POS_CHAR;
	}
	PRINTF("%s", pattern_position_str);
	reset_cursor();
}

/**
* @brief Refresh current color.
*
* Redraws current color.
*
* @param color color to display.
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void draw_current_color(uint8_t color)
{
	char current_color_str[10] = "(0, 0, 0)";

	PRINTF("%s", hide_cursor);
	set_cursor(STATUS_ROW + (1 * LINE_SPACE), strlen(status_name[1]) + STATUS_COL);
	color_to_str(color, current_color_str);
	PRINTF("%s", current_color_str);
	reset_cursor();
}

/**
* @brief Animate mode arrow.
*
* Animates arrow as step mode selection hind.
* Arrow position is changed each time this function is called.
*
* @param
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void animate_arrow(void)
{
	int16_t row, col;

	PRINTF("%s", hide_cursor);
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
}

/**
* @brief Reset cursor for user input.
*
* Resets cursor position back to the user input field after performing
* other UI object updates.
*
* @param
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
* @brief Switch UI to master_mode.
*
* Non returning function for performing master mode UI operations.
*
* @param
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

	/* print master home screen */
	xSemaphoreTake(console, CONSOLE_SEMAPHORE_WAIT);
	draw_ui();
	print_configurations();
	change_cursor_line(cursor_line);

	/* send configurations once */
	configuration.control_mode = NOP;
	xQueueSend(communication_queue, &configuration, QUEUE_SEND_WAIT);

	for ( ; ; ) {
		xSemaphoreGive(console);
		key_pressed = GETCHAR();
		xSemaphoreTake(console, CONSOLE_SEMAPHORE_WAIT);
		switch (key_pressed) {

			/* if read an escape sequence, check which escape sequence is received */
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
						print_line_error(cursor_line);
						change_cursor_line(--cursor_line);
					}
				} else if (key_pressed == 'B') {
					if (cursor_line < MAX_DOWN) {
						PRINTF("%s", down);
						print_line_error(cursor_line);
						change_cursor_line(++cursor_line);
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
				if (pattern_running) {
					if (pattern_paused) {
						configuration.control_mode = RESUME;
						pattern_paused = false;
					} else {
						configuration.control_mode = PAUSE;
						pattern_paused = true;
					}
					xQueueSend(communication_queue, &configuration, QUEUE_SEND_WAIT);
					print_pattern_state(configuration.control_mode);
				}
				break;

			case '\r' :
				/* send configuration and send START command.
				 * The structure is send twice. Configuration is received if
				 * the control command is zero and vice versa */
				if (encode_config()) {
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
				/* send UP command */
				configuration.control_mode = UP;
				xQueueSend(communication_queue, &configuration, QUEUE_SEND_WAIT);
				break;

			case 'C' :
			case 'c' :
				/* redraw configuration with last applied configuration. */
				if (!changes_cancelled) {
					changes_cancelled = true;
					draw_ui();
					draw_current_color(current_color);
					draw_color_slider(current_color);
					decode_config();
					print_pattern_state(configuration.control_mode);
					print_configurations();
					print_connection_status(device_connected);
					change_cursor_line(cursor_line);
				}
				break;

			case 'X' :
			case 'x' :
				/* redraw configuration with last applied configuration. */
				if (width >= 3) {
					width -= 3;
				}
				draw_ui();
				draw_current_color(current_color);
				draw_color_slider(current_color);
				decode_config();
				print_pattern_state(configuration.control_mode);
				print_configurations();
				print_connection_status(device_connected);
				change_cursor_line(cursor_line);

				break;

			case 'V' :
			case 'v' :
				/* redraw configuration with last applied configuration. */
				if (width <= (23 * 3)) {
					width += 3;
				}
				draw_ui();
				draw_current_color(current_color);
				draw_color_slider(current_color);
				decode_config();
				print_pattern_state(configuration.control_mode);
				print_configurations();
				print_connection_status(device_connected);
				change_cursor_line(cursor_line);
				break;

			default :
				process_ui_input(config_value_str[cursor_line], key_pressed, NO_KEY_PRESSED);
		}
	}
}

/**
* @brief Checks if config is valid.
*
* Checks if within the range of all configuration values.
* Converts all string inputs to binary form.
*
* @param
*
* @return Returns TRUE if valid.
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
_Bool encode_config(void)
{
	_Bool config_valid = true;

	/* check if color is greater than minimum value */
	config_valid &= config_value_str[START_COLOR][RED_OFFSET] >= '0';
	config_valid &= config_value_str[START_COLOR][GREEN_OFFSET] >= '0';
	config_valid &= config_value_str[START_COLOR][BLUE_OFFSET] >= '0';
	config_valid &= config_value_str[STOP_COLOR][RED_OFFSET] >= '0';
	config_valid &= config_value_str[STOP_COLOR][GREEN_OFFSET] >= '0';
	config_valid &= config_value_str[STOP_COLOR][BLUE_OFFSET] >= '0';

	/* check if step value is greater than minimum value */
	config_valid &= config_value_str[STEP_VALUE][RED_OFFSET] >= '0';
	config_valid &= config_value_str[STEP_VALUE][GREEN_OFFSET] >= '0';
	config_valid &= config_value_str[STEP_VALUE][BLUE_OFFSET] >= '0';

	/* check if color is less than maximum value */
	config_valid &= config_value_str[START_COLOR][RED_OFFSET] <= '7';
	config_valid &= config_value_str[START_COLOR][GREEN_OFFSET] <= '7';
	config_valid &= config_value_str[START_COLOR][BLUE_OFFSET] <= '3';
	config_valid &= config_value_str[STOP_COLOR][RED_OFFSET] <= '7';
	config_valid &= config_value_str[STOP_COLOR][GREEN_OFFSET] <= '7';
	config_valid &= config_value_str[STOP_COLOR][BLUE_OFFSET] <= '3';

	/* check if step value is less than maximum value */
	config_valid &= config_value_str[STEP_VALUE][RED_OFFSET] <= '7';
	config_valid &= config_value_str[STEP_VALUE][GREEN_OFFSET] <= '7';
	config_valid &= config_value_str[STEP_VALUE][BLUE_OFFSET] <= '3';

	/* Check if start color less than stop color */
	config_valid &= (config_value_str[START_COLOR][RED_OFFSET] <= config_value_str[STOP_COLOR][RED_OFFSET]);
	config_valid &= (config_value_str[START_COLOR][GREEN_OFFSET] <= config_value_str[STOP_COLOR][GREEN_OFFSET]);
	config_valid &= (config_value_str[START_COLOR][BLUE_OFFSET] <= config_value_str[STOP_COLOR][BLUE_OFFSET]);

	/* Check if step value is 0 */
	if (config_value_str[STEP_VALUE][RED_OFFSET] == '0') {
		config_valid &= config_value_str[START_COLOR][RED_OFFSET] == config_value_str[STOP_COLOR][RED_OFFSET];
	}
	if (config_value_str[STEP_VALUE][GREEN_OFFSET] == '0') {
		config_valid &= config_value_str[START_COLOR][GREEN_OFFSET] == config_value_str[STOP_COLOR][GREEN_OFFSET];
	}
	if (config_value_str[STEP_VALUE][BLUE_OFFSET] == '0') {
		config_valid &= config_value_str[START_COLOR][BLUE_OFFSET] == config_value_str[STOP_COLOR][BLUE_OFFSET];
	}

	/* check if values are greater than minimum */
	config_valid &= atoi(config_value_str[NUMBER_OF_CYCLES]) >= MIN_NO_OF_CYCLES;
	config_valid &= atoi(config_value_str[COLOR_CHANGE_RATE]) >= MIN_CHANGE_RATE;
	config_valid &= atoi(config_value_str[REFRESH_RATE]) >= MIN_REFRESH_RATE;

	/* check if values are less than maximum */
	config_valid &= atoi(config_value_str[NUMBER_OF_CYCLES]) <= MAX_NO_OF_CYCLES;
	config_valid &= atoi(config_value_str[COLOR_CHANGE_RATE]) <= MAX_CHANGE_RATE;
	config_valid &= atoi(config_value_str[REFRESH_RATE]) <= MAX_REFRESH_RATE;

	/* convert screen configuration strings to configuration structure */
	if (config_valid) {
		configuration.start_color[0] = parse_color(config_value_str[START_COLOR]);
		configuration.stop_color[0] = parse_color(config_value_str[STOP_COLOR]);
		configuration.step_value = parse_color(config_value_str[STEP_VALUE]);
		configuration.step_mode = (uint8_t)step_mode;
		configuration.no_of_cycles = atoi(config_value_str[NUMBER_OF_CYCLES]);
		configuration.color_change_rate = atoi(config_value_str[COLOR_CHANGE_RATE]);
		configuration.refresh_rate = atoi(config_value_str[REFRESH_RATE]);
	}
	return config_valid;
}

/**
* @brief Switch to slave mode.
*
* Non returning function for slave mode UI operations.
*
* @param
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void run_slave_ui(void)
{
	xSemaphoreTake(console, CONSOLE_SEMAPHORE_WAIT);
	draw_ui();
	print_configurations();

	xSemaphoreGive(console);
	for (; ;) {
		if (xQueueReceive(communication_queue, &configuration, 0)) {

			/* Check if configuration or control is received and update console */
			if (configuration.control_mode == NOP) {

				decode_config();
				xSemaphoreTake(console, CONSOLE_SEMAPHORE_WAIT);
				print_configurations();
				xSemaphoreGive(console);
			} else {

				/* display control received */
				xSemaphoreTake(console, CONSOLE_SEMAPHORE_WAIT);
				print_pattern_state(configuration.control_mode);
				xSemaphoreGive(console);
			}
		} else {
			taskYIELD();
		}
	}
}

/**
* @brief Decode config data to string form.
*
* Converts config data to string config data to be able to print
* to the console.
*
* @param control	control input received
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void decode_config(void)
{
	color_to_str(configuration.start_color[0], config_value_str[START_COLOR]);
	color_to_str(configuration.stop_color[0], config_value_str[STOP_COLOR]);
	color_to_str(configuration.step_value, config_value_str[STEP_VALUE]);
	if (configuration.step_mode <= MAX_MODE_VALUE) {
		strcpy(config_value_str[STEP_MODE], step_mode_name[configuration.step_mode]);
		step_mode = configuration.step_mode;
	}
	itoa(configuration.no_of_cycles, config_value_str[NUMBER_OF_CYCLES], 10);
	itoa(configuration.color_change_rate, config_value_str[COLOR_CHANGE_RATE], 10);
	itoa(configuration.refresh_rate, config_value_str[REFRESH_RATE], 10);
}

/**
* @brief Prints current pattern state.
*
* Prints if the pattern is currently running or is in any other state.
*
* @param control	control input received
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

	reset_cursor();
}

/**
* @brief Prints connection state.
*
* Prints if slave or master is connected or not in status field.
*
* @param
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void print_connection_status(_Bool status)
{
	PRINTF("%s", hide_cursor);
	set_cursor(STATUS_ROW + (0 * LINE_SPACE), strlen(status_name[0]) + STATUS_COL);

	/* Print status corresponding to master or slave. */
	if (master_mode) {
		if (status) {
			PRINTF("%s", status_const_str[0][1]);
			clear_next(5);
		} else {
			PRINTF("%s", status_const_str[0][0]);
			clear_next(5);
		}
	} else {
		if (status) {
			PRINTF("%s", status_const_str[1][1]);
			clear_next(5);
		} else {
			PRINTF("%s", status_const_str[1][0]);
			clear_next(5);
		}
	}
	reset_cursor();
}

/**
* @brief Prints configuration values.
*
* Redraws all the configuration values with current configuration values.
*
* @param
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void print_configurations(void)
{
	PRINTF("%s", hide_cursor);
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

	reset_cursor();
}

/**
* @brief Move cursor to selected line.
*
* Sets cursor to right position after line is changed.
* Updates config description and range values for the current line.
*
* @param line	 destination line to change to
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void change_cursor_line(uint8_t line)
{
	switch (line) {

		/* Lines for color type input */
		case START_COLOR :
		case STOP_COLOR :
		case STEP_VALUE :
			cursor_pos = 1;
			value_length = 9;
			set_cursor(CONFIG_ROW + (line * LINE_SPACE), strlen(config_name[line]) + 1 + CONFIG_COL);
			break;

		/* Lines for number input */
		case NUMBER_OF_CYCLES :
		case COLOR_CHANGE_RATE :
		case REFRESH_RATE :
			cursor_pos = strlen(config_value_str[line]);
			value_length = cursor_pos;
			set_cursor(CONFIG_ROW + (line * LINE_SPACE), strlen(config_name[line]) + value_length + CONFIG_COL);
			break;

		/* Lines for changing mode */
		case STEP_MODE :
			cursor_pos = strlen(step_mode_name[step_mode]) + 4;
			value_length = cursor_pos;
			set_cursor(CONFIG_ROW + (line * LINE_SPACE), strlen(config_name[line]) + value_length + CONFIG_COL);
	}

	PRINTF("%s", hide_cursor);
	set_cursor(HINT_ROW - (0 * LINE_SPACE), HINT_COL);
	PRINTF("%s", config_description[line][0]);
	set_cursor(HINT_ROW + (3 * LINE_SPACE), HINT_COL);
	PRINTF("%s", config_description[line][1]);

	reset_cursor();
}

/**
* @brief Display error.
*
* Display error message if configuration not valid.
*
* @param line    Configuration line
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void print_line_error(uint8_t line)
{
	_Bool value_valid = true;

	PRINTF("%s", hide_cursor);
	switch (line) {
		case START_COLOR :
			/* check if color is greater than minimum value */
			value_valid &= config_value_str[START_COLOR][RED_OFFSET] >= '0';
			value_valid &= config_value_str[START_COLOR][GREEN_OFFSET] >= '0';
			value_valid &= config_value_str[START_COLOR][BLUE_OFFSET] >= '0';

			/* check if color is less than maximum value */
			value_valid &= config_value_str[START_COLOR][RED_OFFSET] <= '7';
			value_valid &= config_value_str[START_COLOR][GREEN_OFFSET] <= '7';
			value_valid &= config_value_str[START_COLOR][BLUE_OFFSET] <= '3';
			break;

		case STOP_COLOR :
			/* check if color is greater than minimum value */
			value_valid &= config_value_str[STOP_COLOR][RED_OFFSET] >= '0';
			value_valid &= config_value_str[STOP_COLOR][GREEN_OFFSET] >= '0';
			value_valid &= config_value_str[STOP_COLOR][BLUE_OFFSET] >= '0';

			/* check if color is less than maximum value */
			value_valid &= config_value_str[STOP_COLOR][RED_OFFSET] <= '7';
			value_valid &= config_value_str[STOP_COLOR][GREEN_OFFSET] <= '7';
			value_valid &= config_value_str[STOP_COLOR][BLUE_OFFSET] <= '3';
			break;

		case STEP_VALUE :
			/* check if step value is greater than minimum value */
			value_valid &= config_value_str[STEP_VALUE][RED_OFFSET] >= '0';
			value_valid &= config_value_str[STEP_VALUE][GREEN_OFFSET] >= '0';
			value_valid &= config_value_str[STEP_VALUE][BLUE_OFFSET] >= '0';

			/* check if step value is less than maximum value */
			value_valid &= config_value_str[STEP_VALUE][RED_OFFSET] <= '7';
			value_valid &= config_value_str[STEP_VALUE][GREEN_OFFSET] <= '7';
			value_valid &= config_value_str[STEP_VALUE][BLUE_OFFSET] <= '3';

			/* Check if step value is 0 */
			if (config_value_str[STEP_VALUE][RED_OFFSET] == '0') {
				value_valid &= config_value_str[START_COLOR][RED_OFFSET] == config_value_str[STOP_COLOR][RED_OFFSET];
			}
			if (config_value_str[STEP_VALUE][GREEN_OFFSET] == '0') {
				value_valid &= config_value_str[START_COLOR][GREEN_OFFSET] == config_value_str[STOP_COLOR][GREEN_OFFSET];
			}
			if (config_value_str[STEP_VALUE][BLUE_OFFSET] == '0') {
				value_valid &= config_value_str[START_COLOR][BLUE_OFFSET] == config_value_str[STOP_COLOR][BLUE_OFFSET];
			}

			if (value_valid) {
				set_cursor(ERROR_ROW + 2, ERROR_COL + 2);
				clear_next(45);
			} else {
				set_cursor(ERROR_ROW + 2, ERROR_COL + 2);
				PRINTF("Start color must be end color if step is 0.");
			}
			break;

		case NUMBER_OF_CYCLES :
			/* check if no of cycles is in range */
			value_valid &= atoi(config_value_str[NUMBER_OF_CYCLES]) >= MIN_NO_OF_CYCLES;
			value_valid &= atoi(config_value_str[NUMBER_OF_CYCLES]) <= MAX_NO_OF_CYCLES;
			break;

		case COLOR_CHANGE_RATE :
			/* check if color change rate is in range */
			value_valid &= atoi(config_value_str[COLOR_CHANGE_RATE]) >= MIN_CHANGE_RATE;
			value_valid &= atoi(config_value_str[COLOR_CHANGE_RATE]) <= MAX_CHANGE_RATE;
			break;

		case REFRESH_RATE :
			/* check if refresh rate is in range */
			value_valid &= atoi(config_value_str[REFRESH_RATE]) >= MIN_REFRESH_RATE;
			value_valid &= atoi(config_value_str[REFRESH_RATE]) <= MAX_REFRESH_RATE;
	}

	/* Check if start color less than stop color */
	if (value_valid && (line == START_COLOR || line == STOP_COLOR)) {
		value_valid &= (config_value_str[START_COLOR][RED_OFFSET] <= \
				config_value_str[STOP_COLOR][RED_OFFSET]);
		value_valid &= (config_value_str[START_COLOR][GREEN_OFFSET] <= \
				config_value_str[STOP_COLOR][GREEN_OFFSET]);
		value_valid &= (config_value_str[START_COLOR][BLUE_OFFSET] <= \
				config_value_str[STOP_COLOR][BLUE_OFFSET]);

		if (value_valid) {
			set_cursor(ERROR_ROW + 1, ERROR_COL + 2);
			clear_next(45);
		} else {
			set_cursor(ERROR_ROW + 1, ERROR_COL + 2);
			PRINTF("Start color must be less than end color.");
		}
	}

	/* Print line Error indicator */
	if (!value_valid) {
		set_cursor(CONFIG_ROW + line, CONFIG_COL - 1 + strlen(config_name[line]));
		PRINTF("*");
	} else if (value_valid && line != STEP_MODE) {
		set_cursor(CONFIG_ROW + line, CONFIG_COL - 1 + strlen(config_name[line]));
		PRINTF(" ");
	}

	/* Print error message */
	if (encode_config()) {
		set_cursor ERROR_ROW_COL;
		clear_next(23);
	} else {
		set_cursor ERROR_ROW_COL;
		PRINTF("* %s", pattern_state_const[4]);
	}

	reset_cursor();
}

/**
* @brief Process key input.
*
* Key pressed is treated differently based on line selected.
*
* @param str             string to process
* @param key_pressed     character input
* @param arrow_pressed	 arrow key pressed
*
* @note
*
* Revision History:
* - 171221 ATG: Creation Date
*/
void process_ui_input(char* str, char key_pressed, arrow_key_type arrow_pressed)
{
	switch (cursor_line) {

		/* Lines for color type input */
		case START_COLOR :
		case STOP_COLOR :
		case STEP_VALUE :
			read_color_input(str, key_pressed, arrow_pressed);
			break;

		/* Lines for number input  */
		case NUMBER_OF_CYCLES :
		case COLOR_CHANGE_RATE :
		case REFRESH_RATE :
			read_number_input(str, key_pressed, arrow_pressed);
			break;

		/* Lines for mode input */
		case STEP_MODE :
			read_mode_input(str, key_pressed, arrow_pressed);
	}
}

/**
* @brief Reads color in RGB format.
*
* Brackets and commas in color format are skipped on input.
*
* @param str             string to process
* @param key_pressed     character input
* @param arrow_pressed	 arrow key pressed
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
* @brief Read step mode type input.
*
* Arrow keys are read to change mode.
*
* @param str             string to process
* @param key_pressed     character input
* @param arrow_pressed	 arrow key pressed
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
				change_cursor_line(cursor_line);
				config_edited = true;
			}
			break;

		case RIGHT_KEY :
			if (step_mode < MANUAL) {
				step_mode++;
				set_cursor(CONFIG_ROW + (MODE_LINE * LINE_SPACE), strlen(config_name[cursor_line]) + CONFIG_COL + 2);
				PRINTF("%s", step_mode_name[step_mode]);
				clear_next(8);
				change_cursor_line(cursor_line);
				config_edited = true;
			}
			break;

		default :;
	}
}

/**
* @brief Text editor interface.
*
* Read number through text editor interface.
*
* @param str             string to process
* @param key_pressed     character input
* @param arrow_pressed	 arrow key pressed
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
* @brief Convert color in string to binary.
*
* Parse color from (R, G, B) formated string.
*
* @param color_str	source string to process.
* @return result 	Converted color output.
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
* @brief 8-bit color to formated string.
*
* Converts *bit color to (R, G, B) formated string.
*
* @param color	 source color.
* @param str_out destination string.
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
* @brief Set cursor to a position.
*
* Set cursor position to a position relative to home position.
*
* @param row	line number from top.
* @param col 	columns from left.
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
	PRINTF("%s%c", row_str, coordinates[3]);
	PRINTF("%s%c", col_str, coordinates[5]);
}

/**
* @brief Moves cursor left.
*
* Moves cursor a number of times in a single shot.
*
* @param no_of_times 	times to move left
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
* @brief Insert character to string.
*
* Insert character in a position between a string.
*
* @param str	 	 string to process
* @param position	 position to insert
* @param input		 character to be inserted
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
* @brief Delete character from string.
*
* Delete character from a position between a string.
*
* @param str	 	 string to process
* @param position	 position to insert
* @param input		 character to be inserted
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
* @brief Draw ASCII square.
*
* Uses ASCII characters to draw a doted square.
*
* @param length  	Length of the square.
* @param breadth    Breadth of the square.
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
* @brief Draw ASCII square.
*
* Uses ASCII characters to draw a square.
*
* @param length  	Length of the square.
* @param breadth    Breadth of the square.
*
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
* @brief Clear characters.
*
* Clear remaining characters number of characters.
*
* @param length		number of characters to clear.
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
