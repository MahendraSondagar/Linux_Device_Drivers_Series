# Linux Device Driver Tutorial - Part 1: Hello World 🌍

**Author:** Mahendra Sondagar <mahendrasondagar08@gmail.com>  

Welcome to the **Linux Device Driver Tutorial Series**!  
In this first part, we’ll create the classic **Hello World** kernel module, understand its building blocks, and learn how to compile and test it both on the host machine and on an ARM-based target board (like BeagleBone Black).

---

## Step 1: Writing the Module Source Code 📜

Create a file called **`hello.c`** with the following code:

```c
#include <linux/module.h>

/* module entry point*/
static int  __init hello_world_init(void)
{
        pr_info("Hey.. Hi.. Hello..\r\n");
        return 0;
}

/* Module exit point */
static void __exit hello_word_exit(void)
{
        pr_info("Good bye world :p \r\n");
}

/* module registration */
module_init(hello_world_init);
module_exit(hello_word_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("My first module");
```

---

## Step 2: Understanding the Code 🧾

### `#include <linux/module.h>`
- Provides the functions and macros needed for Linux kernel modules.

### Entry Function: `static int __init hello_world_init(void)`
- Called **once** when the module is loaded using `insmod`.
- `__init` macro:
  - Places this function in a special `.init.text` section.
  - After initialization, the kernel can free up this memory.
  - Prevents accidental use of this function later (saves memory).

### Exit Function: `static void __exit hello_word_exit(void)`
- Called **once** when the module is unloaded using `rmmod`.
- `__exit` macro:
  - Marks the function as cleanup-only.
  - If the module is compiled directly into the kernel, the function can be safely discarded.

### Why use `__init` and `__exit`?
- Memory in the kernel is precious (especially on embedded systems).  
- These macros help optimize memory usage by discarding one-time-use functions when not needed.

### Logging with `pr_info`
- Like `printf` but for kernel space.  
- Messages appear in the **kernel ring buffer** (`dmesg` command).

### Module Metadata
- `MODULE_LICENSE("GPL")` → Ensures access to GPL-only symbols.
- `MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>")` → Author info.
- `MODULE_DESCRIPTION("My first module")` → Brief description.

---

## Step 3: Creating the Makefile ⚙️

Create a **`Makefile`** with the following content:

```makefile
obj-m += hello.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERN_DIR =/home/mahi-ms/Documents/MyDrives/MyLearnings/Udemy-LDD/source/BBB-5.10.168-linux-ti-r83/
HOST_KERN_DIR= /lib/modules/$(shell uname -r)/build

all:
        make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean: 
        make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

help:
        make -C $(KERN_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) help

host:
        make -C $(HOST_KERN_DIR) M=$(PWD) modules
```

---

## Step 4: Building the Module 🔨

### Build for ARM target (cross-compile)
```bash
make
```

### Build for Host Machine
```bash
make host
```

### Clean the build
```bash
make clean
```

---

## Step 5: Testing the Module 🚀

### Insert the module
```bash
sudo insmod hello.ko
```

### Check kernel logs
```bash
dmesg | tail
```

Expected output:
```
Hey.. Hi.. Hello..
```

### Remove the module
```bash
sudo rmmod hello
```

### Check logs again
```
Good bye world :p
```

---

## Step 6: Summary 📝

In this tutorial, we learned:
- How to write a **Hello World Linux kernel module**.
- The purpose of `__init` and `__exit` macros in optimizing kernel memory usage.
- How to use `pr_info` for kernel logging.
- How to write a `Makefile` for both cross-compilation and host builds.
- How to insert and remove kernel modules.

---

## 📌 Next Steps

In the next tutorial, we will explore:
- **Module parameters** → how to pass arguments to a kernel module at load time.  
- **Character drivers** → introduction to handling device files (`/dev/...`).  

Stay tuned 🚀
