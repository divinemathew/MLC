/**
* @comm_handler.c
* @brief 
*
* All Operations using I2C communication are implemented in this file.
*
* @para
* 
* @note
*
* Revision History:
* - 081221  DAM : Creation Date
* - 091221  DAM : Initilization
*/

#include "mlc_common.h"
#include <comm_handler.h>
#include "fsl_i2c.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_debug_console.h"


/*******************************************
* Const and Macro Defines
*******************************************/



/***********************************
* Typedefs and Enum Declarations
***********************************/

/*Slave Data Structure*/
typedef struct {
	uint8_t slave_status[2];
	uint8_t slave_id[2];
	led_config_type rx_data;
} slave_data;
slave_data database;


/***********************************
* External Variable Declarations
***********************************/

// none

/***********************************
* Const Declarations
***********************************/
extern QueueHandle_t communication_queue;
extern QueueHandle_t slave_status_queue;


/***********************************
* Public Variables
***********************************/

uint8_t slave_buff[I2C_DATA_LENGTH];
uint8_t slave_ID[2] ={0xBE,0xEF};
_Bool g_SlaveCompletionFlag;
uint8_t index = 0;
uint8_t recieve_size=20;
uint8_t * buff;
uint8_t transmit_size = 0;


/***********************************
* Private Variables
***********************************/

// none

/***********************************
* Private Prototypes
***********************************/

void i2c_pin_config(void);
void i2c_slave_init(void);
void i2c_write(uint8_t offset,uint8_t *data,uint8_t add_size,uint8_t data_size);
void i2c_master_init(void);

/***********************************
* Public Functions
***********************************/

/**
* @i2c_pin_config
* @brief 
*
* This function initilizes Pin for I2C0
*
* @param
*
*
* @note
*
* Revision History:
* - 081221  DAM : Creation Date
*/
void i2c_pin_config(void)
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

    PORT_SetPinConfig(PORTE, 24U, &i2c_pin);	/* Pin is configured as I2C0_SCL */
    PORT_SetPinConfig(PORTE, 25U, &i2c_pin);	 /* Pin is configured as I2C0_SDL */	
}





/**
* @i2c_slave_init
* @brief 
*
* Configure I2C as slave
*
* @param
*
*
* @note
*
* Revision History:
* - 081221  DAM : Creation Date
*/
void i2c_slave_init()
{
    i2c_slave_config_t slave_config;
    status_t status = kStatus_Success;
    i2c_pin_config();

    I2C_SlaveGetDefaultConfig(&slave_config);

    slave_config.slaveAddress=SLAVE_ADDRESS;
    slave_config.sclStopHoldTime_ns = I2C0_SLAVE_HOLD_TIME_NS;

    I2C_SlaveInit(I2C0_BASEADDR, &slave_config, I2C0_CLK_FREQ);
}


/**
* @i2c_master_init
* @brief 
*
* Configure I2C as master
*
* @param
*
*
* @note
*
* Revision History:
* - 081221  DAM : Creation Date
*/
void i2c_master_init(void)
{
    i2c_master_config_t master_config;

    I2C_MasterGetDefaultConfig(&master_config);
    master_config.baudRate_Bps = I2C0_BAUDRATE;
    I2C_MasterInit(I2C0_BASEADDR, &master_config, I2C0_CLK_FREQ);
}



/**
* @i2c_write
* @brief 
*
* This function is used by I2C master to check the offset address of the slave and to perform various actions like Handshaking Slave Read, Write etc
*
*
* @param offset	-	Offset address of the slave
* @param *data	-	Data pointer 
* @param add_size -	Size of Address
* @param data_size -Size of Data
*
* @note
*
* Revision History:
* - 081221  DAM : Creation Date
*/
void i2c_write(uint8_t offset,uint8_t *data,uint8_t add_size,uint8_t data_size)
{
	i2c_master_transfer_t masterXfer;
	masterXfer.flags=kI2C_TransferDefaultFlag;
	masterXfer.slaveAddress=SLAVE_ADDRESS;
	masterXfer.direction=kI2C_Write;
	masterXfer.dataSize=data_size;
	masterXfer.subaddress=offset;
	masterXfer.subaddressSize= add_size;
	switch (offset) {
		case CONTROL_MODE_OFFSET:
			masterXfer.data = data;
			//PRINTF("%d",masterXfer.data);
			I2C_MasterTransferBlocking(I2C0,&masterXfer);
			break;
		case SLAVEMODE_OFFSET:;
			uint8_t slave_id[2];
			masterXfer.data=slave_id;
			masterXfer.dataSize=2;
			masterXfer.direction=kI2C_Read;
			I2C_MasterTransferBlocking(I2C0, &masterXfer);
			if(slave_id[0]==0xBE && slave_id[1]==0xEF){
				xQueueSend(slave_status_queue,true,0);
				PRINTF("Slave Found");
				masterXfer.data=data;
				masterXfer.dataSize=data_size;
				masterXfer.direction=kI2C_Write;
				masterXfer.subaddress=SLAVE_CONFIG_OFFSET;
				masterXfer.subaddressSize=1;
				I2C_MasterTransferBlocking(I2C0, &masterXfer);
			} else{
				PRINTF("No Slave Found");
				xQueueSend(slave_status_queue,false,0);
			}
			break;
		case SLAVE_CONFIG_OFFSET:
			masterXfer.data=data;
			masterXfer.dataSize=data_size;
			masterXfer.direction=kI2C_Write;
			masterXfer.subaddress=SLAVE_CONFIG_OFFSET;
			masterXfer.subaddressSize=1;
			I2C_MasterTransferBlocking(I2C0, &masterXfer);
			break;
		default:
			break;
	}

	if(offset==CONTROL_MODE_OFFSET){
		masterXfer.data = data;
		//PRINTF("%d",masterXfer.data);
		I2C_MasterTransferBlocking(I2C0,&masterXfer);

	} else if(offset==SLAVEMODE_OFFSET){
		uint8_t slave_id[2];
		masterXfer.data=slave_id;
		masterXfer.dataSize=2;
		masterXfer.direction=kI2C_Read;
		I2C_MasterTransferBlocking(I2C0, &masterXfer);
		if(slave_id[0]==0xBE && slave_id[1]==0xEF){
			xQueueSend(slave_status_queue,true,0);
			PRINTF("Slave Found");
			masterXfer.data=data;
			masterXfer.dataSize=data_size;
			masterXfer.direction=kI2C_Write;
			masterXfer.subaddress=SLAVE_CONFIG_OFFSET;
			masterXfer.subaddressSize=1;
			I2C_MasterTransferBlocking(I2C0, &masterXfer);
		} else{
			//PRINTF("No Slave Found");
			_Bool status = false;
			xQueueSend(slave_status_queue,&status,0);
		}
	}


	//PRINTF("\r\n%d",((led_config_type*)data)->control_mode);
	//PRINTF("\r\n%d",((led_config_type*)data)->start_color[0]);
}




