#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/delay.h>

/* thread handler */
static struct task_struct *dispatcher_instance;
static struct task_struct *handler_instance;

/* wait queue handler*/
wait_queue_head_t wq;

static int g_event =0;


static int dispatcher_callback_func(void *p)
{
	while(!kthread_should_stop())
	{
		pr_info("sleeping the dispatcher for 5 sec");
		ssleep(5);
		pr_info("setting the g_event: %d", ++g_event);
		wake_up_interruptible(&wq);
	}
	return 0;
}

static int handler_callback_func(void *p)
{
	while(!kthread_should_stop())
	{
		pr_info("Waiting for the event from the handler func...");
		wait_event_interruptible(wq, g_event ==1);
		pr_info("Event received at handler");
		g_event =0;
	}
	return 0;
}


static int __init waitqueue_module_init(void)
{
	pr_info("waitqueue module init");

	dispatcher_instance = kthread_run(dispatcher_callback_func, NULL, "dispatcher");
	handler_instance =    kthread_run(handler_callback_func, NULL, "handler");

	if(IS_ERR(dispatcher_instance) || IS_ERR(handler_instance))
	{
		pr_err("Failed to create the thread");
		return -1;
	}

	/*waitqueue init */
	init_waitqueue_head(&wq);
	return 0;
}

static void __exit waitqueue_module_exit(void)
{
	pr_info("waitqueue module exit");
	if(dispatcher_instance)
	{
		kthread_stop(dispatcher_instance);
	}

	if(handler_instance)
	{
		kthread_stop(handler_instance);
	}

}


module_init(waitqueue_module_init);
module_exit(waitqueue_module_exit);


MODULE_DESCRIPTION("kernal waitqueue tutorial");
MODULE_AUTHOR("MahendraSondagar<mahendrasondagar08@gmail.com>");
MODULE_VERSION("1.0.0");
MODULE_LICENSE("GPL");


