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

/* MLC includes. */
#include "pattern_executor.h"

/***********************************
* Const and Macro Defines
***********************************/

// none

/***********************************
* Typedefs and Enum Declarations
***********************************/
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

typedef enum {
	COMMUNICATION_QUEUE,
	SLAVE_STATUS_QUEUE
} queue_enum;

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
QueueHandle_t get_queue_handle(queue_enum);

#endif /* MLC_COMMON_H_ */
