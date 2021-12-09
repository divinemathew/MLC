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

static xQueueHandle communication_queue;
static xQueueHandle slave_status_queue;



int i2c_slave_init(void);
int i2c_pin_config(void);
void communication_task(void* pvParameter);


typedef struct {
	uint8_t start_color[3];
	uint8_t stop_color[3];
	uint8_t step_value;
	uint8_t step_mode;
	uint8_t no_of_cycles;
	uint16_t color_change_rate;
	uint16_t refresh_rate;
	uint8_t color_scheme;
	uint8_t control_mode;
	uint8_t current_color[3];
} led_config_type;

