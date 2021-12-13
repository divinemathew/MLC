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
TaskHandle_t ui_handler_handle;

static QueueHandle_t communication_queue;
static QueueHandle_t slave_status_queue;

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
//    if(xTaskCreate(communication_task, "Communication Task", configMINIMAL_STACK_SIZE + 200, (void*)false, communication_task_PRIORITY, NULL)!=pdPASS){
//    	PRINTF("\r\nCommunication Task Creation failed");
//    }

    /* UI_handler task creation */
    xTaskCreate(ui_handler_task, "task1", configMINIMAL_STACK_SIZE + 100, NULL, 4, &ui_handler_handle);

    /* Start scheduler */
    vTaskStartScheduler();
	for (; ;) {
	}
    return 0 ;
}

QueueHandle_t get_queue_handle(queue_enum queue_requested)
{
	QueueHandle_t queue;

	switch (queue_requested) {
		case COMMUNICATION_QUEUE :
			queue = communication_queue;
			break;

		case SLAVE_STATUS_QUEUE :
			queue = slave_status_queue;
			break;

		default :
			queue = NULL;
	}
	return queue;
}
