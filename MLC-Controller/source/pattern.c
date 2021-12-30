/**
 * @file    MLC-Controller.c
 * @brief   Application entry point.
 */

#include "mlc_common.h"
#include "fsl_ftm.h"

/*******************************************
 * Const and Macro Defines
 *******************************************/

#define	RED_MASK_VAL	0b11100000
#define	RED_SHIFT_VAL	5

#define GREEN_MASK_VAL	0b00011100
#define GREEN_SHIFT_VAL 2

#define BLUE_MASK_VAL	0b00000011
#define BLUE_SHIFT_VAL	0

#define DUTY_CYCLE_RED(x)	(((x & RED_MASK_VAL) >> RED_SHIFT_VAL) * 100) / 7
#define DUTY_CYCLE_GREEN(x)	(((x & GREEN_MASK_VAL) >> GREEN_SHIFT_VAL) * 100) / 7
#define DUTY_CYCLE_BLUE(x)	(((x & BLUE_MASK_VAL) >> BLUE_SHIFT_VAL) * 100) / 3

#define MLC_FTM 			FTM3
#define FTM_BLUE_CHANNEL    kFTM_Chnl_1  /* PTD1 12 BLUE */
#define FTM_RED_CHANNEL     kFTM_Chnl_2  /* PTD2 pin 8 RED */
#define FTM_GREEN_CHANNEL   kFTM_Chnl_3  /* PTD3 pin 10 GREEN */

#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

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
// none

/***********************************
 * Public Variables
 ***********************************/
static _Bool master = true;
static _Bool going_up = true;
static led_config_type config;
static color_type current_color = 0;
static TimerHandle_t color_change_timer;
static uint8_t count = 0;
static color_type min_color = 0, max_color = 0;

/***********************************
 * Private Variables
 ***********************************/
static QueueHandle_t pattern_status_queue;
static QueueHandle_t pattern_control_queue;

/***********************************
 * Private Prototypes
 ***********************************/
void color_timer(TimerHandle_t xTimer);
color_type next_color(color_type);
color_type previous_color(color_type);
color_type start_color(void);
void set_pwm_frequency(uint16_t frequency);
void set_color(color_type color);
void pwm_init(ftm_chnl_t channel, uint16_t frequency);

/***********************************
 * Public Functions
 ***********************************/
void pattern_executor_task(void* master_mode)
{
	color_change_timer = xTimerCreate("Color change timer", 100, pdTRUE, NULL, color_timer);
	pattern_control_queue = get_queue_handle(PATTERN_CONTROL_QUEUE);
	pattern_status_queue = get_queue_handle(PATTERN_STATUS_QUEUE);
	xTimerStop(color_change_timer, 0);
	master = *((_Bool*)master_mode);

	config.stop_color[0] = 255;
	config.start_color[0] = 0;
	config.step_value = 1;
	config.color_change_rate = 100;
	config.step_mode = AUTO_UP;
	config.no_of_cycles = 1;
	config.refresh_rate = 100;
	config.color_scheme = EIGHT_BIT_TRUE_COLOR;
	config.control_mode = 0;

	led_config_type received_config;
	while (1) {
		if (xQueueReceive(pattern_control_queue, &received_config, 0)) {
			if (received_config.control_mode == NOP) {
				config = received_config;
				set_pwm_frequency(100000 / config.refresh_rate);
				xTimerChangePeriod(color_change_timer,(config.color_change_rate * 2), 0);

				if (config.step_mode == MANUAL) {
					xTimerStop(color_change_timer, 0);
				}

				min_color = config.start_color[0];
				max_color = config.stop_color[0];
				if (config.start_color[0] > config.stop_color[0]) {
					max_color |= 0x100;
				}

			} else {
				switch (received_config.control_mode) {
					case START :
						count = 0;
						current_color = start_color();
						set_color(current_color);
						if (going_up) {
							current_color = next_color(current_color);
						} else {
							current_color = previous_color(current_color);

						}
						xQueueOverwrite(pattern_status_queue, (uint8_t *)&current_color);
						if (config.step_mode != MANUAL) {
							xTimerReset(color_change_timer, 0);
							xTimerStart(color_change_timer, 0);
						}
						break;

					case STOP :
						current_color = 0;
						xTimerStop(color_change_timer, 0);
						set_color(current_color);
						xQueueOverwrite(pattern_status_queue, (uint8_t *)&current_color);
						break;

					case PAUSE:
						if (config.step_mode != MANUAL) {
							xTimerStop(color_change_timer, 0);
						}
						break;

					case RESUME :
						if (config.step_mode != MANUAL) {
							xTimerStart(color_change_timer, 0);
						}
						break;

					case UP :
						if (config.step_mode == MANUAL) {
							current_color = next_color(current_color);
							set_color(current_color);
							xQueueOverwrite(pattern_status_queue, (uint8_t *)&current_color);
						}
						break;

					case DOWN :
						if (config.step_mode == MANUAL) {
							current_color = previous_color(current_color);
							set_color(current_color);
							xQueueOverwrite(pattern_status_queue, (uint8_t *)&current_color);
						}
						break;
				}

			}
		} else {
			taskYIELD();
		}
	}
}

