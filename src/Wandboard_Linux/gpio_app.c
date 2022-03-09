#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
 
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

led_config_type default_config;


_Bool read_switch(int fd){
	_Bool value;
	ioctl(fd,SWITCH_RED,(int32_t *)&value);
	return value;
}

 
int main()
{		FILE* dtb;
		
        int fd,switch_fd;
		_Bool switch_val;
        int32_t value, number;
		default_config.start_color[0]	= 	0xAA;
		default_config.stop_color[0]	=	0xFF;
		default_config.step_value		=	0x10;
		default_config.step_mode		=	0x00;
		default_config.no_of_cycles		=	10;
		default_config.color_change_rate=	0;
		default_config.refresh_rate		=	0;
		default_config.color_scheme		=	0;
		default_config.control_mode		=	0;

		dtb = fopen("database.txt","rw");
		if(dtb == NULL){
			printf("\nError Opening File");
		}
		fwrite(&default_config,sizeof(led_config_type),1,dtb);

   		if(fwrite != 0)
      		printf("Contents to file written successfully !\n");
   		else
      		printf("Error writing file !\n");
 	  	fclose (dtb);
        printf("\nOpening Driver\n");
        fd = open("/dev/mydevice", O_RDWR);
		
		ioctl(fd,CONF_XFER,(int32_t *)&default_config);
/*  		while(1){
		switch(number){
			case 1:
				ioctl(fd,RED_TOOGLE,(int32_t *)&value);
				break;
			case 2:
				ioctl(fd,YEL_TOOGLE,(int32_t *)&number);
				break;			case 3:	
				ioctl(fd,BLU_TOOGLE,(int32_t *)&number);
				break;
			case 4:
				switch_val = read_switch(fd);
				printf("\nSwitch Read Value = %d",switch_val);
				break;
			default:
				printf("\nWrong Entry");
				break;
		} 
		printf("Do you want to continue \n1. Yes\n2. No\nEnter Your Option: ");
		scanf("%d",&number);
 		if(number!=1){
			break;
		}
		}*/
        printf("Closing Driver\n");
        close(fd);
}
