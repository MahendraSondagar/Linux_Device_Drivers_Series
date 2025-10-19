#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *my_thread;

static int thread_callback_fun(void *)
{
	int idx = 0;
	pr_info("Thread callback hits\r\n");
	
	/*if thread is in running state */
	while(!kthread_should_stop())
	{
		pr_info("kthread iterations: %d\r\n", idx++);
		ssleep(1);
	}
	return 0;
}
static int __init module_thread_init(void)
{
	pr_info("Thread module init \r\n");
	
	/*This api will create the thread but not start*/
	my_thread = kthread_create(thread_callback_fun, NULL, "my_thread");
	if(IS_ERR(my_thread))
	{
		pr_err("Failed to creat the thread :P\r\n");
		return PTR_ERR(my_thread);
	}
	/* start the thread in more control mannaer*/

	wake_up_process(my_thread);
	return 0;
}


static void __exit module_thread_exit(void)
{
	pr_info("Thread module exit\r\n");
	/* Terminating the thread*/
	kthread_stop(my_thread);
}

module_init(module_thread_init);
module_exit(module_thread_exit);

MODULE_DESCRIPTION("kernel Thread tutorial");
MODULE_AUTHOR("MahendraSondagar<mahendrasondagar08@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
