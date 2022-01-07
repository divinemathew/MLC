#ifndef COMM_HANDLER_H_
#define COMM_HANDLER_H_



#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "fsl_i2c_freertos.h"

#define SLAVE_ADDRESS 				0x2D
#define CONTROL_MODE_OFFSET			0x11
#define CONFIG_OFFSET				0x04
#define SLAVE_WRITE_OFFSET			0x02
#define SLAVEMODE_OFFSET			0x00
#define SLAVEMODE_VALUE				{BE,EF}
#define I2C0_BASEADDR 				I2C0
#define I2C0_CLK_FREQ         		CLOCK_GetFreq(I2C0_CLK_SRC)
#define I2C_DATA_LENGTH            	34U
#define RECEIVE_DATA_LENGTH			34U
#define TRANSMIT_DATA_LENGTH		2U
#define I2C0_SLAVE_HOLD_TIME_NS 	4000U
#define WAIT_TIME                   10U
#define I2C0_BAUDRATE               100000U
#define I2C_DATA_LEN				35U



#endif /* COMM_HANDLER_H_ */
