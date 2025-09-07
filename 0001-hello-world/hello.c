#include <linux/module.h>

/* module entry point*/
static int  __init hello_world_init(void)
{
	pr_info("Hey.. Hi.. Hello..\r\n");
	return 0;
}

/* Module exit point */
static void __exit hello_word_exit(void)
{
	pr_info("Good bye world :p \r\n");
}


/* module registration */
module_init(hello_world_init);
module_exit(hello_word_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("My first module");

