#ifndef COMM_HANDLER_H_
#define COMM_HANDLER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "fsl_i2c_freertos.h"

#define SLAVE_ADDRESS 				0x2D
#define CONTROL_MODE_OFFSET			0x11
#define SLAVE_CONFIG_OFFSET			0x04
#define SLAVEMODE_OFFSET			0x00
#define SLAVEMODE_VALUE				{BE,EF}
#define I2C0_BASEADDR 				I2C0
#define I2C0_CLK_FREQ         		CLOCK_GetFreq(I2C0_CLK_SRC)
#define I2C_DATA_LENGTH            	34U
#define I2C0_SLAVE_HOLD_TIME_NS 	4000U
#define WAIT_TIME                   10U
#define I2C0_BAUDRATE               100000U
#define I2C_DATA_LEN				35U

extern xQueueHandle communication_queue;
extern xQueueHandle slave_status_queue;

int i2c_slave_init(void);
int i2c_pin_config(void);
void communication_task(void* pvParameter);



#endif /* COMM_HANDLER_H_ */