void color_timer(TimerHandle_t timer1)
{
	if (count < config.no_of_cycles || config.no_of_cycles == 0) {
		set_color(current_color);
		xQueueOverwrite(pattern_status_queue, (uint8_t *)&current_color);

		if (going_up) {
			current_color = next_color(current_color);
		} else {
			current_color = previous_color(current_color);
		}
	} else {
		xTimerStop(color_change_timer, 0);
	}
}

color_type start_color(void)
{
	color_type color_out = 0;

	switch (config.step_mode) {
		case AUTO_UP :
			going_up = true;
			color_out = min_color;
			break;

		case AUTO_DOWN :
			going_up = false;
			color_out = max_color;
			break;

		case AUTO_UP_DOWN :
		case MANUAL:
			if (master) {
				going_up = true;
				color_out = min_color;
			} else {
				going_up = false;
				color_out = max_color;
			}
			break;
	}

	return color_out;
}

color_type next_color(color_type color)
{
	color += config.step_value;

	if (color == max_color + config.step_value) {
		if (config.step_mode == AUTO_UP_DOWN) {
			going_up = false;
			if (!master) {
				count++;
			}
		} else {
			color = min_color;
			count++;
		}
	} else if (color > max_color) {
		color = max_color;
	}

	return color;
}

color_type previous_color(color_type color)
{
	color -= config.step_value;

	if (color == min_color - config.step_value) {
		if (config.step_mode == AUTO_UP_DOWN) {
			going_up = true;
			if (master) {
				count++;
			}
		} else {
			color = max_color;
			count++;
		}
	} else if (color < min_color) {
		color = min_color;
	}

	return color;
}

void set_pwm_frequency(uint16_t frequency)
{
    ftm_config_t ftm_config;

    FTM_GetDefaultConfig(&ftm_config);
    if(frequency < 45000){
    	ftm_config.prescale = kFTM_Prescale_Divide_128;
    } else {
    	ftm_config.prescale = kFTM_Prescale_Divide_32;
    }
    FTM_Init(FTM3, &ftm_config);

    /*  */
	pwm_init(FTM_RED_CHANNEL, frequency);
	pwm_init(FTM_GREEN_CHANNEL, frequency);
	pwm_init(FTM_BLUE_CHANNEL, frequency);
	FTM_StartTimer(MLC_FTM, kFTM_SystemClock);
}

void pwm_init(ftm_chnl_t channel, uint16_t frequency)
{
	ftm_chnl_pwm_signal_param_t pwm_config;

	pwm_config.chnlNumber = channel;
	pwm_config.dutyCyclePercent = 0;
	pwm_config.level = kFTM_LowTrue;
	pwm_config.firstEdgeDelayPercent = 0;
	pwm_config.enableComplementary = false;
	pwm_config.enableDeadtime = false;

	FTM_SetupPwm(MLC_FTM, &pwm_config, 1, kFTM_EdgeAlignedPwm, frequency, FTM_SOURCE_CLOCK);
}

void set_color(color_type color)
{
	color = (uint8_t)color;
  FTM_UpdatePwmDutycycle(MLC_FTM, FTM_RED_CHANNEL, kFTM_EdgeAlignedPwm, DUTY_CYCLE_RED(color));
  FTM_UpdatePwmDutycycle(MLC_FTM, FTM_GREEN_CHANNEL, kFTM_EdgeAlignedPwm, DUTY_CYCLE_GREEN(color));
  FTM_UpdatePwmDutycycle(MLC_FTM, FTM_BLUE_CHANNEL, kFTM_EdgeAlignedPwm, DUTY_CYCLE_BLUE(color));
  FTM_SetSoftwareTrigger(MLC_FTM, true);
}