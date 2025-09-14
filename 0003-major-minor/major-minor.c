#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>


/* uint32_t typedef for the to hold the major & Minor  number */
dev_t device_number;

/* Module Entry point */
static int __init pcd_module_init(void)
{
	pr_info("Module init successfull\r\n");
	/*Dynamically allocte the number  */
	if(alloc_chrdev_region(&device_number, 0, 1, "pcd_module")<0)
	{
		pr_info("dynamically major minor number failed to generate\r\n");
		return -1;
	}
	pr_info("major: %d, minor: %d \r\n", MAJOR(device_number), MINOR(device_number));
	return 0;

}


/* Module exit point */
static void __exit pcd_module_exit(void)
{
	/* De-registraration of the number */
	pr_info("module exit successfull\r\n");
	unregister_chrdev_region(device_number, 1);
}

/* Module registartion */

module_init(pcd_module_init);
module_exit(pcd_module_exit);

 /* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("Dynamically allocation for major, minor number");
MODULE_VERSION("1.0.0");
