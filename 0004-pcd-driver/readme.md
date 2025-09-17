# Pseudo Character Device Driver (pcd.c)

**Author:** Mahendra Sondagar <mahendrasondagar08@gmail.com>  

This tutorial explains the implementation of a simple **pseudo character device driver (PCD)** in Linux. The driver provides basic file operations such as open, read, write, release, and llseek. It also demonstrates interaction between **user space** and **kernel space** using kernel APIs.

---

## 📌 Overview

- Device name: `pcd_device`
- Driver registers dynamically allocated **major** and **minor** numbers.
- A memory buffer (`pcd_buffer`) of **512 bytes** is used to simulate device storage.
- Supported file operations:
  - `open`
  - `read`
  - `write`
  - `release`
  - `llseek`

---

## 📂 Code Walkthrough

### 1. Header Files

```c
#include <linux/kernel.h>   // Kernel helper macros (pr_info, pr_err, etc.)
#include <linux/module.h>   // Required for all kernel modules (module_init, module_exit, MODULE_LICENSE, etc.)
#include <linux/init.h>     // For __init and __exit macros
#include <linux/fs.h>       // File system support (struct file_operations, alloc_chrdev_region, etc.)
#include <linux/cdev.h>     // Character device structure (cdev)
#include <linux/device.h>   // For creating device nodes (class_create, device_create)
#include <linux/kdev_t.h>   // For major/minor number macros
#include <linux/uaccess.h>  // For copy_to_user, copy_from_user APIs
```

---

### 2. Device Memory

```c
#define DEV_MEM_SIZE   512U
char pcd_buffer[DEV_MEM_SIZE] = {0};
```

- A kernel buffer is allocated statically (`pcd_buffer`).
- This acts as device memory.

---

### 3. File Operations

#### `pcd_read()`

```c
ssize_t pcd_read (struct file * flip, char __user * buff, size_t count, loff_t * f_pos)
```

- Reads data from `pcd_buffer` into user-space buffer.
- Uses **copy_to_user()**.
- Adjusts count if request exceeds memory size.
- Updates file offset.

#### `pcd_write()`

```c
ssize_t pcd_write (struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
```

- Writes user-space data into `pcd_buffer`.
- Uses **copy_from_user()**.
- Truncates count if it exceeds device memory.
- Updates file offset.

#### `pcd_open()` & `pcd_release()`

- Print kernel logs when the file is opened or closed.

#### `pcd_llseek()`

```c
loff_t pcd_llseek (struct file * filp, loff_t offset, int whence)
```

- Changes file position pointer (`f_pos`) based on:
  - `SEEK_SET`: from start
  - `SEEK_CUR`: from current
  - `SEEK_END`: from end of device buffer

---

### 4. Kernel APIs Used

#### `copy_to_user()`

```c
unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
```

- Copies `n` bytes **from kernel space → to user space**.
- Returns:
  - `0` → success
  - Non-zero → number of bytes not copied (failure)

#### `copy_from_user()`

```c
unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
```

- Copies `n` bytes **from user space → to kernel space**.
- Returns:
  - `0` → success
  - Non-zero → number of bytes not copied (failure)

#### `IS_ERR()`

```c
long IS_ERR(const void *ptr);
```

- Checks if a pointer returned by kernel APIs is an **error code**.
- Returns `true` if `ptr` contains an error (negative encoded value).

#### `__user`

- A **sparse annotation** used in kernel code.
- Marks pointers that belong to **user space memory**.
- Helps static analyzers catch invalid memory accesses.

---

### 5. Device Registration

#### Allocating Device Numbers

```c
alloc_chrdev_region(&device_number, 0, 1, "pcd");
```

- Dynamically allocates a major and minor number.

#### Initializing & Adding `cdev`

```c
cdev_init(&pcd_cdev, &pcd_fops);
cdev_add(&pcd_cdev, device_number, 1);
```

- Binds file operations (`pcd_fops`) with the device.

#### Creating Sysfs Class & Device Node

```c
pcd_class  = class_create("pcd_class");
pcd_device = device_create(pcd_class, NULL, device_number, NULL, "pcd_device");
```

- Creates `/sys/class/pcd_class/pcd_device`.
- Also creates `/dev/pcd_device`.

---

### 6. Module Init & Exit

```c
static int __init pcd_module_init(void);
static void __exit pcd_module_exit(void);
```

- `module_init` → called when module is inserted (`insmod`).
- `module_exit` → called when module is removed (`rmmod`).

---

### 7. File Operations Table

```c
struct file_operations pcd_fops = {
  .open    = pcd_open,
  .read    = pcd_read,
  .write   = pcd_write,
  .llseek  = pcd_llseek,
  .release = pcd_release,
  .owner   = THIS_MODULE
};
```

---

## ⚙️ Compilation & Usage

### 1. Write a Makefile

```makefile
obj-m += pcd.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

### 2. Build Module

```bash
make
```

### 3. Insert Module

```bash
sudo insmod pcd.ko
```

Check kernel logs:

```bash
dmesg | tail
```

### 4. Verify Device Node

```bash
ls -l /dev/pcd_device
```

### 5. Test the Driver

#### Write Data

```bash
echo "Hello PCD" > /dev/pcd_device
```

#### Read Data

```bash
cat /dev/pcd_device
```

#### Seek & Write

```bash
dd if=/dev/pcd_device of=out.txt bs=10 count=1 skip=0
```

### 6. Remove Module

```bash
sudo rmmod pcd
```

---

## 🧾 Module Metadata

```c
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("Pseudo Character Device Driver Example");
MODULE_VERSION("1.0.0");
```

---

## ✅ Summary

- Implemented a pseudo character driver with **read, write, llseek, open, release**.
- Learned about:
  - `copy_to_user`, `copy_from_user`, `IS_ERR`
  - `alloc_chrdev_region`, `cdev_add`, `class_create`, `device_create`
- Created `/dev/pcd_device` for user interaction.
- Demonstrated module build, insertion, usage, and removal.
