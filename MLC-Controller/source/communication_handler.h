

#define SLAVE_ADDRESS 				0x2D
#define I2C0_BASEADDR 				I2C0
#define I2C0_CLK_FREQ         		CLOCK_GetFreq(I2C0_CLK_SRC)
#define I2C_DATA_LENGTH            	34U
#define I2C0_SLAVE_HOLD_TIME_NS 	4000U
#define WAIT_TIME                   10U
#define I2C0_BAUDRATE               100000U



int i2c_slave_init(void);
int i2c_pin_config(void);
