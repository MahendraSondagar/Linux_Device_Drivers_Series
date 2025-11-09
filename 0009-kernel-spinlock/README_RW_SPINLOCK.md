#  Linux Kernel Tutorial — Read-Write Spinlock

##  Introduction

In the Linux kernel, a **read-write spinlock (`rwlock_t`)** is an advanced synchronization primitive that allows **multiple readers** to access a shared resource concurrently **as long as no writer is active**. However, when a writer thread needs to update the shared data, it must acquire **exclusive access**, blocking all readers and writers until it finishes.

This improves performance in **read-dominated workloads** — where many threads only read shared data, and writes are infrequent.

---

##  Why Do We Need Read-Write Spinlocks?

Let’s revisit a scenario:

You have **five threads**:
- **Thread 1** → writes (updates a global variable)
- **Thread 2–5** → only read (print or monitor the variable)

If you use a normal **spinlock**, only one thread (reader or writer) can enter the critical section at a time.  
That means if Thread 2 holds the lock to read, Threads 3–5 must wait — even though they’re only reading and not modifying anything!

This **reduces concurrency** and **wastes CPU cycles**.

 **Solution:** Use a **read-write spinlock**.  
- Multiple readers can enter the critical section together.
- A writer gets exclusive access when no readers are active.

---

##  Working of Read-Write Spinlock

| State | What happens |
|--------|---------------|
| No thread holds lock | Any reader or writer can acquire it |
| One or more readers hold lock | Other readers can join, but writers must wait |
| A writer holds lock | No reader or writer can acquire it until released |

 **Reader Priority:**  
Read-write spinlocks are **reader-preferential** — meaning if many readers continuously hold the lock, the writer may starve.  
If you want writer preference, use **Seqlock** instead.

---

##  Kernel API

```c
rwlock_t my_lock;

rwlock_init(&my_lock);

read_lock(&my_lock);
/* critical section (read) */
read_unlock(&my_lock);

write_lock(&my_lock);
/* critical section (write) */
write_unlock(&my_lock);
```

---

##  Practical Example — Linux Kernel Module

```c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_DESCRIPTION("Linux Read-Write Spinlock Example");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");

static int global_var = 0;

/* Thread objects */
static struct task_struct *writer_thread;
static struct task_struct *reader_thread1;
static struct task_struct *reader_thread2;

/* Define a read-write spinlock */
static rwlock_t rw_lock;

/* Writer thread: modifies the global variable */
static int writer_fn(void *data)
{
    pr_info("Writer thread started\n");

    while (!kthread_should_stop())
    {
        write_lock(&rw_lock);
        global_var++;
        pr_info("Writer updated global_var = %d\n", global_var);
        write_unlock(&rw_lock);

        msleep(1500);
    }
    return 0;
}

/* Reader thread: reads the global variable */
static int reader_fn(void *data)
{
    const char *name = data;

    pr_info("%s started\n", name);

    while (!kthread_should_stop())
    {
        read_lock(&rw_lock);
        pr_info("%s reading global_var = %d\n", name, global_var);
        read_unlock(&rw_lock);

        msleep(500);
    }
    return 0;
}

static int __init rw_spinlock_init(void)
{
    pr_info("Initializing Read-Write Spinlock Example\n");

    rwlock_init(&rw_lock);

    /* Create threads */
    writer_thread = kthread_run(writer_fn, NULL, "writer_thread");
    reader_thread1 = kthread_run(reader_fn, "Reader-1", "reader_thread1");
    reader_thread2 = kthread_run(reader_fn, "Reader-2", "reader_thread2");

    if (IS_ERR(writer_thread) || IS_ERR(reader_thread1) || IS_ERR(reader_thread2))
    {
        pr_err("Failed to create kernel threads\n");
        return -1;
    }

    return 0;
}

static void __exit rw_spinlock_exit(void)
{
    pr_info("Exiting Read-Write Spinlock Example\n");

    if (writer_thread)
        kthread_stop(writer_thread);
    if (reader_thread1)
        kthread_stop(reader_thread1);
    if (reader_thread2)
        kthread_stop(reader_thread2);
}

module_init(rw_spinlock_init);
module_exit(rw_spinlock_exit);
```

---

##  Expected Kernel Log Output

```
Initializing Read-Write Spinlock Example
Writer thread started
Reader-1 started
Reader-2 started
Reader-1 reading global_var = 0
Reader-2 reading global_var = 0
Writer updated global_var = 1
Reader-1 reading global_var = 1
Reader-2 reading global_var = 1
Writer updated global_var = 2
...
```

You’ll notice that **both readers can access the global variable simultaneously** without blocking each other, but they **pause when the writer updates** it.

---

##  When to Use Read-Write Spinlocks

 Use `rwlock_t` when:
- You have **many readers** and **few writers**
- Reads happen frequently, but writes are rare
- The critical section is small (short duration)
- Context is **atomic** (cannot sleep)

 Avoid if:
- Writes are frequent (as writers will starve)
- You need to sleep in the critical section (use `rw_semaphore` instead)

---

##  Summary

| Feature | Spinlock | Read-Write Spinlock |
|----------|-----------|---------------------|
| Multiple readers | ❌ Not allowed | ✅ Allowed |
| Single writer | ✅ Yes | ✅ Yes |
| Reader concurrency | ❌ Only one at a time | ✅ Multiple readers |
| Writer priority | N/A | ❌ Low |
| Used in | Atomic, SMP-safe sections | Read-dominant scenarios |

---

##  Author

**Mahendra Sondagar**  
*mahendrasondagar08@gmail.com*  
*Linux Kernel Synchronization Series*
