/**
 * @file    Pattern_Executor.c
 * @brief   Application entry point.
 */
/*#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"*/
#include "pattern_executor.h"

/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */
extern TaskHandle_t 	pattern_executor_handler;
TaskHandle_t 	dummy_task_handler;
QueueHandle_t 	pattern_control_queue;
QueueHandle_t 	pattern_status_queue;
TimerHandle_t	check_control_timer;

/*void pattern_executor_task(void *);
void control_task(void *);
void dummy_task(void *);*/

void get_default_config(led_config_type *);
void led_execution(led_config_type *config);


led_config_type *local_config;


/*
 * @brief   Application entry point.
 */

void pattern_executor_task(void *pvParameters)
{
	led_config_type *received_config;
	local_config=(led_config_type *)malloc(sizeof(led_config_type));
	received_config=malloc(sizeof(led_config_type));

	get_default_config(local_config);
	pattern_control_queue = get_queue_handle(PATTERN_CONTROL_QUEUE);
	pattern_status_queue = get_queue_handle(PATTERN_STATUS_QUEUE);

	while(1){
//		xQueueReceive(pattern_control_queue, local_config, 10000);
		led_execution(local_config);

		if(xQueueReceive(pattern_control_queue, received_config, 0)==pdPASS){
			/* 2 Ticks equivalent to 10ms */
			if(0&received_config->control_mode){
				/* control mode when control field !=0 */

				/* copy configuration to local structure */
				memcpy(local_config, received_config, sizeof(led_config_type));

				/* call timer and perform callback */
				if(xTimerStart(check_control_timer,BlOCKED_TIME_PERIOD)==pdPASS){
					/* timer started */
				}

			} else{
			  /* configuration */
			  /* copy configuration to local structure */
				memcpy(local_config, received_config, sizeof(led_config_type));
			  /* call led_execution() */
				led_execution(local_config);

			}
		} else {
			/* queue is empty */
			/* execute with current configuration */
			//led_execution(local_config);

		}
	}
}


//void dummy_task(voireceived_configd *pvParameters)
//{
//
//	led_config_type *led_param;
//	led_param=pvPortMalloc(sizeof(led_config_type));
//
//	while(1){
//		////PRINTF("here dummy\n\r");
//		led_param->start_color[0]=10;
//		led_param->stop_color[0]=25;
//		led_param->step_value=1;
//		led_param->no_of_cycles=1;
//		led_param->step_mode=2;
//		led_param->color_change_rate=50;
//		led_param->refresh_rate=1000;
//		led_param->control_mode=0;
//
//		xQueueSend(pattern_control_queue,led_param,0);
//	}
//
//
//}


void led_execution(led_config_type *config)
{

	uint8_t checkmode,controlmode;
	uint8_t currentcolor,cycles;

	cycles=config->no_of_cycles;

	//PRINTF("led execution\n\r");

	ftm_init(config->refresh_rate);

	/* check whether mode is manual or auto */
	checkmode= config->step_mode & (MANUAL);

	/* Check which mode in manual mode */
	controlmode=config->control_mode & (UP | DOWN);

	//PRINTF("%u\n\r",controlmode);
	//PRINTF("%u\n\r",checkmode);
	/* Auto Mode */
	if(checkmode==0){
		checkmode=config->step_mode & (AUTO_UP | AUTO_DOWN | AUTO_UP_DOWN);
		switch(checkmode){
		 case AUTO_UP:
		  if(config->no_of_cycles==CONTINOUS){
			  while(1){
				  pwm_up_execution(config->start_color[0],config->stop_color[0],config->step_value);

			  }
		  } else {
			  while(config->no_of_cycles){
				  pwm_up_execution(config->start_color[0],config->stop_color[0],config->step_value);
				  --(config->no_of_cycles);
			   }
		  }

	      break;
		 case AUTO_DOWN:
		  if(config->no_of_cycles==CONTINOUS){
			  while(1){
				  pwm_down_execution(config->stop_color[0],config->start_color[0],config->step_value);
			  }
		  } else {
			  while(cycles){
				  //PRINTF("NO OF CYCLES:%u",cycles);
				  pwm_down_execution(config->stop_color[0],config->start_color[0],config->step_value);
				  (cycles)--;
			  }
		  }
		  ////PRINTF("\n\rAUTO DOWN\n\r");
		  break;
		 case AUTO_UP_DOWN:
		  if(config->no_of_cycles==CONTINOUS){
			  while(1){

				  pwm_up_execution(config->start_color[0],config->stop_color[0]-1,config->step_value);
				  pwm_down_execution(config->stop_color[0],config->start_color[0],config->step_value);
			  }
		  } else {
			  while(config->no_of_cycles){
				  pwm_up_execution(config->start_color[0],config->stop_color[0]-1,config->step_value);
				  pwm_down_execution(config->stop_color[0],config->start_color[0],config->step_value);

				  --(config->no_of_cycles);
			  }
		  }
		  break;
		 default:
		  break;
		}
	} /*else{
	  /* Manual mode */

//	  currentcolor=*((uint8_t *)pvTaskGetThreadLocalStoragePointer(pattern_executor_handler,\
//			  THREAD_LOCAL_STORAGE_INDEX));
//	  if(controlmode== UP){
//		  pwm_up_execution(currentcolor, currentcolor+1,1);	/*check for step val */
//	  } else if(controlmode == DOWN) {
//		pwm_down_execution(currentcolor, currentcolor-1,1);
//	  }
//	}

}

