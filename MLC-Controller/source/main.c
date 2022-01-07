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

TaskHandle_t ui_handler_handle;
TaskHandle_t pattern_executor_handle;
static QueueHandle_t communication_queue;
static QueueHandle_t device_status_queue;
static QueueHandle_t pattern_status_queue;
static QueueHandle_t pattern_control_queue;

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
    BOARD_InitLEDsPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();

    /* Read if master mode */
    master_mode = GPIO_PinRead(GPIOD, JUMPER_PIN);

    /* queue creations */
    communication_queue = xQueueCreate(2, sizeof(led_config_type));
    device_status_queue = xQueueCreate(1, sizeof(_Bool));
    pattern_status_queue = xQueueCreate(1, sizeof(int16_t));
    pattern_control_queue = xQueueCreate(2, sizeof(led_config_type));

    /* task creations */
    xTaskCreate(ui_handler_task, "UI Task", configMINIMAL_STACK_SIZE + 300, &master_mode, 4, &ui_handler_handle);
    xTaskCreate(communication_task, "Communication Task", configMINIMAL_STACK_SIZE + 300, &master_mode,  4, NULL);
    xTaskCreate(pattern_executor_task, "Pattern Execution Task", configMINIMAL_STACK_SIZE + 300, &master_mode, 4, &pattern_executor_handle);

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

		case PATTERN_CONTROL_QUEUE :
			queue = pattern_control_queue;
			break;

		default :
			queue = NULL;
	}
	return queue;
}
