obj-m +=mlc_driver.o
obj-m +=i2c_temp.o
obj-m +=mlc.o

all:
	make -C /home/divine/Desktop/DCT_EEP/WANDBOARD/kernal/linux M=$(PWD) modules
	


clean:
	make -C /home/divine/Desktop/DCT_EEP/WANDBOARD/kernal/linux M=$(PWD) clean
