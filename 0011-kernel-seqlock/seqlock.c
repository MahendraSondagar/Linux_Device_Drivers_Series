#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/seqlock.h>
#include <linux/kthread.h>
#include <linux/delay.h>


static struct task_struct *thread_th1;
static struct task_struct *thread_th2;
seqlock_t my_seqlock;
int global_var =0;

static int read_callback_func(void *)
{
	unsigned int seq_no;
	int g_copy;
	while(!kthread_should_stop())
	{
		do
		{
			seq_no = read_seqbegin(&my_seqlock);
			g_copy = global_var;
		}while(read_seqretry(&my_seqlock, seq_no));
		pr_info("read values : %d\r\n", g_copy);
		ssleep(1);
	}

	return 0;
}

static int write_callback_func(void *)
{
	while(!kthread_should_stop())
	{
		write_seqlock(&my_seqlock);
		global_var ++;
		write_sequnlock(&my_seqlock);
		ssleep(1);
	}

	return 0;
}

static int __init module_seqlock_init(void)
{
	pr_info("module seqlock init fun");
	thread_th1 = kthread_run(write_callback_func, NULL, "thread_1");
	if(IS_ERR(thread_th1))
	{
	return (PTR_ERR(thread_th1));
	}

	thread_th2 = kthread_run(read_callback_func, NULL, "thread_2");
	if(IS_ERR(thread_th2))
	{
	return (PTR_ERR(thread_th2));
	}

	seqlock_init(&my_seqlock);

	return 0;
}

static void __exit module_seqlock_exit(void)
{
	pr_info("module seqlock exit function");
	if(thread_th1)
	{
		kthread_stop(thread_th1);
	}

	if(thread_th2)
	{
		kthread_stop(thread_th2);
	}
}

module_init(module_seqlock_init);
module_exit(module_seqlock_exit);

MODULE_DESCRIPTION("Linux kernel seqlock tutorial");
MODULE_VERSION("1.0.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
