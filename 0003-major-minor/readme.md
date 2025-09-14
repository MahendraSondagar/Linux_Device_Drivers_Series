# Linux Device Driver Tutorial - Part 3: Dynamic Major & Minor Number Allocation 🔢

**Author:** Mahendra Sondagar <mahendrasondagar08@gmail.com>  

In this tutorial, we will learn how to **dynamically allocate major and minor numbers** for a Linux character device driver.  
This is essential when writing modular drivers that do not use fixed major numbers.

---

## Step 1: Source Code 📜

**Source file name:** `major-minor.c`

```c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

/* dev_t typedef for holding major & minor number */
dev_t device_number;

/* Module Entry point */
static int __init pcd_module_init(void)
{
    pr_info("Module init successful\r\n");

    /* Dynamically allocate a major and minor number */
    if (alloc_chrdev_region(&device_number, 0, 1, "pcd_module") < 0) {
        pr_info("Failed to allocate dynamic major/minor number\r\n");
        return -1;
    }

    pr_info("Allocated Major: %d, Minor: %d \r\n", MAJOR(device_number), MINOR(device_number));
    return 0;
}

/* Module exit point */
static void __exit pcd_module_exit(void)
{
    /* De-registration of the device number */
    pr_info("Module exit successful\r\n");
    unregister_chrdev_region(device_number, 1);
}

/* Module registration */
module_init(pcd_module_init);
module_exit(pcd_module_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("Dynamically allocate major and minor numbers");
MODULE_VERSION("1.0.0");
```

---

## Step 2: Header File Explanation 📚

- **`#include <linux/kernel.h>`** - Provides logging functions like `pr_info`.  
- **`#include <linux/module.h>`** - Provides macros for writing loadable kernel modules (`module_init`, `module_exit`, `MODULE_*`).  
- **`#include <linux/fs.h>`** - Provides file system-related structures and APIs, including device numbers (`dev_t`, `MAJOR()`, `MINOR()`) and registration functions (`alloc_chrdev_region`, `unregister_chrdev_region`).  

---

## Step 3: Understanding dev_t & Major/Minor Number 🧾

- **`dev_t device_number;`**  
  - Type: **unsigned 32-bit integer** (`uint32_t`) on most architectures.  
  - Holds both **major** and **minor** numbers in a single variable.  

- **Bit Allocation**:  
  - The `dev_t` type splits bits to store major and minor numbers.  
  - Example (Linux typical default):  
    - **Major number:** upper 12 bits  
    - **Minor number:** lower 20 bits  
  - This allows **4096 major numbers** and **~1 million minor numbers** per major number.  

- **Macros**:  
  - `MAJOR(dev_t dev)` → extracts the **major number** from `dev_t`.  
  - `MINOR(dev_t dev)` → extracts the **minor number** from `dev_t`.  

- **Example**:  
```c
pr_info("Major: %d, Minor: %d
", MAJOR(device_number), MINOR(device_number));
```

---

## Step 4: Allocating & Releasing Device Numbers

- **`alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)`**  
  - Allocates `count` contiguous character device numbers dynamically.  
  - Stores the first allocated number in `*dev`.  
  - Returns 0 on success, <0 on failure.  

- **`unregister_chrdev_region(dev_t dev, unsigned count)`**  
  - Frees previously allocated device numbers.  
  - `dev` → the first `dev_t` number.  
  - `count` → number of device numbers to release.  
  - Must be called in **module exit** to avoid leaking device numbers.  

---

## Step 5: Makefile ⚙️

```makefile
obj-m += major-minor.o
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

## Step 6: Build Instructions 🔨

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

## Step 7: Testing 🚀

### Insert module
```bash
sudo insmod major-minor.ko
```

### Check kernel logs
```bash
dmesg | tail -n 10
```

Expected output:
```
Module init successful
Allocated Major: <major_number>, Minor: 0
```

### Remove module
```bash
sudo rmmod major-minor
```

Expected log:
```
Module exit successful
```

---

## Step 8: Summary 📝

In this tutorial, we learned:
- How to **dynamically allocate major and minor numbers** using `alloc_chrdev_region`.  
- How `dev_t` splits into **major and minor numbers** using bit allocation.  
- How to extract major/minor numbers using `MAJOR()` and `MINOR()`.  
- How to **release device numbers** using `unregister_chrdev_region()`.  
- The importance of freeing resources during module exit.  

---

## 📌 Next Steps

Next, we will explore:
- **Character device driver registration** using these major/minor numbers.  
- Creating device files under `/dev/` for read/write operations.  

Stay tuned 🚀
