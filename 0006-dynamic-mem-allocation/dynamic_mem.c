#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>

/* pointer to point the starting address of the allocated block of the memory*/
void *ptr;

static int __init module_dynamic_mem_init(void)
{
	pr_info("Dynamically allocating the memory\r\n");
	ptr = kmalloc(100* sizeof(int), GFP_KERNEL);
	
	if(!ptr)
	{
		pr_err("Failed memory allocation");
		return -ENOMEM;
	}
	pr_info("Block of memory allocated!\r\n");
	return 0;
}

static void __exit module_dynamic_mem_exit(void)
{
	pr_info("De-allocating the memory\r\n");
	kfree(ptr);
}

module_init(module_dynamic_mem_init);
module_exit(module_dynamic_mem_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("kernel module for the DMA");
MODULE_VERSION("1.0.0");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
