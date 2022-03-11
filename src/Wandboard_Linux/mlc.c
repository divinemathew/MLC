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
#include <linux/jiffies.h>
#include <linux/gpio.h>     //GPIO
#include <linux/interrupt.h>



#define DRIVER_NAME "mlc"
#define DRIVER_CLASS "mlcClass"
#define LED1_GPIO     90	/*GPIO3_26*/
#define LED2_GPIO     200	/*GPIO7_8*/
#define LED3_GPIO     24	/*GPIO1_24*/
#define PUSH_BUTTON   91	/*GPIO3_27*/
#define START		  0x01
#define STOP		  0x02



static struct i2c_adapter * 	mlc_i2c_adapter = NULL;
static struct i2c_client * 		mlc_i2c_client = NULL;
static char *device_name 	= 	"MLC";
static int major_number 	= 	0;


/*irqnumber for Push button*/
unsigned int GPIO_irqNumber;
//extern unsigned long jiffies;
unsigned long jiffie_before = 0;




MODULE_AUTHOR("Divine A. Mathew");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A driver for reading i2c device");
MODULE_SUPPORTED_DEVICE("NONE");


#define I2C_BUS_AVAILABLE		1
#define SLAVE_DEVICE_NAME		"MLC-SLAVE"	
#define MLC_SLAVE_ADDRESS		0x2D	
#define CONF_XFER  _IOW('a','e',int32_t*)
#define HANDSHAKE  _IOW('a','f',int32_t*)
#define COMM_XFER  _IOW('a','g',int32_t*)

/* Variables for Device and Deviceclass*/
static dev_t myDeviceNr;
static struct class *myClass;
static struct cdev myDevice;


typedef struct {
	uint8_t offset;
	uint8_t start_color[3];
	uint8_t stop_color[3];
	uint8_t step_value;
	uint8_t step_mode;
	uint8_t no_of_cycles;
	uint16_t color_change_rate;
	uint16_t refresh_rate;
	uint8_t color_scheme;
} led_config_type;

led_config_type xfer_config;
led_config_type check;
char master_ID[3];
char command[2];



void switch_init(void);


static long driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static irqreturn_t push_button_irq_handler(int irq, void *dev_id);


static const struct i2c_device_id bmp_id[] = {
		{ SLAVE_DEVICE_NAME, 0 }, 
		{ }
};


static struct i2c_driver mlc_driver = {
	.driver = {
		.name = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE
	}
};



static struct i2c_board_info mlc_i2c_board_info = {
	I2C_BOARD_INFO(SLAVE_DEVICE_NAME, MLC_SLAVE_ADDRESS)
};




static int driver_open(struct inode *deviceFile, struct file *instance) {
	printk("MyDeviceDriver -  Open was called\n");
	return 0;
}



static int driver_close(struct inode *deviceFile, struct file *instance) {
	printk("MyDeviceDriver -  Close was called\n");
	return 0;
}



void switch_init(){

/*checking gpio numbers valid or not*/
	if(gpio_is_valid(LED1_GPIO) == false){
		printk(KERN_INFO "invalid gpio\n");
	}

	if(gpio_is_valid(LED2_GPIO) == false){
		printk(KERN_INFO "invalid gpio\n");
	}

	if(gpio_is_valid(LED3_GPIO) == false){
		printk(KERN_INFO "invalid gpio\n");
	}

	if(gpio_is_valid(PUSH_BUTTON) == false){
		printk(KERN_INFO "invalid gpio\n");
	}

	/*requesting required gpios*/
	gpio_request(LED1_GPIO, "led1_gpio");
	gpio_request(LED2_GPIO, "led2_gpio");
	gpio_request(LED3_GPIO, "led3_gpio");
	gpio_request(PUSH_BUTTON, "push_button_gpio");

	/*setting pin gpio directions*/
	gpio_direction_output(LED1_GPIO, 0);
	gpio_direction_output(LED2_GPIO, 0);
	gpio_direction_output(LED3_GPIO, 0);
	gpio_direction_input(PUSH_BUTTON);
	GPIO_irqNumber = gpio_to_irq(PUSH_BUTTON);
	if(request_irq(GPIO_irqNumber, (void*)push_button_irq_handler, IRQF_TRIGGER_RISING, "my_device", NULL)) {

		printk(KERN_INFO "irq request failed\n");
	}

}


static irqreturn_t push_button_irq_handler(int irq, void *dev_id)
{	
	unsigned long diff;
	
	/*checking debounce*/
	diff = jiffies - jiffie_before;
	if(diff < msecs_to_jiffies(200)) {
		return IRQ_HANDLED;
	}
	jiffie_before = jiffies;

	gpio_set_value(LED1_GPIO, 1^gpio_get_value(LED1_GPIO));	
	gpio_set_value(LED2_GPIO, 1^gpio_get_value(LED2_GPIO));			
	gpio_set_value(LED3_GPIO, 1^gpio_get_value(LED3_GPIO));

	printk(KERN_INFO "entered in interrupt handler\n");
	switch(command[1]){
		case 0x00:
			command[1]=START;
			break;
		case START:
			command[1]=STOP;
			break;
		case STOP:
			command[1]=START;
			break;
	}
	i2c_master_send(mlc_i2c_client,(u8 *)&command,3);
	return IRQ_HANDLED;
}

