#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

int mod_param;
int arr_mod_param[5];
char *mod_char;
int cb_mod_param;
/* module param initger */
module_param(mod_param, int,  S_IRUSR | S_IWUSR);
/* module param string */
module_param(mod_char, charp, S_IRUSR | S_IWUSR);
/* module param array */
module_param_array(arr_mod_param, int, NULL, S_IRUSR | S_IWUSR);

static int __init my_module_init(void)
{
	int i=0;
        pr_info("mod_param: %d\r\n", mod_param);
	pr_info("mod_char: %s\r\n", mod_char);
	for (i=0; i< sizeof(arr_mod_param)/sizeof(int); i++)
	{
		pr_info("arr_mod_param[%d]: %d\r\n", i, arr_mod_param[i]);
	}
	pr_info("Module init successfull :P \r\n");
	return 0;
}

/*----------------------Module_param_cb()--------------------------------*/
int notify_param(const char *val, const struct kernel_param *kp)
{
        int res = param_set_int(val, kp); // Use helper for write variable
        if(res==0) {
                pr_info("Call back function called...\n");
                pr_info("New value of cb_valueETX = %d\n", cb_mod_param);
                return 0;
        }
        return -1;
}

const struct kernel_param_ops my_param_ops =
{
        .set = &notify_param, // Use our setter ...
        .get = &param_get_int, // .. and standard getter
};

module_param_cb(cb_mod_patram, &my_param_ops, &cb_mod_param, S_IRUGO|S_IWUSR );

static void __exit my_module_exit(void)
{
	pr_info("Module exit successfull");
}


module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("Device driver variable arguments");
MODULE_VERSION("1.0.0");


