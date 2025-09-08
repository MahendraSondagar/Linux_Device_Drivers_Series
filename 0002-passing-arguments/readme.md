
---

````markdown
# Linux Device Driver Tutorial - Part 2: Advanced Module Parameters ⚙️

**Author:** Mahendra Sondagar <mahendrasondagar08@gmail.com>  

In this tutorial, we extend our journey into Linux kernel modules by learning how to **pass arguments** to kernel modules at load time.  
We’ll cover integers, strings, arrays, and callback functions when parameters are updated.  

---

## Step 1: Source Code 📜

Create a file called **`adv_params.c`** with the following code:

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

int mod_param;
int arr_mod_param[5];
char *mod_char;
int cb_mod_param;

/* module param integer */
module_param(mod_param, int, S_IRUSR | S_IWUSR);

/* module param string */
module_param(mod_char, charp, S_IRUSR | S_IWUSR);

/* module param array */
module_param_array(arr_mod_param, int, NULL, S_IRUSR | S_IWUSR);

static int __init my_module_init(void)
{
    int i = 0;
    pr_info("mod_param: %d\r\n", mod_param);
    pr_info("mod_char: %s\r\n", mod_char);

    for (i = 0; i < sizeof(arr_mod_param)/sizeof(int); i++) {
        pr_info("arr_mod_param[%d]: %d\r\n", i, arr_mod_param[i]);
    }

    pr_info("Module init successful :P \r\n");
    return 0;
}

/*---------------------- Module_param_cb() --------------------------------*/
int notify_param(const char *val, const struct kernel_param *kp)
{
    int res = param_set_int(val, kp); // Use helper for writing variable
    if (res == 0) {
        pr_info("Callback function called...\n");
        pr_info("New value of cb_mod_param = %d\n", cb_mod_param);
        return 0;
    }
    return -1;
}

const struct kernel_param_ops my_param_ops = {
    .set = &notify_param,   // Use our setter ...
    .get = &param_get_int,  // .. and standard getter
};

module_param_cb(cb_mod_param, &my_param_ops, &cb_mod_param, S_IRUGO | S_IWUSR);

static void __exit my_module_exit(void)
{
    pr_info("Module exit successful\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("Device driver variable arguments");
MODULE_VERSION("1.0.0");
````

---

## Step 2: Header File Explanations 📚

* **`#include <linux/module.h>`**
  Provides core macros for writing modules (`module_init`, `module_exit`, `MODULE_LICENSE`, etc).

* **`#include <linux/kernel.h>`**
  Contains common kernel functions like `printk` and `pr_info`.

* **`#include <linux/init.h>`**
  Defines `__init` and `__exit` macros to optimize memory usage.

* **`#include <linux/moduleparam.h>`**
  Provides APIs to declare and manage module parameters (`module_param`, `module_param_array`, `module_param_cb`).

---

## Step 3: Understanding the Code 🧾

* **`module_param(mod_param, int, S_IRUSR | S_IWUSR)`**
  Declares an integer parameter `mod_param`. It can be set during `insmod`.

* **`module_param(mod_char, charp, S_IRUSR | S_IWUSR)`**
  Declares a string parameter `mod_char`.

* **`module_param_array(arr_mod_param, int, NULL, S_IRUSR | S_IWUSR)`**
  Declares an array of integers. Default length is 5 here.

* **Callback parameter with `module_param_cb`**

  * Links a parameter to custom callbacks (`set` and `get`).
  * The function `notify_param` is called whenever the value changes.
  * Useful for monitoring, validation, or triggering actions dynamically.

---

## Step 4: Makefile ⚙️

Create a **`Makefile`**:

```makefile
obj-m += adv_params.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERN_DIR =/home/mahi-ms/Documents/MyDrives/MyLearnings/Udemy-LDD/source/BBB-5.10.168-linux-ti-r83/
HOST_KERN_DIR= /lib/modules/$(shell uname -r)/build

all:
        make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
        make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

host:
        make -C $(HOST_KERN_DIR) M=$(PWD) modules
```

---

## Step 5: Building 🔨

For ARM target:

```bash
make
```

For Host machine:

```bash
make host
```

Clean:

```bash
make clean
```

---

## Step 6: Testing 🚀

### Insert module with parameters

```bash
sudo insmod adv_params.ko mod_param=42 mod_char="Mahendra" arr_mod_param=1,2,3,4,5 cb_mod_param=7
```

### Check kernel logs

```bash
dmesg | tail -n 20
```

Expected output:

```
mod_param: 42
mod_char: Mahendra
arr_mod_param[0]: 1
arr_mod_param[1]: 2
arr_mod_param[2]: 3
arr_mod_param[3]: 4
arr_mod_param[4]: 5
Callback function called...
New value of cb_mod_param = 7
Module init successful :P
```

### Remove module

```bash
sudo rmmod adv_params
```

Logs:

```
Module exit successful
```

---

## Step 7: Summary 📝

In this tutorial, we learned:

* How to declare **integer, string, and array module parameters**.
* How to implement a **callback function** for parameter updates using `module_param_cb`.
* The purpose of each **header file** used in module parameter handling.
* How to compile and test the driver on both host and ARM targets.

---

## 📌 Next Steps

In the next tutorial, we’ll explore **character device drivers**, where we will:

* Create device files under `/dev/`
* Implement `open`, `read`, `write`, and `release` file operations

Stay tuned 🚀

```

---

