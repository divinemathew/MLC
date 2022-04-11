#include <../../linux/include/linux/module.h>   /* Needed by all modules */                                                                       
#include <../../linux/include/linux/kernel.h>   /* Needed for KERN_INFO */
#include <../../linux/include/linux/fs.h>       /*file system structure*/
#include <linux/ioctl.h>                        /* for ioctl */
#include <../../linux/include/linux/uaccess.h>  /*for copy from user and copy to user*/
#include <linux/i2c.h> 							/*for i2c*/
#include <linux/types.h>
#include <linux/delay.h>


#define I2C_BUS_AVAILABLE             1          // I2C Bus number
#define SLAVE_DEVICE_NAME    "MLC_SLAVE"         // Device and Driver Name
#define SLAVE_ADDR        	  0x2D               // SSD1304 OLED Slave Address


static struct i2c_adapter *mlc_i2c_adapter     = NULL;   // I2C Adapter Structure
static struct i2c_client  *mlc_i2c_client = NULL; 		 // I2C Cient Structure

typedef struct {
	uint8_t start_clr[3];
	uint8_t end_clr[3];
	uint8_t step_value;
	uint8_t mode;
	uint8_t cycle_num;
	uint16_t step_rate;
	uint16_t refresh_rate;
	uint8_t color_code;
} config_t;

/**
 * @brief function to write data to slave
 *
 */
static int I2C_Write(uint8_t* tx_buf, uint8_t offset, uint8_t len)
{	
	int ret, i;
	uint8_t cpy_ar[17];

	cpy_ar[0] = offset;

	for(i = 0; i <= len; i++) {
		
		cpy_ar[i+1] = *(tx_buf+i);
	}

	ret = i2c_master_send(mlc_i2c_client, cpy_ar, len+1);


	if(ret>-1) {
	pr_info("write success%d\n", ret);
	} else {
		pr_info("write failed%d\n",ret);
	}

    return ret;
}

/**
 * @brief function to read data from slave
 *
 */

static int I2C_Read(uint8_t *rx_buf, uint8_t cmd)
{	
	uint8_t ret;
	uint8_t slave_id_offset = 0x00;
	i2c_master_send(mlc_i2c_client, &slave_id_offset, 1);

	ret = i2c_master_recv(mlc_i2c_client, rx_buf, 2);

    return ret;
}

static void I2C_handshake_mlc(void)
{

	uint8_t master_id[] = {0xde, 0xad};
	uint8_t offset = 0x02;
	uint8_t slave_id_offset = 0x00;
	uint8_t slave_id[2];
	uint8_t ret;


	pr_info("entered in probe\n");
	ret = I2C_Write(master_id, offset, 2);
	ret = I2C_Read(slave_id, slave_id_offset);

	if(ret > 0) {
		printk(KERN_INFO "handshake succes!!!\nslave id: %x %x",slave_id[0], slave_id[1]);
	}


}

/**
 * @brief set default configuration
 *
 * @param default_config Address of configuration structure
 *
 * @return None
 *
 *  Revision History:
 *  -201221 ERK : Creation Date
 */

void get_default_config(config_t *default_config)
{
	default_config->start_clr[0] = 1;
	default_config->start_clr[1] = 0;
	default_config->start_clr[2] = 0;
	default_config->end_clr[0] = 0b01101110;
	default_config->end_clr[1] = 0;
	default_config->end_clr[2] = 0;
	default_config->step_value = 0b00100101;
	default_config->mode = 4;
	default_config->cycle_num = 1;
	default_config->step_rate = 10;
	default_config->refresh_rate = 1000;
	default_config->color_code = 0;

}

/*
	Called when driver in added
 */
static int i2c_slave_mlc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	config_t current_config;
	uint8_t config_offset = 0x04;
	
  
	get_default_config(&current_config);
	

	/*hand shake*/
	I2C_handshake_mlc();

	/*sneding configurations*/
	I2C_Write((uint8_t*)&current_config, config_offset, sizeof(config_t));
	return 0;
}

static int i2c_slave_mlc_remove(struct i2c_client *client)
{
	pr_info("mlc driver removed\n");
	return 0;

}

/*
 I2C device id structure
 */

static const struct i2c_device_id mlc_slave_id[] = {
		{ SLAVE_DEVICE_NAME, 0 },
		{ }
};

MODULE_DEVICE_TABLE(i2c, mlc_slave_id);

/*
* I2C driver strucutre
*/
static struct i2c_driver mlc_driver = {
	.driver = {
		.name = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE
	},
	.probe = i2c_slave_mlc_probe,
	.remove = i2c_slave_mlc_remove,
	.id_table = mlc_slave_id,

};

/*
* I2C Board Info strucutre
*/
static struct i2c_board_info mlc_i2c_board_info = {

     I2C_BOARD_INFO(SLAVE_DEVICE_NAME,SLAVE_ADDR)
 
 };

/*function to be called when module inserted*/
static int __init my_driver_init(void)
{ 
	int ret = -1;

	   mlc_i2c_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);

    if( mlc_i2c_adapter != NULL )
    {
        mlc_i2c_client = i2c_new_device(mlc_i2c_adapter, &mlc_i2c_board_info);

        if( mlc_i2c_client != NULL )
        {
            i2c_add_driver(&mlc_driver);
            ret = 0;
        }

        i2c_put_adapter(mlc_i2c_adapter);
    }

    pr_info("Driver Added!!!\n");

    return ret;
}

/*called when driver removed*/
static void __exit my_driver_exit(void)
{
	i2c_unregister_device(mlc_i2c_client);
	i2c_del_driver(&mlc_driver);
    pr_info("Driver Removed!!!\n");

}

module_init(my_driver_init);
module_exit(my_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EMIL RAJU");
MODULE_DESCRIPTION("character driver");
MODULE_VERSION("0.1");


