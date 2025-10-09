#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/platform_device.h>
#include "platform.h"

#define MAX_DEVICES  2U


/* Device privet data structure */
struct pcdev_private_data 
{
	struct pcdev_platform_data pdata;
	char *buffer;
	dev_t dev_num;
	struct cdev cdev;
};

/* driver private data structure */
struct pcdrv_private_data
{
	int total_devices;
	dev_t device_num_base;
	struct class *pcd_class;
	struct device *pcd_device;
};

/*global instance for the driver private data */
struct pcdrv_private_data pcdrv_data;


ssize_t pcd_read (struct file * flip, char __user * buff, size_t count, loff_t * f_pos)
{
	return 0;
}

/* write operations from user space to kernel space */
ssize_t pcd_write (struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
{
	pr_info("write file operations\r\n");
	return -ENOMEM;
}


/* open the device driver file */
int pcd_open (struct inode * inode, struct file * flip)
{
	pr_info("open file operation called\r\n");
	return 0;
} 

/* close the device file  */
int pcd_release (struct inode * inode, struct file * flip)
{
	pr_info("release operation called\r\n");
	return 0;
}

/* lseek the current file position pointer */
loff_t pcd_llseek (struct file * filp, loff_t offset, int whence)
{
	pr_info("lseek operation called\r\n");
	return 0;

}

/* uint32_t variable to hold the major(12 bit) + minor(20 bit) number */
dev_t device_number;

/* cdev structure variable */
struct file_operations pcd_fops =
{
	.open    = pcd_open,
	.write   = pcd_write,
	.read    = pcd_read,
	.llseek  = pcd_llseek,
	.release = pcd_release,
	.owner   = THIS_MODULE
};

/* get called when the match platfrom device found */
int pcd_platform_driver_prob(struct platform_device *pdev)
{
	pr_info("device is detected :)\r\n");
	return 0;
}

/* get called when the device remove from the system */
void pcd_platform_driver_remove(struct platform_device *pdev)
{
	pr_info("device is removed :p \r\n");

}

/* instance for the platfrom driver*/
struct platform_driver pcd_platform_driver =
{
	.probe 	= pcd_platform_driver_prob,
	.remove	= pcd_platform_driver_remove,
	.driver = {
		/*use the same, name used with the device for matching*/
		.name = "pcd-char-device",
	},
};


/* Module insertion section */
static int __init pcd_platform_driver_init(void)
{
	int ret;
	pr_info("platform driver init \r\n");
	
	/*1. Dynamically allocate the device number for MAX_DEVICES */
	ret = alloc_chrdev_region(&pcdrv_data.device_num_base, 0, MAX_DEVICES, "pcdevs");
	if(ret <0){
		pr_err("alloc chardev failed!");
		return ret;
	}
	
	/*2. create the class under /sys/class */
	pcdrv_data.pcd_class  = class_create("pcd_class");
        if(IS_ERR(pcdrv_data.pcd_class))
        {
                pr_err("class creation failed!\n");
                ret = PTR_ERR(pcdrv_data.pcd_class);
		unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
		return ret;
        }


	/*3. register the platform driver  */
	platform_driver_register(&pcd_platform_driver);
	pr_info("pcd_platform_driver loaded\r\n");
	return 0;
}
/* Module exit section */
static void __exit pcd_platform_driver_exit(void)
{
	pr_info("platform driver exited successfully\r\n");

	/*1. Unregister the platform driver */
	platform_driver_unregister(&pcd_platform_driver);

	/*2. class destroy*/
	class_destroy(pcdrv_data.pcd_class);

	/*3. Unregister the chrdev_region*/
	unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);

}


/* Module registartion section*/
module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_exit);

/* Module description section */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("pcd driver example");
MODULE_VERSION("1.0.0");

