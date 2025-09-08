obj-m += arg-pass.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERN_DIR =/home/mahi-ms/Documents/MyDrives/MyLearnings/Udemy-LDD/source/BBB-5.10.168-linux-ti-r83/
HOST_KERN_DIR= /lib/modules/$(shell uname -r)/build

all:
	
	make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean: 
	make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

help:
	make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) help

host:
	make -C $(HOST_KERN_DIR) M=$(PWD)  modules
