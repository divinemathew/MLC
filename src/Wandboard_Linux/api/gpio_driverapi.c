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

#define RED_LI		24
#define YEL_LI		200
#define BLU_LI		90

#define HIGH		1
#define LOW			0

#define RED_TOOGLE _IOW('a','a',int32_t*)
#define BLU_TOOGLE _IOW('a','b',int32_t*)
#define YEL_TOOGLE _IOW('a','c',int32_t*)

static char *device_name = "Character Device";
static int major_number = 0;
unsigned long size = 32;
static char* mallocbuff;
int32_t value = 0;




static int      __init my_start(void);
static void     __exit my_end(void);
static int      driver_open(struct inode *inode, struct file *file);
static int      driver_release(struct inode *inode, struct file *file);
static long     driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static void 	led_toogle(unsigned int gpio);


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
	red_gpio_valid	= gpio_is_valid((int)RED_LI);
	blu_gpio_valid	= gpio_is_valid((int)BLU_LI);
	yel_gpio_valid	= gpio_is_valid((int)YEL_LI);

	gpio_direction_output(RED_LI,LOW);
	gpio_direction_output(BLU_LI,LOW);
	gpio_direction_output(YEL_LI,LOW);
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
                        break;
                case BLU_TOOGLE:             
                        led_toogle((unsigned int)BLU_LI);
                     break;
				case YEL_TOOGLE:	
                        led_toogle((unsigned int)YEL_LI);
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
	printk(KERN_INFO "Module Deregistered");
}



module_init(my_start);
module_exit(my_end);





MODULE_LICENSE("GPL");
MODULE_AUTHOR("Divine A Mathew");
MODULE_DESCRIPTION("GPIO-API_Driver");
MODULE_VERSION("1.5");
