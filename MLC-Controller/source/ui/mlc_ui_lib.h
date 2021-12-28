/**
 * @file mlc_common.h
 * @brief mlc header.
 *
 * @note
 *
 * Revision History:
 *	- 091221 ATG : Creation Date
 */

#ifndef MLC_UI_DRIVERS_H_
#define MLC_UI_DRIVERS_H_

#include "mlc_common.h"

/***********************************
* Const and Macro Defines
***********************************/
#define STATUS_UPDATE_TICKS 10
#define ANIMATE_TICKS 10
#define MAX_INPUT_LENGTH    5
#define CONFIG_VALUE_LENGTH 15
#define TITLE_SQUARE_SIZE   20
#define SMALL_SQUARE_SIZE   21
#define LINE_SPACE          1

#define CONFIG_ROW            20
#define CONFIG_COL            5
#define CONFIG_ROW_COL        (CONFIG_ROW, CONFIG_COL)
#define STATUS_ROW            12
#define STATUS_COL            5
#define STATUS_ROW_COL        (STATUS_ROW, STATUS_COL)
#define PATTERN_STATE_ROW_COL (10, 49)
#define HEAD_1_ROW_COL        (3, 25)
#define HEAD_2_ROW_COL        (4, 25)
#define HINT_ROW              24
#define HINT_COL              49
#define POSITION_STR_LEN 	  21

#define MIN_STEP_VALUE   1
#define MAX_STEP_VALUE   255
#define MIN_NO_OF_CYCLES 1
#define MAX_NO_OF_CYCLES 100
#define MIN_CHANGE_RATE  1
#define MAX_CHANGE_RATE  500
#define MIN_REFRESH_RATE 1
#define MAX_REFRESH_RATE 9999
#define MAX_MODE_VALUE   4

#define MAX_UP          0
#define MAX_DOWN        6
#define MODE_LINE       3
#define STATUS_COUNT    3
#define BACKSPACE       '\b'
#define SEQUENCE        '\e'
#define RED_OFFSET      1
#define GREEN_OFFSET    4
#define BLUE_OFFSET     7
#define QUEUE_SEND_WAIT 200 * 20
#define PATTERN_POS_CHAR 'v'
#define PATTERN_POS_STR "[_____________________]"

const char up[4] = 			"\e[A"; 		//{0x1B, 0x5B, 'A'};
const char down[4] =		"\e[B"; 		//{0x1B, 0x5B, 'B'};
const char right[4] =		"\e[C"; 		//{0x1B, 0x5B, 'C'};
const char left[4] = 		"\e[D"; 		//{0x1B, 0x5B, 'D'};
const char clear_end[4] = 	"\e[K"; 		//{0x1B, 0x5B, 0x4B};
const char clear[5] = 		"\e[2J"; 		//{0x1B, 0x5B, 0x32, 0x4A};
const char coordinates[7] = "\e[1;1H"; 		//{0x1B, 0x5B, 0x39, 0x3B, 0x39, 0x48};
const char hide_cursor[7] = "\e[?25l";
const char show_cursor[7] = "\e[?25h";
const char save_cursor[4] = "\e[s";
const char load_cursor[4] = "\e[u";

const char title_1[] =      "DAK Technologies Pvt Ltd";
const char title_2[] =      "Multicolor LED Controller";
const char master_name[] =  "MASTER";
const char slave_name[] =   "SLAVE";
const char status_title[] = "Status";
const char config_title[] = "Configuration";


const char status_name[][20] = {"Connection: ",
							    "Current Color: ",
							    "Color Position: ",
							    "Status 4"
							    };

const char status_const_str[][2][25] = {{"Slave not connected", "Slave Connected"},
									    {"Master not connected", "Master Connected"}
									   };

const char config_name[][20] = {"1. Start Color:  ",
						   	   	"2. End Color:    ",
								"3. Step Value:   ",
								"5. Step Mode:  ",
								"4. No. of cycles ",
								"6. Change Rate:  ",
								"7. Refresh Rate: "
						 	    };
const char config_description[][2][50] = {{" (R, G, B)       ", " (0,0,0) - (7,7,3)"},
										  {" (R, G, B)       ", " (0,0,0) - (7,7,3)"},
										  {" Steps per change", " 1 - 255          "},
										  {" UP/DOWN/Manual  ", " 4 Modes          "},
										  {" 0 is continuous ", " 0 - 100          "},
										  {" Execution Rate  ", " 1 - 500          "},
										  {" PWM frequency   ", " 1 - 9999         "}
										};

const char step_mode_name[][CONFIG_VALUE_LENGTH] = {"0.Mode X",
													"1.Auto Up",
													"2.Auto Down",
													"3.Auto Up-Down",
													"4.Manual"
													};
const char pattern_state_const[][22] = {"   Pattern Stopped   ",
										"   Pattern Running   ",
										"   Pattern Paused    ",
										"     Manual Mode     ",
										"Invalid configuration"
										};

/***********************************
* Typedefs and Enum Declarations
***********************************/
typedef enum {
	NO_KEY_PRESSED,
	UP_KEY,
	DOWN_KEY,
	LEFT_KEY,
	RIGHT_KEY
} arrow_key_type;


/***********************************
* Const Declarations
***********************************/
// none

/***********************************
* Variable Declarations
***********************************/
// none

/***********************************
* Prototypes
***********************************/
void set_cursor(uint16_t row, uint16_t col);
void move_cursor_left(uint16_t no_of_times);
void insert(char* str, uint16_t position, char input);
void delete(char* str, uint16_t position);
void clear_next(uint8_t length);
void draw_dotted_square(uint16_t length, uint16_t breadth);
void draw_square(uint16_t length, uint16_t breadth);

#endif /* MLC_UI_DRIVERS_H_ */
