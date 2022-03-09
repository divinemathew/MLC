#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/moduleparam.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/i2c.h>

#define RED_LI		24
#define YEL_LI		200
#define BLU_LI		90
#define SWITCH		91
#define I2C_BUS		1




#define I2C_BUS_AVAILABLE   (          1 )              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME   ( "ETX_OLED" )              // Device and Driver Name
#define SSD1306_SLAVE_ADDR  (       0x2D )              // SSD1306 OLED Slave Address

#define HIGH		1
#define LOW			0

#define RED_TOOGLE _IOW('a','a',int32_t*)
#define BLU_TOOGLE _IOW('a','b',int32_t*)
#define YEL_TOOGLE _IOW('a','c',int32_t*)
#define SWITCH_RED _IOR('a','d',int32_t*)
#define HANDSHK_S  _IOR('a','e',int32_t*)
#define HANDSHK_M  _IOW('a','f',int32_t*)
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

static char *device_name = "MLC";
static int major_number = 0;
unsigned long size = 32;
static char* mallocbuff;
int32_t value = 0;
led_config_type xfer_config;
static struct i2c_adapter *etx_i2c_adapter     = NULL;  // I2C Adapter Structure
static struct i2c_client  *etx_i2c_client_oled = NULL; 


static int      __init my_start(void);
static void     __exit my_end(void);
static int      driver_open(struct inode *inode, struct file *file);
static int      driver_release(struct inode *inode, struct file *file);
static long     driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static void 	led_toogle(unsigned int gpio);
static void		i2c_init(void);


static struct i2c_board_info oled_i2c_board_info = {
        I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SSD1306_SLAVE_ADDR)
    };

static const struct i2c_device_id etx_oled_id[] = {
        { SLAVE_DEVICE_NAME, 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, etx_oled_id);

static int I2C_Write(unsigned char *buf, unsigned int len)
{
    int ret = i2c_master_send(etx_i2c_client_oled, buf, len);
    
    return ret;
}
 
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
    int ret = i2c_master_recv(etx_i2c_client_oled, out_buf, len);
    
    return ret;
}
 

static struct i2c_driver etx_oled_driver = {
        .driver = {
            .name   = SLAVE_DEVICE_NAME,
            .owner  = THIS_MODULE,
        },
//        .probe          = etx_oled_probe,
//        .remove         = etx_oled_remove,
        .id_table       = etx_oled_id,
};


static void i2c_init(void){
    int ret = -1;
    etx_i2c_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
    if( etx_i2c_adapter != NULL )
    {
        etx_i2c_client_oled = i2c_new_device(etx_i2c_adapter, &oled_i2c_board_info);
        
        if( etx_i2c_client_oled != NULL )
        {
            i2c_add_driver(&etx_oled_driver);
            ret = 0;
        }
        
        i2c_put_adapter(etx_i2c_adapter);
    }
    etx_i2c_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);
    etx_i2c_client_oled = i2c_new_device(etx_i2c_adapter, &oled_i2c_board_info);
}


static struct file_operations fops =
{
        .owner          = THIS_MODULE,
//        .read           = driver_read,
//	      .write          = driver_write,
        .open           = driver_open,
        .unlocked_ioctl = driver_ioctl,
        .release        = driver_release,
};




static void led_toogle(unsigned int gpio){
	int value;
	value = gpio_get_value(gpio);
	if(value==HIGH){
		gpio_set_value(gpio, LOW);
	} else if(value==LOW){
		gpio_set_value(gpio, HIGH);
	}
}


static int __init my_start(void){

	bool red_gpio_valid;
	bool blu_gpio_valid;
	bool yel_gpio_valid;

	printk(KERN_INFO "Module Inserted\n");
	major_number=register_chrdev(0, device_name, &fops);
	mallocbuff=kzalloc(size,GFP_KERNEL);
	printk(KERN_INFO "\nMajor Number %d",major_number);
	red_gpio_valid	= gpio_is_valid((int)RED_LI);
	blu_gpio_valid	= gpio_is_valid((int)BLU_LI);
	yel_gpio_valid	= gpio_is_valid((int)YEL_LI);
	yel_gpio_valid	= gpio_is_valid((int)SWITCH);


	gpio_direction_output(RED_LI,LOW);
	gpio_direction_output(BLU_LI,LOW);
	gpio_direction_output(YEL_LI,LOW);
	gpio_direction_input(SWITCH);
	i2c_init();	
	return 0;
}


static int driver_open(struct inode *pnode, struct file *pfile){	
//	printk(KERN_INFO "Inside %s Function",__FUNCTION__);
	return 0;
}







static long driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case RED_TOOGLE:
                        led_toogle((unsigned int)RED_LI);
						value = gpio_get_value((unsigned int)SWITCH);
						printk(KERN_INFO "\nInside Red Toogle %d",value);
						break;
                case BLU_TOOGLE:             
                        led_toogle((unsigned int)BLU_LI);
                     break;
				case YEL_TOOGLE:	
                        led_toogle((unsigned int)YEL_LI);
						break;
				case SWITCH_RED:
						value = gpio_get_value((unsigned int)SWITCH);	
                        if( copy_to_user((int32_t*) arg, &value, sizeof(value)) )
                        {
                                pr_err("Data Read : Err!\n");
                        }
						break;
				case CONF_XFER:
						printk(KERN_INFO "\nCONF_XFER invocked");
						copy_from_user(&xfer_config ,(int32_t*) arg, sizeof(led_config_type));
						printk(KERN_INFO "\nNo of cycles : %d",xfer_config.no_of_cycles);
						printk(KERN_INFO "\nStart Color: %x",xfer_config.start_color[0]);
						printk(KERN_INFO "\nStop Color: %x",xfer_config.stop_color[0]);
						break;
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}

int driver_release(struct inode *pnode, struct file *pfile){	
	return 0;
}



static void __exit my_end(void){
	unregister_chrdev(major_number, device_name);		
	printk(KERN_INFO "Module Deregistered ");
	kfree(mallocbuff);
	gpio_free(RED_LI);
	gpio_free(BLU_LI);
	gpio_free(YEL_LI);
	gpio_free(SWITCH);
	i2c_unregister_device(etx_i2c_client_oled);
    i2c_del_driver(&etx_oled_driver);
	printk(KERN_INFO "Module Deregistered");
}



module_init(my_start);
module_exit(my_end);





MODULE_LICENSE("GPL");
MODULE_AUTHOR("Divine A Mathew");
MODULE_DESCRIPTION("GPIO-API_Driver");
MODULE_VERSION("1.5");
