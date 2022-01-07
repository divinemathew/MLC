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
* - 091221  DAM : Initialization
*/

#include "mlc_common.h"
#include "comm_handler.h"
#include "fsl_i2c.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_debug_console.h"

/*******************************************
* Const and Macro Defines
*******************************************/
// none

/***********************************
* Typedefs and Enum Declarations
***********************************/
/*Slave Data Structure*/
 typedef struct {
	uint8_t slave_status[2];
	uint8_t slave_id[2];
	led_config_type rx_data;
} slave_data;

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
uint8_t rx_buff[I2C_DATA_LENGTH];
uint8_t slave_ID[2] ={0xBE,0xEF};

/***********************************
* Private Variables
***********************************/
static QueueHandle_t communication_queue;
static QueueHandle_t device_status_queue;
static QueueHandle_t pattern_control_queue;

static _Bool g_SlaveCompletionFlag;
static _Bool ismaster 				= false;

/***********************************
* Private Prototypes
***********************************/
void i2c_pin_config(void);
void i2c_slave_init(void);
status_t I2C_write(uint32_t offset,uint8_t add_size,uint8_t* data,uint8_t data_size);
status_t I2C_read(uint32_t offset,uint8_t add_size,uint8_t* data,uint8_t data_size);
void i2c_master_init(void);

/***********************************
* Public Functions
***********************************/

/**
* @i2c_pin_config
* @brief 
*
* This function initializes Pin for I2C0
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
    i2c_pin_config();
    I2C_SlaveGetDefaultConfig(&slave_config);
    slave_config.addressingMode     = kI2C_Address7bit;
    slave_config.upperAddress       = 0; /*  not used for this example */
    slave_config.slaveAddress=SLAVE_ADDRESS;
    slave_config.sclStopHoldTime_ns = I2C0_SLAVE_HOLD_TIME_NS;
    I2C_SlaveInit(I2C0, &slave_config, I2C0_CLK_FREQ);
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
    i2c_pin_config();
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
status_t I2C_write(uint32_t offset,uint8_t add_size,uint8_t* data,uint8_t data_size)
{
	i2c_master_transfer_t masterXfer;
	status_t i2c_writestatus;
	masterXfer.flags=kI2C_TransferDefaultFlag;
	masterXfer.slaveAddress=SLAVE_ADDRESS;
	masterXfer.direction=kI2C_Write;
	masterXfer.data=data;
	masterXfer.dataSize=data_size;
	masterXfer.subaddress=offset;
	masterXfer.subaddressSize= add_size;
	i2c_writestatus = I2C_MasterTransferBlocking(I2C0, &masterXfer);
	return i2c_writestatus;
}

status_t I2C_read(uint32_t offset,uint8_t add_size,uint8_t* data,uint8_t data_size)
{
	i2c_master_transfer_t masterXfer;
	status_t i2c_readstatus;
	masterXfer.flags=kI2C_TransferDefaultFlag;
	masterXfer.slaveAddress=SLAVE_ADDRESS;
	masterXfer.direction=kI2C_Read;
	masterXfer.data=data;
	masterXfer.dataSize=data_size;
	masterXfer.subaddress=offset;
	masterXfer.subaddressSize= add_size;
	i2c_readstatus = I2C_MasterTransferBlocking(I2C0, &masterXfer);
	return i2c_readstatus;
}


