#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/delay.h>


/* object for the threads */
struct task_struct *thread_th1;
struct task_struct *thread_th2;

/* obj for the mutext */
struct mutex my_mutex;
int global_resource =0;


void access_precious_resource(void)
{
	pr_info("operation on precious resource: %d\r\n", global_resource++);
}
static int thread1_callback_fun(void *)
{
	pr_info("thread1_callback is executing\r\n");

	while(!kthread_should_stop())
	{
		pr_info("accessing the precious resource \r\n");

		mutex_lock(&my_mutex);
		access_precious_resource();
		mutex_unlock(&my_mutex);
		ssleep(1);
	}
	return 0;
}

static int thread2_callback_fun(void *)
{
	pr_info("thread2_callback is executing\r\n");
	while(!kthread_should_stop())
	{
		pr_info("accessing the precious resource\r\n");

		mutex_lock(&my_mutex);
		access_precious_resource();
		mutex_unlock(&my_mutex);
		ssleep(1);
	}

	return 0;
}

static int __init mutex_module_init(void)
{
	pr_info("mutex ex. module init\r\n");

	/*1. creating the first thread */
	thread_th1 = kthread_run(thread1_callback_fun, NULL, "my_thread1");
	if(IS_ERR(thread_th1))
	{
		pr_err("fail to creat first thread\r\n");
		return PTR_ERR(thread_th1);
	}

	/*2. creating the second  thread */
	thread_th2 = kthread_run(thread2_callback_fun, NULL, "my_thread2");
	if(IS_ERR(thread_th2))
	{
		pr_err("fail to creat first thread\r\n");
		return PTR_ERR(thread_th2);
	}

	/*init the mutex to avoid race condition */
	mutex_init(&my_mutex);

	return 0;
}

static void __exit mutex_module_exit(void)
{
	pr_info("mutex ex module exir \r\n");
	kthread_stop(thread_th1);
	kthread_stop(thread_th2);
}

module_init(mutex_module_init);
module_exit(mutex_module_exit);

MODULE_DESCRIPTION("Kernel mutex tutorial");
MODULE_AUTHOR("MahendraSondagar <mahendrasondagar08@gmail.com");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
