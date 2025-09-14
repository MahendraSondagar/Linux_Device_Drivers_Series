# Linux Device Driver Tutorial - Part 3(ii): Static Major & Minor Number Allocation 🔢

**Author:** Mahendra Sondagar <mahendrasondagar08@gmail.com>  

In this tutorial, we will learn how to **statically allocate major and minor numbers** for a Linux character device driver.  
This method assigns fixed numbers to your device rather than requesting dynamic allocation from the kernel.

---

## Step 1: Source Code 📜

**Source file name:** `major-minor-static.c`

```c
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>

/* Statically allocating the major & minor number */
dev_t device_number = MKDEV(255, 0);  /* 255 = major, 0 = minor */

/* Module init function */
static int __init module_pcd_init(void)
{
    if (register_chrdev_region(device_number, 1, "pcd_device") < 0)
    {
        pr_info("Failed to register the device number!\r\n");
        return -1;
    }

    pr_info("Major = %d & Minor = %d\r\n", MAJOR(device_number), MINOR(device_number));
    pr_info("Module init successful\r\n");
    return 0;
}

/* Module exit function */
static void __exit module_pcd_exit(void)
{
    unregister_chrdev_region(device_number, 1);
    pr_info("Module exit successful\r\n");
}

/* Module registration */
module_init(module_pcd_init);
module_exit(module_pcd_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("Major Minor Static Allocation");
MODULE_VERSION("1.0.0");
```

---

## Step 2: Header File Explanation 📚

- **`#include <linux/module.h>`** - Provides macros and functions for kernel modules (`module_init`, `module_exit`, `MODULE_LICENSE`, etc.).  
- **`#include <linux/fs.h>`** - Provides file system-related structures and APIs, including `dev_t`, `register_chrdev_region`, and `unregister_chrdev_region`.  
- **`#include <linux/init.h>`** - Provides `__init` and `__exit` macros for module initialization and cleanup.  
- **`#include <linux/kdev_t.h>`** - Provides macros to create device numbers (`MKDEV`) and extract major/minor numbers (`MAJOR`, `MINOR`).  

---

## Step 3: Understanding the Code 🧾

- **`dev_t device_number = MKDEV(255, 0);`**  
  - `MKDEV(major, minor)` combines major and minor numbers into a single `dev_t` variable.  
  - Here, **255** is the major number and **0** is the minor number.  

- **`register_chrdev_region(dev_t dev, unsigned count, const char *name)`**  
  - Registers a **statically allocated device number**.  
  - `dev` → the first `dev_t` device number.  
  - `count` → number of contiguous minor numbers to register (here 1).  
  - `name` → device name as shown in `/proc/devices`.  
  - Returns 0 on success, negative on failure.  

- **`MAJOR(dev_t dev)` & `MINOR(dev_t dev)`**  
  - Extract major and minor numbers from the `dev_t` variable.  
  - Example:  
```c
pr_info("Major = %d & Minor = %d
", MAJOR(device_number), MINOR(device_number));
```  

- **`unregister_chrdev_region(dev_t dev, unsigned count)`**  
  - Releases the previously registered device number.  
  - Must be called in **module exit** to prevent conflicts.  

---

## Step 4: Makefile ⚙️

```makefile
obj-m += major-minor-static.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERN_DIR=/home/mahi-ms/Documents/MyDrives/MyLearnings/Udemy-LDD/source/BBB-5.10.168-linux-ti-r83/
HOST_KERN_DIR=/lib/modules/$(shell uname -r)/build

all:
        make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
        make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

host:
        make -C $(HOST_KERN_DIR) M=$(PWD) modules
```

---

## Step 5: Build Instructions 🔨

Cross-compile for ARM target:
```bash
make
```

Build for Host machine:
```bash
make host
```

Clean build:
```bash
make clean
```

---

## Step 6: Testing 🚀

### Insert module
```bash
sudo insmod major-minor-static.ko
```

### Check kernel logs
```bash
dmesg | tail -n 10
```

Expected output:
```
Major = 255 & Minor = 0
Module init successful
```

### Remove module
```bash
sudo rmmod major-minor-static
```

Expected log:
```
Module exit successful
```

---

## Step 7: Summary 📝

In this tutorial, we learned:
- How to **statically allocate major and minor numbers** using `MKDEV`.  
- How to register device numbers with `register_chrdev_region`.  
- How to extract major and minor numbers using `MAJOR()` and `MINOR()`.  
- How to release device numbers using `unregister_chrdev_region`.  

---

## 📌 Next Steps

Next, we will explore:
- **Character device registration and file operations** (`open`, `read`, `write`, `release`).  
- Creating `/dev` device files to interact with the driver.  

Stay tuned 🚀
