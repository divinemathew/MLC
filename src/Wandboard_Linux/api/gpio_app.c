#include <stdio.h>
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

 
int main()
{
        int fd;
        int32_t value, number;
 
        printf("\nOpening Driver\n");
        fd = open("/dev/mydevice", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
		
		while(1){
		printf("\nSelect the LED to toggle\n1. RED\n2. YELLOW\n3. BLUE\nYour Option :");
		scanf("%d",&number);
		switch(number){
			case 1:
				ioctl(fd,RED_TOOGLE,(int32_t *)&value);
				break;
			case 2:
				ioctl(fd,YEL_TOOGLE,(int32_t *)&number);
				break;
			case 3:	
				ioctl(fd,BLU_TOOGLE,(int32_t *)&number);
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
		}
        printf("Closing Driver\n");
        close(fd);
}
