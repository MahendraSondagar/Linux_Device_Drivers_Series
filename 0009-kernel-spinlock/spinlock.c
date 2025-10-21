#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>

int global_var =0;

/*thread objects */
struct task_struct *Thread_th1; 
struct task_struct *Thread_th2;

/*spinlock obj*/
spinlock_t my_lock;

static void access_precious_resource(void)
{
	pr_info("making operation on global_var: %d", global_var++);
}

static int thread1_callback_func(void *)
{
	pr_info("thread1 calback function executing ");

	while(!kthread_should_stop())
	{
		spin_lock(&my_lock);
		access_precious_resource();
		spin_unlock(&my_lock);
		msleep(1000);
	}
	return 0;
}

static int thread2_callback_func(void *)
{
	pr_info("thread2 calback function executing ");

	while(!kthread_should_stop())
	{
		spin_lock(&my_lock);
		access_precious_resource();
		spin_unlock(&my_lock);
		msleep(1000);
	}
	return 0;
}

static int __init module_spinlock_init(void)
{
	pr_info("Module spinlock init");
	Thread_th1 = kthread_run(thread1_callback_func, NULL, "my_thread1");
	if(IS_ERR(Thread_th1))
	{
		pr_err("failed to start thread1");
		return (PTR_ERR(Thread_th1));
	}

	Thread_th2 = kthread_run(thread2_callback_func, NULL, "my_thread2");
	if(IS_ERR(Thread_th2))
	{
		pr_err("failed to start thread2");
		return (PTR_ERR(Thread_th2));
	}

	spin_lock_init(&my_lock);

	return 0;
}

static void __exit module_spinlock_exit(void)
{
	pr_info("Module spinlock exit");
	kthread_stop(Thread_th2);
	kthread_stop(Thread_th2);
}

module_init(module_spinlock_init);
module_exit(module_spinlock_exit);

MODULE_DESCRIPTION("Linux spinlock example");
MODULE_AUTHOR("MahendarSondagar<mahendrasondagar08@gmail.com>");
MODULE_VERSION("1.0.0");
MODULE_LICENSE("GPL");