void pwm_up_execution(uint8_t init, uint8_t final,uint8_t res)
{
	uint8_t current_color,mod;
	uint16_t delay;


	/* delay is change rate / */
	delay=local_config->color_change_rate/TICK_PERIOD;

	current_color=init;

	if(final>init){
		if((mod=((final-init)%res))!=0){
			final-=mod;
		}
	} else {
		if((mod=(((255-init)+final+1)%res))!=0){
			final-=mod;
		}
	}

	while(current_color!=final){

		// storing current color in Thread Local Storage index 0
		 vTaskSetThreadLocalStoragePointer(pattern_executor_handler,\
				 THREAD_LOCAL_STORAGE_INDEX, (void *)&current_color);

		 if(xQueueSend(pattern_status_queue,&current_color,0)==pdPASS){
			 // data sent successfully to queue
		 } else {
			 // queue is full
		 }

		 //PRINTF("\n\rcurrent color-%u\n\r",current_color);

		 update_color(current_color);
		 current_color+=res;
	     vTaskDelay(delay);

	}
	//PRINTF("\n\rcurrent color-%u\n\r",current_color);
	update_color(current_color);
}

void pwm_down_execution(uint8_t init, uint8_t final, uint8_t res)
{

	uint8_t current_color,mod;
	uint16_t delay;

	delay=local_config->color_change_rate/TICK_PERIOD;

	if(init>final){
		if((mod=((init-final)%res))!=0){
			final+=mod;
		}
	} else{
	  if((mod=((255-final)+init+1)%res)!=0){
			final+=mod;
	  }
	}

	current_color=init;

	while(current_color!=final){

		/* storing current color in Thread Local Storage index 0 */
		 vTaskSetThreadLocalStoragePointer(pattern_executor_handler,\
				 THREAD_LOCAL_STORAGE_INDEX, (void *)&current_color);

		 if(xQueueSend(pattern_status_queue,&current_color,0)==pdPASS){
			 /* data sent successfully to queue */
		 } else {
			 /* queue is full */
		 }
		 //PRINTF("\n\rcurrent color-%u\n\r",current_color);

		 update_color(current_color);

		 current_color-=res;

	    vTaskDelay(delay);

	}
	//PRINTF("\n\rcurrent color-%u\n\r",current_color);
	update_color(current_color);

}

void ftm_init(uint16_t frequency)
{
    ftm_config_t ftmInfo;
    ftm_chnl_pwm_signal_param_t ftmParam[4];
    //ftm_pwm_level_select_t pwmLevel = kFTM_LowTrue;


    FTM_GetDefaultConfig(&ftmInfo);

    if(FREQUENCY_LIMIT(frequency)){
    	ftmInfo.prescale=kFTM_Prescale_Divide_128;
    } else {
    	ftmInfo.prescale=kFTM_Prescale_Divide_32;
    }

    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);



	ftmParam[1].chnlNumber=BOARD_FTM_CHANNEL1;
	ftmParam[2].chnlNumber=BOARD_FTM_CHANNEL2;
	ftmParam[3].chnlNumber=BOARD_FTM_CHANNEL3;


	ftmParam[2].dutyCyclePercent=DUTY_CYCLE_RED(((local_config->start_color[0] & RED_MASK_VAL) >> RED_SHIFT_VAL));
	ftmParam[3].dutyCyclePercent=DUTY_CYCLE_RED(((local_config->start_color[0] & GREEN_MASK_VAL) >> GREEN_SHIFT_VAL));
	ftmParam[1].dutyCyclePercent=DUTY_CYCLE_BLUE(((local_config->start_color[0] & BLUE_MASK_VAL) >> BLUE_SHIFT_VAL));


	ftmParam[1].level=kFTM_LowTrue;
	ftmParam[2].level=kFTM_LowTrue;
		ftmParam[3].level=kFTM_LowTrue;

	ftmParam[1].firstEdgeDelayPercent=0;
	ftmParam[2].firstEdgeDelayPercent=0;
	ftmParam[3].firstEdgeDelayPercent=0;

	ftmParam[1].enableComplementary=false;
	ftmParam[2].enableComplementary=false;
	ftmParam[3].enableComplementary=false;

	ftmParam[1].enableDeadtime=false;
	ftmParam[2].enableDeadtime=false;
	ftmParam[3].enableDeadtime=false;//	  currentcolor=*((uint8_t *)pvTaskGetThreadLocalStoragePointer(pattern_executor_handler,\
	//			  THREAD_LOCAL_STORAGE_INDEX));
	//	  if(controlmode== UP){
	//		  pwm_up_execution(currentcolor, currentcolor+1,1);	/*check for step val */
	//	  } else if(controlmode == DOWN) {
	//		pwm_down_execution(currentcolor, currentcolor-1,1);
	//	  }
	//	}

	FTM_SetupPwm(BOARD_FTM_BASEADDR, &ftmParam[1], 3, kFTM_EdgeAlignedPwm, local_config->refresh_rate, FTM_SOURCE_CLOCK);
	FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);

}



