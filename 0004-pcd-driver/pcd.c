#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

/* device memory buffer size */
#define DEV_MEM_SIZE   512U

/* kernel buffer of the pcd driver*/
char pcd_buffer[DEV_MEM_SIZE] = {0};

/* file operations from the file_operations struct of fs.h */
ssize_t pcd_read (struct file * flip, char __user * buff, size_t count, loff_t * f_pos)
{
	pr_info("requested to read bytes: %zu \r\n", count);
	pr_info("previous file position : %lld\r\n", *f_pos);
	/* 1. adjust the  count */
	if((*f_pos + count) > DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *f_pos;

	/* 2. copy_to_user */
	if(!copy_to_user(&buff[0], &pcd_buffer[*f_pos], count))
		return -EFAULT;
	/* 3. update the f_pos w.r.t count */
	*f_pos += count;

	pr_info("No of bytes read from pcd_read: %zu\r\n", count);
	pr_info("Update file position: %lld\r\n", *f_pos);
	return count;
}

/* write operations from user space to kernel space */
ssize_t pcd_write (struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
{
    pr_info("Requested bytes to write: %zu\n", count);
    pr_info("Previous file position: %lld\n", *f_pos);

    /* 1. validate the count */
    if ((*f_pos + count) > DEV_MEM_SIZE)
        count = DEV_MEM_SIZE - *f_pos;

    if (!count)
        return -ENOMEM;

    /* 2. copy_from_user */
    if (copy_from_user(&pcd_buffer[*f_pos], buff, count))
        return -EFAULT;

    /* 3. update f_pos */
    *f_pos += count;

    /* Add a terminating null so printk shows it as a string */
    if (*f_pos < DEV_MEM_SIZE)
        pcd_buffer[*f_pos] = '\0';

    pr_info("User wrote: %s\n", pcd_buffer);
    pr_info("Current file position: %lld\n", *f_pos);

    return count;
}


/* open the device driver file */
int pcd_open (struct inode * inode, struct file * flip)
{
	pr_info("open file operation called\r\n");
	return 0;
} 

/* close the device file  */
int pcd_release (struct inode * inode, struct file * flip)
{
	pr_info("release operation called\r\n");
	return 0;
}

/* lseek the current file position pointer */
loff_t pcd_llseek (struct file * filp, loff_t offset, int whence)
{
	pr_info("lseek operation called\r\n");

	switch (whence)
	{
		case SEEK_SET:
			filp->f_pos = offset;
			break;
		case SEEK_CUR:
			filp->f_pos += offset;
			break;
		case SEEK_END:
			filp->f_pos = DEV_MEM_SIZE + offset;
			break;
		default:
			return -EINVAL;
	}
	return filp->f_pos;
}

/* uint32_t variable to hold the major(12 bit) + minor(20 bit) number */
dev_t device_number;

/* cdev structure variable */
struct cdev pcd_cdev;
struct file_operations pcd_fops =
{
	.open    = pcd_open,
	.write   = pcd_write,
	.read    = pcd_read,
	.llseek  = pcd_llseek,
	.release = pcd_release,
	.owner   = THIS_MODULE
};

/*class and device structure variable */
struct class *pcd_class;
struct device *pcd_device;

/* Module insertion section */
static int __init pcd_module_init(void)
{
	int retval;
	/* 1. dynamically creating the major & minor numbers */
	retval = alloc_chrdev_region(&device_number, 0, 1, "pcd");
		if(retval < 0)
			goto exit;

	/* printing the major & minor numbers */
	pr_info("Major : %d Minor : %d\r\n", MAJOR(device_number), MINOR(device_number));

	/* 2. registration of the major & minor numbers  with the VFS (virtual file system) */
	cdev_init(&pcd_cdev, &pcd_fops);

	/* 2(i) owner init */
	pcd_cdev.owner = THIS_MODULE;

	retval= cdev_add(&pcd_cdev, device_number, 1);
	if(retval<0)
		goto unreg_device;
	
	/* 3. create the class and device and register with the sysfs i.e creating the class under /sys/class*/

	pcd_class  = class_create("pcd_class");
	if(IS_ERR(pcd_class))
	{
		pr_err("class creation failed!\n");
		retval = PTR_ERR(pcd_class);
		goto cdev_del;
	}
	pcd_device = device_create(pcd_class, NULL, device_number, NULL, "pcd_device");
	if(IS_ERR(pcd_device))
	{
		pr_err("device create failed\n");
		retval = PTR_ERR(pcd_device);
		goto class_destroy;
	}

	pr_info("pcd module init successfully \r\n");
	return 0;

class_destroy:
	class_destroy(pcd_class);

cdev_del:
	cdev_del(&pcd_cdev);

unreg_device:
	unregister_chrdev_region(device_number, 1);

exit:
	pr_info("Module insertion failed!\n");
	return retval;
}

/* Module exit section */
static void __exit pcd_module_exit(void)
{
	device_destroy(pcd_class, device_number);
	class_destroy(pcd_class);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number, 1);
	pr_info("pcd module exited successfully\r\n");
}


/* Module registartion section*/
module_init(pcd_module_init);
module_exit(pcd_module_exit);

/* Module description section */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("pcd driver example");
MODULE_VERSION("1.0.0");

