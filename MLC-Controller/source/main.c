/**
 * @file    MLC-Controller.c
 * @brief   Application entry point.
 */

#include "mlc_common.h"

/*******************************************
 * Const and Macro Defines
 *******************************************/
#define communication_task_PRIORITY  (configMAX_PRIORITIES - 2)
#define master_task_PRIORITY (configMAX_PRIORITIES - 2)

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

// none

/***********************************
 * Private Variables
 ***********************************/
TimerHandle_t status_timer;

/***********************************
 * Private Prototypes
 ***********************************/
void communication_task(void *pvParameters);

/***********************************
 * Public Functions
 ***********************************/
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();


    communication_queue = xQueueCreate(1, sizeof (led_config_type));
    if(xTaskCreate(communication_task, "Communication Task", configMINIMAL_STACK_SIZE + 200, (void*)true, communication_task_PRIORITY, NULL)!=pdPASS){
    	PRINTF("\r\nCommunication Task Creation failed");
    }

    /* UI_handler task creation */
    xTaskCreate(ui_handler_task, "task1", configMINIMAL_STACK_SIZE + 100, NULL, 4, &ui_handler_handle);
    status_timer = xTimerCreate("AutoReload", 100000, pdTRUE, 0, print_status);
    xTimerStart(status_timer, 0);

    vTaskStartScheduler();
	for (; ;) {
	}


    return 0 ;
}