_Bool I2C_Handshake(void)
{
	uint8_t rx_data[2];
	uint8_t tx_data[2] = {0xDE,0xAD};

	I2C_read(SLAVEMODE_OFFSET,1,rx_data,2);

	if(I2C_read(SLAVEMODE_OFFSET,1,rx_data,2)==kStatus_Success){
		if(rx_data[0]==0xBE && rx_data[1]==0xEF){

			if(I2C_write(SLAVE_WRITE_OFFSET, 1, tx_data, 2)==kStatus_Success){
				return true;
			}
		} else{
			return false;
		}
	} else{
		return false;
	}

	return false;
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
            xfer->data     = slave_ID;
            xfer->dataSize = TRANSMIT_DATA_LENGTH;
            break;
        /*  Receive request */
        case kI2C_SlaveReceiveEvent:
            /*  Update information for received process */
            xfer->data     = rx_buff;
            xfer->dataSize = RECEIVE_DATA_LENGTH;
            break;
        /*  Transfer done */
        case kI2C_SlaveCompletionEvent:
            g_SlaveCompletionFlag = true;
            xfer->data            = NULL;
            xfer->dataSize        = 0;

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
	ismaster = *((_Bool*) pvParameter);
	status_t xfer_status;
	led_config_type config;
	config.control_mode=0;
	led_config_type temp_config;
	temp_config.control_mode=0;
	_Bool device_status;
	xfer_data xferdata;
	communication_queue = get_queue_handle(COMMUNICATION_QUEUE);
	device_status_queue = get_queue_handle(DEVICE_STATUS_QUEUE);
	pattern_control_queue = get_queue_handle(PATTERN_CONTROL_QUEUE);
	if (ismaster) {
			/*MASTER MODE*/
			i2c_master_init();
			while(true){
				if(xQueueReceive(communication_queue, &config, 0)==pdPASS){
					if(config.control_mode==START){
						if(I2C_Handshake()){
							device_status=true;
							xQueueSend(device_status_queue,&device_status,100);
						}else {
							device_status=false;
							xQueueSend(device_status_queue,&device_status,100);
						}
						xfer_status = I2C_write(CONTROL_MODE_OFFSET, 1,(uint8_t*) &config.control_mode, sizeof(uint8_t));
						xQueueSend(pattern_control_queue,&config,100);
					}else if(config.control_mode==UP || config.control_mode==DOWN){
						/*Control Byte Only*/
						xfer_status = I2C_write(CONTROL_MODE_OFFSET, 1,(uint8_t*) &config.control_mode, sizeof(uint8_t));
						xQueueSend(pattern_control_queue,&config,100);
					}else if(config.control_mode!=temp_config.control_mode){
						/*Control Byte Only*/
						if(I2C_Handshake()){
							device_status=true;
							xQueueSend(device_status_queue,&device_status,100);
						}else {
							device_status=false;
							xQueueSend(device_status_queue,&device_status,100);
						}
						xfer_status = I2C_write(CONTROL_MODE_OFFSET, 1,(uint8_t*) &config.control_mode, sizeof(uint8_t));
						xQueueSend(pattern_control_queue,&config,100);
					}else{
						/*HANDSHAKE + Send Full Config*/
						if(I2C_Handshake()){
							device_status = true;
							xQueueSend(device_status_queue,&device_status,100);
							xferdata.start_color[0]		=	config.start_color[0];
							xferdata.stop_color[0]		=	config.stop_color[0];
							xferdata.step_value			= 	config.step_value;
							xferdata.step_mode			=	config.step_mode;
							xferdata.no_of_cycles 		=	config.no_of_cycles;
							xferdata.color_change_rate	=	config.color_change_rate;
							xferdata.refresh_rate 		=	config.refresh_rate;
							xferdata.color_scheme		=	config.color_scheme;
							config.control_mode=NOP;
							xfer_status = I2C_write(CONFIG_OFFSET, 1, (uint8_t*)&xferdata, sizeof(xferdata));
							if (xfer_status !=kStatus_Success) {
							}
							xQueueSend(pattern_control_queue,&config,100);
						}else{
							device_status = false;
							xQueueSend(device_status_queue,&device_status,0);
							xQueueSend(pattern_control_queue,&config,100);
						}
					}
				} else {
					taskYIELD();
				}
			}
	}else{
			/*SLAVE MODE*/
				i2c_slave_init();
				i2c_slave_handle_t slave_handle;
				memset(&slave_handle, 0, sizeof(slave_handle));
				I2C_SlaveTransferCreateHandle(I2C0, &slave_handle, i2c_slave_callback, NULL);
				I2C_SlaveTransferNonBlocking(I2C0, &slave_handle, kI2C_SlaveCompletionEvent |
							kI2C_SlaveAddressMatchEvent | kI2C_SlaveTransmitEvent | kI2C_SlaveReceiveEvent);
				while(1){
					if(g_SlaveCompletionFlag==true){
						switch (rx_buff[0]) {
							case 0x00:
								device_status = true;
								xQueueSend(device_status_queue,&device_status,0);
								g_SlaveCompletionFlag=false;
								break;
							case 0x02:
								if(rx_buff[1]==0xDE && rx_buff[2]==0xAD){
									device_status = true;
									xQueueSend(device_status_queue,&device_status,0);
								}
								g_SlaveCompletionFlag=false;
								break;
								case 0x04:
									xferdata = *(xfer_data *)&rx_buff[1];
									if(xferdata.step_mode==AUTO_UP){
										xferdata.step_mode=AUTO_DOWN;
									} else if(xferdata.step_mode ==AUTO_DOWN){
										xferdata.step_mode = AUTO_UP;
									}
									config.start_color[0]		=	xferdata.start_color[0];
									config.stop_color[0]		=	xferdata.stop_color[0];
									config.step_value			= 	xferdata.step_value;
									config.step_mode			=	xferdata.step_mode;
									config.no_of_cycles 		=	xferdata.no_of_cycles;
									config.color_change_rate	=	xferdata.color_change_rate;
									config.refresh_rate 		=	xferdata.refresh_rate;
									config.color_scheme			=	xferdata.color_scheme;
									config.control_mode=NOP;
									xQueueSend(pattern_control_queue,&config,0);
									xQueueSend(communication_queue,&config,0);
									g_SlaveCompletionFlag=false;
									break;
								case 0x11:
									config.control_mode = rx_buff[1];
									if(config.step_mode==MANUAL){
										if(config.control_mode==UP){
											config.control_mode=DOWN;
										} else if(config.control_mode==DOWN){
											config.control_mode=UP;
										}
									}
									xQueueSend(pattern_control_queue,&config,0);
									xQueueSend(communication_queue,&config,0);
									g_SlaveCompletionFlag=false;
									break;
								default:
									g_SlaveCompletionFlag=false;
									break;
							}
						}else{
							taskYIELD();
						}

					}
	}
}