void get_default_config(led_config_type *config)
{
	assert(config!=NULL);

	/*configuration field*/

	config->start_color[0]=0;
	config->stop_color[0]=255;
	config->step_value=1;
	config->color_change_rate=100;
	config->step_mode=1;
	config->no_of_cycles=1;

	/*control field*/
	config->refresh_rate=100;
	config->color_scheme=EIGHT_BIT_TRUE_COLOR;
	config->control_mode=0;

}

/* Timer callback function */

void check_control_mode(TimerHandle_t timer)
{

	//xQueueReceive(pattern_control_queue, local_config, 0);
	uint8_t ctrl_mode;
	uint8_t currentcolor;
	ctrl_mode=local_config->control_mode &(START |  STOP |\
									PAUSE | RESUME | UP | DOWN);
	switch(ctrl_mode){
		case START:
		 /* restart logic */
		 /* first send configuration then send START */

		 led_execution(local_config);
		 break;

		case STOP:
		 local_config->start_color[0]=0;
		 local_config->stop_color[0]=0;
		 led_execution(local_config);
		 /*  stop logic */
		 break;

		case PAUSE:
		/* vTaskSetThreadLocalStoragePointer(pattern_executor_handler,\
				 THREAD_LOCAL_STORAGE_INDEX+1,(void *)&local_config->stop_color[0]);
		 local_config->stop_color[0]=\
				 *((uint8_t *)pvTaskGetThreadLocalStoragePointer\
						 (pattern_executor_handler, THREAD_LOCAL_STORAGE_INDEX));
		 led_execution(local_config);*/
			vTaskSuspend(pattern_executor_handler);
		 /* pause logic */
		 break;

		case RESUME:
		 /* resume logic */
			/*local_config->stop_color[0]=\
			*((uint8_t *)pvTaskGetThreadLocalStoragePointer\
					(pattern_executor_handler,THREAD_LOCAL_STORAGE_INDEX+1));
			led_execution(local_config);*/
			vTaskResume(pattern_executor_handler);
		 break;

		case UP:
		 /* manual up mode */
		 /*led_execution(local_config);*/
			 currentcolor=*((uint8_t *)pvTaskGetThreadLocalStoragePointer(pattern_executor_handler,\
						  THREAD_LOCAL_STORAGE_INDEX)); pwm_up_execution(currentcolor+1, currentcolor+1,local_config->step_value);
			 pwm_up_execution(currentcolor+1, currentcolor+1,local_config->step_value);
		 break;

		case DOWN:
		  /*led_execution(local_config);*/
		 /* manual down mode */
			currentcolor=*((uint8_t *)pvTaskGetThreadLocalStoragePointer(pattern_executor_handler,\
									  THREAD_LOCAL_STORAGE_INDEX));
			pwm_down_execution(currentcolor-1, currentcolor-1,local_config->step_value);
		 break;

		default:
		 /* no condition */
		 break;
	}
}

/* update color */

void update_color(uint8_t current_color)
{
	uint8_t updatedDutyCycle1,updatedDutyCycle2,updatedDutyCycle3;

	/* channel 2; RED */

    updatedDutyCycle2=DUTY_CYCLE_RED(((current_color & RED_MASK_VAL) >> RED_SHIFT_VAL));
    FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL2, kFTM_EdgeAlignedPwm, updatedDutyCycle2);
    FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

    /* channel 3;GREEN */

    updatedDutyCycle3=DUTY_CYCLE_GREEN(((current_color & GREEN_MASK_VAL) >> GREEN_SHIFT_VAL));
    FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL3, kFTM_EdgeAlignedPwm, updatedDutyCycle3);
    FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

	/* channel 1; BLUE */

	updatedDutyCycle1=DUTY_CYCLE_BLUE(((current_color & BLUE_MASK_VAL) >> BLUE_SHIFT_VAL));
    FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL1, kFTM_EdgeAlignedPwm, updatedDutyCycle1);
    FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

}


