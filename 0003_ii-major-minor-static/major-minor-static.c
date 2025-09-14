#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>

/* statically allocating the major & minor number */

dev_t device_number = MKDEV (255,0);

/* 255= major number, 0 = minor number */

/* Module init function */
static int __init module_pcd_init(void)
{
	if(register_chrdev_region(device_number, 1, "pcd_device")<0)
	{
		pr_info("Failed to register the device number!\r\n");
		return -1;
	}
        
	pr_info("Major = %d & Minor = %d\r\n", MAJOR(device_number), MINOR(device_number));
	pr_info("module init successfull\r\n");
	return 0;
}

/* Module exit function */
static void __exit module_pcd_exit(void)
{
	unregister_chrdev_region(device_number, 1);
	pr_info("module exit successfull \r\n");
}

/* Module registration */
module_init(module_pcd_init);
module_exit(module_pcd_exit);


/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra sondaqgar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("major minor static allocation");
MODULE_VERSION("1.0.0");
