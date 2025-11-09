#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>

/* instances for the thread and task */
static rwlock_t my_lock;
static struct task_struct *write_thread;
static struct task_struct *read_thread_1;
static struct task_struct *read_thread_2;

int global_var = 0;
int g_read_var;

/* write thread to update the global variable */
static int write_callback_func(void *p)
{
	while(!kthread_should_stop())
	{
		write_lock(&my_lock);
		global_var ++;
		pr_info("WRITE THREAD : global_var: %d", global_var);
		write_unlock(&my_lock);
		ssleep(1);
	}
	return 0;
}


/* read thread_1 to read the global variable */
static int read_callback_func_1(void *p)
{
	while(!kthread_should_stop())
	{
		read_lock(&my_lock);
		/* Copying (reading) the shared global variable */
		g_read_var = global_var;
		pr_info("READ THREAD 1: g_read_var: %d", g_read_var);
		read_unlock(&my_lock);
		ssleep(1);
	}
	return 0;
}

/* read thread_2 to read the global variable */
static int read_callback_func_2(void *p)
{
	while(!kthread_should_stop())
	{
		read_lock(&my_lock);
		/* Copying (reading) the shared global variable */
		g_read_var = global_var;
		pr_info("READ THREAD 2: g_read_var: %d", g_read_var);
		read_unlock(&my_lock);
		ssleep(1);
	}
	return 0;
}


static int __init rw_spinlock_module_init(void)
{
	pr_info("read-write spinlock init module");
	write_thread = kthread_run(write_callback_func, NULL, "write_thread");
	read_thread_1 = kthread_run(read_callback_func_1, NULL, "read_thread_1");
	read_thread_2 = kthread_run(read_callback_func_2, NULL, "read_thread_2");

	if(IS_ERR(write_thread) || IS_ERR(read_thread_1) || IS_ERR(read_thread_2))
	{
		pr_err("failed to create the threads");
		return -1;
	}

	/*init read-write spinlock*/
	rwlock_init(&my_lock);

	return 0;
}

static void __exit rw_spinlock_module_exit(void)
{
	pr_info("read-write spinlock exit module");
	 if(write_thread)
		 kthread_stop(write_thread);
	 if(read_thread_1)
		 kthread_stop(read_thread_1);
	 if(read_thread_2)
		 kthread_stop(read_thread_2);
}

module_init(rw_spinlock_module_init);
module_exit(rw_spinlock_module_exit);

MODULE_DESCRIPTION("Linux kernel rw_spinlock example");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_VERSION("1.0.0");
MODULE_LICENSE("GPL");
