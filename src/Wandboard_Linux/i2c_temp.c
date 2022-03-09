#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>

#define DRIVER_NAME "mlc"
#define DRIVER_CLASS "mlcClass"

static struct i2c_adapter * bmp_i2c_adapter = NULL;
static struct i2c_client * bmp280_i2c_client = NULL;
static char *device_name = "MLC";
static int major_number = 0;


/* Meta Information */
MODULE_AUTHOR("Divine A. Mathew");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A driver for reading i2c device");
MODULE_SUPPORTED_DEVICE("NONE");

/* Defines for device identification */ 
#define I2C_BUS_AVAILABLE		1		/* The I2C Bus available on the raspberry */
#define SLAVE_DEVICE_NAME		"MLC-SLAVE"	/* Device and Driver Name */
#define BMP280_SLAVE_ADDRESS	0x2D		/* BMP280 I2C address */
#define CONF_XFER  _IOW('a','g',int32_t*)


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
} led_config_type;

led_config_type xfer_config;

static long driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static const struct i2c_device_id bmp_id[] = {
		{ SLAVE_DEVICE_NAME, 0 }, 
		{ }
};

static struct i2c_driver bmp_driver = {
	.driver = {
		.name = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE
	}
};

static struct i2c_board_info bmp_i2c_board_info = {
	I2C_BOARD_INFO(SLAVE_DEVICE_NAME, BMP280_SLAVE_ADDRESS)
};

/* Variables for Device and Deviceclass*/
static dev_t myDeviceNr;
static struct class *myClass;
static struct cdev myDevice;

/* Variables for temperature calculation */
s32 dig_T1, dig_T2, dig_T3;

/**
 * @brief Read current temperature from BMP280 sensor
 *
 * @return temperature in degree
 */
s32 read_temperature(void) {
	int var1, var2;
	s32 raw_temp;
	s32 d1, d2, d3;

	/* Read Temperature */
	d1 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFA);
	d2 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFB);
	d3 = i2c_smbus_read_byte_data(bmp280_i2c_client, 0xFC);
	raw_temp = ((d1<<16) | (d2<<8) | d3) >> 4;

	/* Calculate temperature in degree */
	var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;

	var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	return ((var1 + var2) *5 +128) >> 8;
}

/**
 * @brief Get data out of buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;
	char out_string[20];
	int temperature;

	/* Get amount of bytes to copy */
	to_copy = min(sizeof(out_string), count);

	/* Get temperature */
	temperature = read_temperature();
	snprintf(out_string, sizeof(out_string), "%d.%d\n", temperature/100, temperature%100);

	/* Copy Data to user */
	not_copied = copy_to_user(user_buffer, out_string, to_copy);

	/* Calculate delta */
	delta = to_copy - not_copied;

	return delta;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *deviceFile, struct file *instance) {
	printk("MyDeviceDriver -  Open was called\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is close
 */
static int driver_close(struct inode *deviceFile, struct file *instance) {
	printk("MyDeviceDriver -  Close was called\n");
	return 0;
}

/* Map the file operations */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
};


static struct file_operations fops_mydevice = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
	.unlocked_ioctl = driver_ioctl,
};



static long driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
				case CONF_XFER:
						printk(KERN_INFO "\nCONF_XFER invocked");
						copy_from_user(&xfer_config ,(int32_t*) arg, sizeof(led_config_type));
						printk(KERN_INFO "\nNo of cycles : %d",xfer_config.no_of_cycles);
						printk(KERN_INFO "\nStart Color: %x",xfer_config.start_color[0]);
						printk(KERN_INFO "\nStop Color: %x",xfer_config.stop_color[0]);
						int ret_1 = i2c_smbus_write_block_data(bmp280_i2c_client,0x04,sizeof(xfer_config),(const u8 *)&xfer_config);					
						printk(KERN_INFO "I2C return Val%d",ret_1);
						dig_T1 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x00);
						printk(KERN_INFO "\nRead Value from slave %x",dig_T1);
						int ret = i2c_smbus_write_byte_data(bmp280_i2c_client, 0x02,0xDE);
						printk(KERN_INFO "\nI2C return Value %x",ret);
						int ret_2 = i2c_smbus_write_byte_data(bmp280_i2c_client, 0x03,0xAD);
						printk(KERN_INFO "\nI2C return Value %x",ret_2);
						break;
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}


/**
 * @brief function, which is called after loading module to kernel, do initialization there
 */
static int __init ModuleInit(void) {
	int fd_mydevice;
	int ret = -1;
	u8 id;
	printk("MyDeviceDriver - Hello Kernel\n");

	major_number=register_chrdev(0, device_name, &fops_mydevice);	
	printk(KERN_INFO "\nMajor Number %d",major_number);	
	bmp_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

	if(bmp_i2c_adapter != NULL) {
		bmp280_i2c_client = i2c_new_device(bmp_i2c_adapter, &bmp_i2c_board_info);
		if(bmp280_i2c_client != NULL) {
			if(i2c_add_driver(&bmp_driver) != -1) {
				ret = 0;
			}
			else
				printk("Can't add driver...\n");
		}
		i2c_put_adapter(bmp_i2c_adapter);
	}
	printk("BMP280 Driver added!\n");

	/* Read Chip ID */
	id = i2c_smbus_read_byte_data(bmp280_i2c_client, 0x00);
	printk("ID: 0x%x\n", id);

	/* Read Calibration Values */
	dig_T1 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x88);
	dig_T2 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x8a);
	dig_T3 = i2c_smbus_read_word_data(bmp280_i2c_client, 0x8c);

	if(dig_T2 > 32767)
		dig_T2 -= 65536;

	if(dig_T3 > 32767)
		dig_T3 -= 65536;

	/* Initialice the sensor */
	i2c_smbus_write_byte_data(bmp280_i2c_client, 0xf5, 5<<5);
	i2c_smbus_write_byte_data(bmp280_i2c_client, 0xf4, ((5<<5) | (5<<2) | (3<<0)));
	return ret;

KernelError:
	device_destroy(myClass, myDeviceNr);
FileError:
	class_destroy(myClass);
ClassError:
	unregister_chrdev(myDeviceNr, DRIVER_NAME);
	return (-1);
}

/**
 * @brief function, which is called when removing module from kernel
 * free alocated resources
 */
static void __exit ModuleExit(void) {
	printk("MyDeviceDriver - Goodbye, Kernel!\n");
	i2c_unregister_device(bmp280_i2c_client);
	i2c_del_driver(&bmp_driver);
	cdev_del(&myDevice);
    device_destroy(myClass, myDeviceNr);
    class_destroy(myClass);
    unregister_chrdev_region(myDeviceNr, 1);
}

module_init(ModuleInit);
module_exit(ModuleExit);
