/*
 * pattern_executor.h
 *
 *  Created on: 15-Dec-2021
 *      Author: karthik
 */

#ifndef PATTERN_EXECUTOR_H_
#define PATTERN_EXECUTOR_H_



/**
 * @file	pattern_executor.h
 * @brief	LED execution HAL header
 *
 * Header file for Pattern execution
 *
 * @note
 *
 * Revision History:
 *	  - 141221 KMO : Creation Date
 */

/* The Flextimer instance/channel used for board */
#define BOARD_FTM_BASEADDR 	FTM3
#define BOARD_FTM_CHANNEL1  kFTM_Chnl_1   // PTD1 12 BLUE
#define BOARD_FTM_CHANNEL2  kFTM_Chnl_2  //PTD2 pin 8 RED
#define BOARD_FTM_CHANNEL3  kFTM_Chnl_3  //PTD3 pin 10 GREEN

#define MLC_FTM 			FTM3
#define FTM_BLUE_CHANNEL    kFTM_Chnl_1   // PTD1 12 BLUE
#define FTM_RED_CHANNEL     kFTM_Chnl_2  //PTD2 pin 8 RED
#define FTM_GREEN_CHANNEL   kFTM_Chnl_3  //PTD3 pin 10 GREEN


/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

#define	RED_MASK_VAL	224
#define	RED_SHIFT_VAL	5

#define GREEN_MASK_VAL	28
#define GREEN_SHIFT_VAL 2

#define BLUE_MASK_VAL	3
#define BLUE_SHIFT_VAL	0

#define TIMER_PERIOD		2
#define BlOCKED_TIME_PERIOD	1
#define TICK_PERIOD 		5	/* One tick takes 1/configTICK_RATE_HZ */


#define THREAD_LOCAL_STORAGE_INDEX	0


/* Freescale includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "board.h"
#include "fsl_ftm.h"
#include "mlc_common.h"


/* FreeRTOS kernel includes. */
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

/***********************************
* Const and Macro Defines
***********************************/

#define DUTY_CYCLE_RED(x)	(((x & RED_MASK_VAL) >> RED_SHIFT_VAL) * 100) / 7
#define DUTY_CYCLE_GREEN(x)	(((x & GREEN_MASK_VAL) >> GREEN_SHIFT_VAL) * 100) / 7
#define DUTY_CYCLE_BLUE(x)	(((x & BLUE_MASK_VAL) >> BLUE_SHIFT_VAL) * 100) / 3

#define FREQUENCY_LIMIT(x)  ((x < 45000)? 1 : 0)

/***********************************
* Typedefs and Enum Declarations
***********************************/
typedef int16_t color_type;



/***********************************
* Const Declarations
***********************************/

// none

/***********************************
* Variable Declarations
***********************************/
//none

/***********************************
* Prototypes
***********************************/

//void led_execution(led_config_type * );
//
///* FTM related functions*/
//
//void get_default_config(led_config_type *);
void ftm_init(uint16_t);
void pwm_up_execution(uint8_t, uint8_t,uint8_t);
void pwm_down_execution(uint8_t, uint8_t,uint8_t);
void update_color(color_type);




/* Timer callback function */

void check_control_mode(TimerHandle_t );

/*Task callback functions */

void control_task(void *);
void dummy_task(void *);



#endif /* PATTERN_EXECUTOR_H_ */