/**
* @i2c_slave_callback
* @brief 
*
* This function is used by I2C SLAVE ISR, this function will be invocked at various events of I2C Transfer
*
*
* @param base	-	Base Address of I2C
* @param xfer	-	Handle for I2C Transfer 
* @param userData -	Userdata
*
* @note
*
* Revision History:
* - 081221  DAM : Creation Date
*/

static void i2c_slave_callback(I2C_Type *base, i2c_slave_transfer_t *xfer, void *userData)
{

	_Bool g_SlaveCompletionFlag;
    switch (xfer->event)
    {
        /*  Address match event */
        case kI2C_SlaveAddressMatchEvent:
            xfer->data     = NULL;
            xfer->dataSize = 0;
            break;
        /*  Transmit request */
        case kI2C_SlaveTransmitEvent:
            /*  Update information for transmit process */
            xfer->data     = buff;
            xfer->dataSize = transmit_size;
            break;

        /*  Receive request */
        case kI2C_SlaveReceiveEvent:
            /*  Update information for received process */

            xfer->data     = g_slave_buff;
            xfer->dataSize = recieve_size;

            switch (g_slave_buff[0]) {
				case 0x00:
		          	buff = (uint8_t *)database.slave_status;
		            transmit_size = 2;
		            recieve_size = 1;
					break;
				case 0x03:
	            	buff = (uint8_t *) database.slave_id;;
	            	recieve_size = 2;
	            	break;
				case 0x04:
					buff = (uint8_t*) &database.rx_data;
					recieve_size = sizeof(led_config_type);
				default:
					break;
			}
            break;
        /*  Transfer done */
        case kI2C_SlaveCompletionEvent:
            g_SlaveCompletionFlag = true;
            xfer->data            = NULL;
            xfer->dataSize        = 0;
            index = 0;
            break;

        default:
            g_SlaveCompletionFlag = false;
            break;
    }
}








/**
* @communication_task
* @brief 
*
* This is RTOS Task used for Communication between MLC master & MLC slave 
*
*
* @param 
*
* @note
*
* Revision History:
* - 081221  DAM : Creation Date
*/
void communication_task(void* pvParameter)
{
	led_config_type tx_buff;
	tx_buff.control_mode = 0;
	i2c_pin_config();
	
	/*Master Mode*/
	if(pvParameter == true){
		i2c_master_init(); /*I2C Master Initilization*/
		
		/*Communication Task Loop*/
		while(1)
		{
			/*Checking For any item available in Queue*/
			if(xQueueReceive(communication_queue, &tx_buff, 0)==pdPASS){
				PRINTF("%d",tx_buff.start_color[0]);
				
				if(tx_buff.control_mode!=0){
					PRINTF("\r\nEntered In conTrol Mode");
					/*Transfer the data to MLC Slave via I2C Protocol*/
					i2c_write(CONTROL_MODE_OFFSET, &tx_buff.control_mode, 1, 1);
					/*Transfer the control bit (CONFIG) to PATTERN QUEUE*/
				}
				else if(tx_buff.control_mode==0){
					/*Transfer the CONFIG to PATTERN QUEUE*/
					PRINTF("%d",tx_buff.start_color[0]);
					/*Transfer the control bit to MLC Slave via I2C Protocol*/
					i2c_write(SLAVEMODE_OFFSET, &tx_buff, 1, sizeof(led_config_type));
				}
			}
			else {
				__NOP();
			}
		}
	}
	/*MLC Slave Mode*/
	else if (pvParameter == false){
		i2c_slave_handle_t slave_handle;
	    memset(&slave_handle, 0, sizeof(slave_handle));
		I2C_SlaveTransferCreateHandle(I2C0_BASE, &slave_handle, i2c_slave_callback, NULL);
		i2c_slave_init(); /*Initilization of I2C Slave*/

		I2C_SlaveTransferNonBlocking(I2C0_BASE, &slave_handle, kI2C_SlaveCompletionEvent | kI2C_SlaveTransmitEvent | kI2C_SlaveReceiveEvent);
		while(true){
			while(g_SlaveCompletionFlag){
				tx_data = database.rx_data;
				xQueueSend(communication_queue,&tx_data,0);
				g_SlaveCompletionFlag=false;
			}
	}
	}
}

