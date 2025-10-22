#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

static struct timer_list mytimer;
int count =0;

/*Time callback function */
static void timer_callback_func(struct timer_list *t)
{
	pr_info("Timer callback called :[ %d]", count++);

	/*re-starts the timer for 1000 msec */
	mod_timer(&mytimer, jiffies + msecs_to_jiffies(1000));
}

static int __init module_timer_init(void)
{
	pr_info("module timer init");
	
	/*create the timer */
	timer_setup(&mytimer, timer_callback_func, 0);

	/*starts the timer of 1000 msec */
	mod_timer(&mytimer, jiffies + msecs_to_jiffies(1000));
	return 0;
}

static void __exit module_timer_exit(void)
{
	pr_info("module timer exit");
	/* delete t5he timer safely */
	del_timer_sync(&mytimer);
}

module_init(module_timer_init);
module_exit(module_timer_exit);


MODULE_DESCRIPTION("Linux kernel timer tutorial");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_VERSION("1.0.0");
MODULE_LICENSE("GPL");
