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
#define JUMPER_PIN 0U

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
static _Bool master_mode = true;

static TaskHandle_t ui_handler_handle;
static QueueHandle_t communication_queue;
static QueueHandle_t device_status_queue;
static QueueHandle_t pattern_status_queue;


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

    /* queue creations */
    master_mode = GPIO_PinRead(GPIOD, JUMPER_PIN);
    communication_queue = xQueueCreate(2, sizeof(led_config_type));
    device_status_queue = xQueueCreate(1, sizeof(_Bool));
    pattern_status_queue = xQueueCreate(1, sizeof(uint8_t));

    /* task creations */
    xTaskCreate(communication_task, "Communication Task", configMINIMAL_STACK_SIZE + 200, &master_mode, 4, NULL);
    xTaskCreate(ui_handler_task, "UI Task", configMINIMAL_STACK_SIZE + 100, &master_mode, 4, &ui_handler_handle);

    /* start scheduler */
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

		case DEVICE_STATUS_QUEUE :
			queue = device_status_queue;
			break;

		case PATTERN_STATUS_QUEUE :
			queue = pattern_status_queue;
			break;

		default :
			queue = NULL;
	}
	return queue;
}
