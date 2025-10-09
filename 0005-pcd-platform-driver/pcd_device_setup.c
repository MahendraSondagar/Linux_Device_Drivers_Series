#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"



/* platform device release function */
void pcdev_release(struct device *dev)
{
	pr_info("device setup module is relesed!\r\n");
}

/*1. creating the platfrom data with 2 instances*/

struct pcdev_platform_data pcdev_data[2]=
{
	[0] = {.size= 512,   .perm = RDWR, .serial_number = "PCDEV0011AA"},
	[1] = {.size = 1024, .perm = RDWR, .serial_number = "PCDEV0022BB"}
};

/*2. first platfrom device */
struct platform_device platform_pcdev_1 ={
	.name = "pcd-char-device",
	.id = 0,
	.dev = {
		.platform_data = &pcdev_data[0],
		.release = pcdev_release
	}
};

/* 2nd platform device */
struct platform_device platform_pcdev_2 ={
	.name = "pcd-char-device",
	.id = 1,
	.dev = {
		.platform_data = &pcdev_data[1],
		.release = pcdev_release
	}
};

/* Module load entry point*/
static int __init pcdev_platform_init(void)
{
	/* register platform device */
	platform_device_register(&platform_pcdev_1);
	platform_device_register(&platform_pcdev_2);

	pr_info("device setup module is inserted\r\n");
	return 0;
}

/*module exit function*/
static void __exit pcdev_platform_exit(void)
{
	/*unregister the platfrom device */
	platform_device_unregister(&platform_pcdev_1);
	platform_device_unregister(&platform_pcdev_2);
	pr_info("device setup module is module removed!\r\n");
}

/*Module registartion macro*/
module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);


/*module description*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("Module platform device example");
 