static struct file_operations fops_mydevice = {
	.owner = THIS_MODULE,
	.open = driver_open,
	//.release = driver_close,
	//.read = driver_read,
	.unlocked_ioctl = driver_ioctl,
};






static long driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
		int i;
		u8 * value;
		u8 tx_buff[sizeof(led_config_type)];
		//memcpy( tx_buff, &xfer_config, sizeof(led_config_type));
		tx_buff[0]= xfer_config.offset;
		tx_buff[1]=xfer_config.start_color[0];
		tx_buff[2]=0x00;
		tx_buff[3]=0x00;
		tx_buff[4]=xfer_config.stop_color[0];
		tx_buff[5]=0x00;
		tx_buff[6]=0x00;
		tx_buff[7]=xfer_config.step_value;
		tx_buff[8]=xfer_config.step_mode;
		tx_buff[9]=xfer_config.no_of_cycles;
		tx_buff[10]=xfer_config.color_change_rate;
		tx_buff[11]=(xfer_config.color_change_rate)<<8;
		tx_buff[12]=xfer_config.refresh_rate;
		tx_buff[13]=(xfer_config.color_change_rate)<<8;
		tx_buff[14]=xfer_config.color_scheme;
		value = (u8*)&xfer_config;
		check = *(led_config_type *)value;
		printk(KERN_INFO "\nCheck Start Color : %x",check.start_color[0]);
		for(i=0;i<=(sizeof(led_config_type));i++){
		printk(KERN_INFO "\nCheck Value[%d] : %x",i,tx_buff[i]);}
         switch(cmd) {
				case HANDSHAKE:
						printk(KERN_INFO "\nInside HANDSHAKE");
						i2c_master_send(mlc_i2c_client,(const u8*)&master_ID,3);	
						break;
				case CONF_XFER:
						//copy_from_user(&value ,(int32_t*) arg, sizeof(value));
						printk(KERN_INFO "\nStart Color %x",xfer_config.start_color[0]);	
						printk(KERN_INFO "\nInside CONF_XFER");
						xfer_config.offset = 0x04;
						printk(KERN_INFO "\nOffset ID %x",xfer_config.offset);
						i2c_master_send(mlc_i2c_client,(u8 *)&tx_buff,15);	
						break;
				case COMM_XFER:
						printk(KERN_INFO "\nInside COMM_XFER");
						command[0]=0x11;
						command[1]=0x01;
						i2c_master_send(mlc_i2c_client,(const u8*)&command,3);
						break;						
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}


static int __init ModuleInit(void) {

	int ret = -1;
	u8 id;
	printk("MyDeviceDriver - Hello Kernel\n");

	master_ID[0]=0x02;
	master_ID[1]=0xDE;
	master_ID[2]=0xAD;
	command[1]=0x00;
	xfer_config.offset=0x04;
	xfer_config.start_color[0]=0xFF;
	xfer_config.no_of_cycles = 4;
	xfer_config.stop_color[0]=0x00;
	xfer_config.step_mode = 0x02;
	xfer_config.refresh_rate = 0xFFFF;
	major_number=register_chrdev(0, device_name, &fops_mydevice);	
	printk(KERN_INFO "\nMajor Number %d",major_number);	
	mlc_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
	switch_init();
	if(mlc_i2c_adapter != NULL) {
		mlc_i2c_client = i2c_new_device(mlc_i2c_adapter, &mlc_i2c_board_info);
		if(mlc_i2c_client != NULL) {
			if(i2c_add_driver(&mlc_driver) != -1) {
				ret = 0;
			}
			else
				printk("Can't add driver...\n");
		}
		i2c_put_adapter(mlc_i2c_adapter);
	}
	printk("BMP280 Driver added!\n");

	/* Read Chip ID */
	id = i2c_smbus_read_byte_data(mlc_i2c_client, 0x00);
	printk("ID: 0x%x\n", id);
	return ret;
}


static void __exit ModuleExit(void) {
	printk("MyDeviceDriver - Goodbye, Kernel!\n");
	free_irq(GPIO_irqNumber, NULL);
	gpio_free(PUSH_BUTTON);
	gpio_free(LED1_GPIO);
	gpio_free(LED2_GPIO);
	gpio_free(LED3_GPIO);
	i2c_unregister_device(mlc_i2c_client);
	i2c_del_driver(&mlc_driver);
	unregister_chrdev(major_number, device_name);
}

module_init(ModuleInit);
module_exit(ModuleExit);
