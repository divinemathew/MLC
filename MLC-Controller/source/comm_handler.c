#include <comm_handler.h>
#include "fsl_i2c.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_debug_console.h"





int i2c_pin_config(void)
{
    /* Port B Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortB);
    /* Port E Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortE);

    const port_pin_config_t i2c_pin = {/* Internal pull-up resistor is enabled */
                                                    kPORT_PullUp,
                                                    /* Fast slew rate is configured */
                                                    kPORT_FastSlewRate,
                                                    /* Passive filter is disabled */
                                                    kPORT_PassiveFilterDisable,
                                                    /* Open drain is enabled */
                                                    kPORT_OpenDrainEnable,
                                                    /* Low drive strength is configured */
                                                    kPORT_LowDriveStrength,
                                                    /* Pin is configured as I2C0_SCL */
                                                    kPORT_MuxAlt5,
                                                    /* Pin Control Register fields [15:0] are not locked */
                                                    kPORT_UnlockRegister};

    PORT_SetPinConfig(PORTE, 24U, &i2c_pin);
    PORT_SetPinConfig(PORTE, 25U, &i2c_pin);

    return true;

}


int i2c_slave_init()
{
    i2c_slave_config_t slave_config;
    status_t status = kStatus_Success;
    i2c_pin_config();

    I2C_SlaveGetDefaultConfig(&slave_config);

    slave_config.slaveAddress=SLAVE_ADDRESS;
    slave_config.sclStopHoldTime_ns = I2C0_SLAVE_HOLD_TIME_NS;

    I2C_SlaveInit(I2C0_BASEADDR, &slave_config, I2C0_CLK_FREQ);
    return true;
}

void i2c_write(uint8_t offset,uint8_t *data,uint8_t size)
{
	i2c_master_transfer_t masterXfer;
	masterXfer.flags=kI2C_TransferDefaultFlag;
	masterXfer.slaveAddress=SLAVE_ADDRESS;

}


int i2c_master_init()
{
    i2c_master_config_t master_config;

    I2C_MasterGetDefaultConfig(&master_config);
    master_config.baudRate_Bps = I2C0_BAUDRATE;
    I2C_MasterInit(I2C0_BASEADDR, &master_config, I2C0_CLK_FREQ);
    return true;
}

/* Slave task*/

void communication_task(void* pvParameter)
{
	led_config_type tx_data;
	tx_data.control_mode = 0;
	led_config_type tx_buff;
	tx_buff.control_mode = 0;

	i2c_pin_config();
	if(pvParameter == true){
		i2c_master_init();
		while(1)
		{
			if(xQueueReceive(communication_queue, &tx_buff, 0)==pdPASS){
				if(tx_buff.control_mode!=0){
					i2c_write(CONTROL_MODE_OFFSET,)
					I2C_MasterTransferBlocking(I2C0_BASEADDR, &masterXfer);
					/*Transfer the control bit to PATTERN QUEUE*/
				}
				else if(tx_buff.control_mode==0){
				}
			}
		}
	}
	else if (pvParameter == false){
		i2c_slave_init();
		while(1){
		PRINTF("False");
		}
	}
}
