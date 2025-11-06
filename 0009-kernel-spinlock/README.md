# Linux Kernel Spinlock Tutorial

### Author: Mahendra Sondagar <mahendrasondagar08@gmail.com>

---

##  Overview

This project demonstrates how to use **spinlocks** in the Linux kernel to protect a shared resource between two kernel threads.

A **spinlock** is a synchronization primitive that prevents multiple threads or CPUs from accessing shared data simultaneously. Unlike mutexes, spinlocks **do not sleep** — they **busy-wait** until the lock becomes available.

---

##  Features

- Demonstrates basic spinlock usage
- Shows how to protect a shared global variable (`global_var`)
- Uses two kernel threads running concurrently
- Ensures race-free increments using `spin_lock()` and `spin_unlock()`

---

##  Files

| File | Description |
|------|--------------|
| `spinlock_demo.c` | Main kernel module demonstrating spinlock usage |
| `Makefile` | Build script for the kernel module |

---

##  Spinlock vs Mutex

| Feature | **Spinlock** | **Mutex** |
|----------|---------------|-----------|
| Sleep behavior | Busy-waits (spins) | Puts thread to sleep |
| Suitable for | Short critical sections | Long critical sections |
| Used in | Interrupt context | Process context |
| Overhead | Low | High |
| Blocking calls | Not allowed | Allowed |

---

##  How It Works

Two kernel threads (`my_thread1` and `my_thread2`) increment a shared variable `global_var`.  
Without synchronization, a **race condition** can occur.

A **spinlock** is used to protect access to the shared variable so that only one thread can modify it at a time.

### Key APIs

```c
spin_lock_init(&my_lock);
spin_lock(&my_lock);
spin_unlock(&my_lock);
```

---

##  Example Code

```c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>

int global_var = 0;
struct task_struct *Thread_th1;
struct task_struct *Thread_th2;
spinlock_t my_lock;

static void access_precious_resource(void)
{
    pr_info("Making operation on global_var: %d\n", global_var++);
}

static int thread_func(void *data)
{
    int id = *(int *)data;
    pr_info("Thread%d callback function executing\n", id);

    while (!kthread_should_stop()) {
        spin_lock(&my_lock);
        access_precious_resource();
        spin_unlock(&my_lock);
        msleep(1000);
    }
    return 0;
}

static int __init module_spinlock_init(void)
{
    pr_info("Module spinlock init\n");
    spin_lock_init(&my_lock);

    static int id1 = 1, id2 = 2;
    Thread_th1 = kthread_run(thread_func, &id1, "my_thread1");
    Thread_th2 = kthread_run(thread_func, &id2, "my_thread2");
    return 0;
}

static void __exit module_spinlock_exit(void)
{
    pr_info("Module spinlock exit\n");
    kthread_stop(Thread_th1);
    kthread_stop(Thread_th2);
    pr_info("Final value of global_var = %d\n", global_var);
}

module_init(module_spinlock_init);
module_exit(module_spinlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_DESCRIPTION("Linux Spinlock Example");
MODULE_VERSION("1.0.0");
```

---

##  Build Instructions

###  Create the Module

**Makefile**
```makefile
obj-m += spinlock_demo.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

###  Build the Module
```bash
make
```

###  Insert the Module
```bash
sudo insmod spinlock_demo.ko
```

###  Check Kernel Logs
```bash
dmesg | tail -n 20
```

###  Remove the Module
```bash
sudo rmmod spinlock_demo
```

---

##  Example Output

```
[  +0.000001] Module spinlock init
[  +0.001230] Thread1 callback function executing
[  +0.001234] Thread2 callback function executing
[  +1.002345] Making operation on global_var: 0
[  +1.002349] Making operation on global_var: 1
[  +2.003400] Making operation on global_var: 2
...
[  +5.002010] Module spinlock exit
[  +5.002050] Final value of global_var = 10
```

---

##  Notes

- Always initialize the spinlock before using it.
- Never use blocking calls (`msleep`, `schedule`) inside the locked section.
- Keep the critical section short for better performance.

---

##  Summary

| Concept | Description |
|----------|--------------|
| Purpose | Prevent race conditions in concurrent kernel code |
| Type | Busy-wait lock (non-sleeping) |
| Used In | Interrupt or atomic contexts |
| Example | Two kernel threads updating a global variable |

---

##  Author

**Mahendra Sondagar**  
📧 mahendrasondagar08@gmail.com  

---

> This tutorial is for educational purposes and tested on Linux kernel 6.x.
