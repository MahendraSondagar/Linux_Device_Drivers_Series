#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>


static struct task_struct *my_thread;

static int thread_callback_fun(void *)
{
	int count =0;
	pr_info("Thread callback function called");

	/*check the status if the thread is not stopped */
	while(!kthread_should_stop())
	{
		pr_info("thread is running at the itegartion: %d\r\n", count++);
		/*1 sec delay i.e thread blocking state */ 
		ssleep(1);
	}
	return 0;
}


static int __init my_thread_mod_init(void)
{
	pr_info("Thread module has been loaded\r\n");
	
	/* @brief: creating the thread, and run immediately 
	 * @arg1: Thread callback function
	 * @arg2: parameters
	 * @arg3: debug name
	 * @retval: pointer */


	my_thread = kthread_run(thread_callback_fun, NULL, "my_thread");
	if(IS_ERR(my_thread))
	{
		pr_err("Thread creation failed!\r\n");
		return (PTR_ERR(my_thread));
	}
	return 0;
}

static void __exit my_thread_mod_exit(void)
{
	pr_info("Thread module exited");

	/* stopping the running thread*/ 
	if(my_thread)
	{
		kthread_stop(my_thread);
		pr_info("stopping the thread :p");
	}
}

module_init(my_thread_mod_init);
module_exit(my_thread_mod_exit);

MODULE_DESCRIPTION("KERNEL THREAD EXAMPLE");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MahendraSondagar<mahendarsondagar08gmail.com>");
MODULE_VERSION("1.0.0");

