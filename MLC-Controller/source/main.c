/**
 * @file    MLC-Controller.c
 * @brief   Application entry point.
 */

#include "mlc_common.h"
#include "comm_handler.h"

/*******************************************
 * Const and Macro Defines
 *******************************************/
#define communication_task_PRIORITY  (configMAX_PRIORITIES - 2)
#define master_task_PRIORITY (configMAX_PRIORITIES - 2)
#define STATUS_UPDATE_RATE 100

/***********************************
 * Typedefs and Enum Declarations
 ***********************************/

// none

/***********************************
 * External Variable Declarations
 ***********************************/
TimerHandle_t status_timer;
TaskHandle_t ui_handler_handle;


xQueueHandle communication_queue;
xQueueHandle slave_status_queue;

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

// none

/***********************************
 * Private Prototypes
 ***********************************/

// none

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
    if(xTaskCreate(communication_task, "Communication Task", configMINIMAL_STACK_SIZE + 200, (void*)false, communication_task_PRIORITY, NULL)!=pdPASS){
    	PRINTF("\r\nCommunication Task Creation failed");
    }

    /* UI_handler task creation */
    xTaskCreate(ui_handler_task, "task1", configMINIMAL_STACK_SIZE + 100, NULL, 4, &ui_handler_handle);
    status_timer = xTimerCreate("AutoReload", STATUS_UPDATE_RATE, pdTRUE, 0, update_status);
    xTimerStart(status_timer, 0);


    vTaskStartScheduler();
	for (; ;) {
	}


    return 0 ;
}

