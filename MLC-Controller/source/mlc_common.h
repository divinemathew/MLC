/**
 * @file mlc_common.h
 * @brief mlc header.
 *
 * @note
 *
 * Revision History:
 *	- 091221 ATG : Creation Date
 */
#ifndef MLC_COMMON_H_
#define MLC_COMMON_H_

/* Freescale includes. */
#include "board.h"
#include "pin_mux.h"
#include "MK64F12.h"
#include "peripherals.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

/* FreeRTOS kernel includes. */
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

/***********************************
* Const and Macro Defines
***********************************/
// none

/***********************************
* Typedefs and Enum Declarations
***********************************/
typedef enum{
	CONTINOUS,
	FIXED
} no_of_cycles_enum;

typedef enum{
	EIGHT_BIT_TRUE_COLOR,
	TWENTY_FOUR_BIT_RGB_SCHEME
} color_scheme_enum;

typedef enum {
	COMMUNICATION_QUEUE,
	DEVICE_STATUS_QUEUE,
	PATTERN_STATUS_QUEUE,
	PATTERN_CONTROL_QUEUE
} queue_enum;

typedef enum {
	DEFAULT,
	AUTO_UP,
	AUTO_DOWN,
	AUTO_UP_DOWN,
	MANUAL
} step_mode_enum;

typedef enum {
	START_COLOR,
	STOP_COLOR,
	STEP_VALUE,
	STEP_MODE,
	NUMBER_OF_CYCLES,
	COLOR_CHANGE_RATE,
	REFRESH_RATE,
	COLOR_SCHEME,
	CONTROL_MODE
} config_name_enum;

typedef enum {
	NOP,
	START,
	STOP,
	PAUSE,
	RESUME,
	UP,
	DOWN
} control_mode_enum;

typedef struct {
	uint8_t start_color[3];
	uint8_t stop_color[3];
	uint8_t step_value;
	uint8_t step_mode;
	uint8_t no_of_cycles;
	uint16_t color_change_rate;
	uint16_t refresh_rate;
	uint8_t color_scheme;
	uint8_t control_mode;
} led_config_type;

typedef struct {
	uint8_t start_color[3];
	uint8_t stop_color[3];
	uint8_t step_value;
	uint8_t step_mode;
	uint8_t no_of_cycles;
	uint16_t color_change_rate;
	uint16_t refresh_rate;
	uint8_t color_scheme;
} xfer_data;


typedef int16_t color_type;

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
void ui_handler_task(void* board_is_master);
void communication_task(void* pvParameter);
void pattern_executor_task(void *);
QueueHandle_t get_queue_handle(queue_enum);

#endif /* MLC_COMMON_H_ */
