#  Linux Kernel Mutex Tutorial

### Author: *Mahendra Sondagar*  
**Module:** `mutex_example.c`  
**License:** GPL v2  
**Version:** 1.0.0  

---

##  1. Definition

A **mutex (Mutual Exclusion)** is a synchronization mechanism used in the Linux kernel to protect shared resources from concurrent access.  
At any given time, **only one thread** can hold the mutex lock, ensuring mutual exclusion in critical code sections.

---

##  2. Why Mutex Is Required?

In a multitasking kernel, multiple threads or processes may run in parallel.  
If two or more threads access the same shared variable or hardware resource without synchronization, **data corruption** or **undefined behavior** can occur.

A mutex ensures:
- Only one thread at a time modifies shared data.
- Prevents race conditions.
- Protects critical regions of code safely.

---

##  3. What Is a Race Condition?

A **race condition** happens when multiple threads or processes attempt to access or modify the same shared data concurrently, leading to unpredictable results.

### Example:
```c
global_resource++;
```
If two threads execute this at the same time:
- Thread A reads `global_resource = 0`
- Thread B also reads `global_resource = 0`
- Both increment locally → `1`
- Both write back → `global_resource = 1` (instead of `2`)

 Result: **Data loss** due to unsynchronized access.

Using a **mutex** guarantees that one thread completes its operation before another begins.

---

##  4. Practical Use Case of the Mutex

When two kernel threads (created using `kthread_run()`) share and modify a global variable or hardware register, they may execute simultaneously.

To prevent conflicts:
- Mutex locks the resource before modification.
- Other threads are blocked until it’s released.
- Once the lock is released, the next thread continues safely.

Example scenarios:
- Updating a shared global variable.
- Writing to a common device register.
- Logging data to a shared memory buffer.

---

##  5. Linux Kernel Mutex API

Here are the most commonly used **mutex APIs** in the Linux kernel:

| API | Description |
|------|-------------|
| `DEFINE_MUTEX(name)` | Statically defines and initializes a mutex at compile time. |
| `mutex_init(struct mutex *lock)` | Dynamically initializes a mutex during runtime. |
| `mutex_lock(struct mutex *lock)` | Acquires the mutex; if already locked, the thread sleeps until available. |
| `mutex_trylock(struct mutex *lock)` | Tries to acquire the mutex without blocking — returns non-zero on success. |
| `mutex_unlock(struct mutex *lock)` | Releases the mutex. |
| `mutex_destroy(struct mutex *lock)` | Destroys a dynamically allocated mutex (optional cleanup). |

---

##  6. Example Code of the Mutex

```c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/delay.h>

struct task_struct *thread_th1;
struct task_struct *thread_th2;

struct mutex my_mutex;
int global_resource = 0;

void access_precious_resource(void)
{
    pr_info("operation on precious resource: %d\r\n", global_resource++);
}

static int thread1_callback_fun(void *data)
{
    pr_info("thread1_callback is executing\r\n");

    while (!kthread_should_stop()) {
        pr_info("thread1 accessing the precious resource\r\n");

        mutex_lock(&my_mutex);
        access_precious_resource();
        mutex_unlock(&my_mutex);

        ssleep(1);
    }
    return 0;
}

static int thread2_callback_fun(void *data)
{
    pr_info("thread2_callback is executing\r\n");

    while (!kthread_should_stop()) {
        pr_info("thread2 accessing the precious resource\r\n");

        mutex_lock(&my_mutex);
        access_precious_resource();
        mutex_unlock(&my_mutex);

        ssleep(1);
    }
    return 0;
}

static int __init mutex_module_init(void)
{
    pr_info("mutex example module init\r\n");

    mutex_init(&my_mutex);

    thread_th1 = kthread_run(thread1_callback_fun, NULL, "my_thread1");
    if (IS_ERR(thread_th1)) {
        pr_err("failed to create first thread\r\n");
        return PTR_ERR(thread_th1);
    }

    thread_th2 = kthread_run(thread2_callback_fun, NULL, "my_thread2");
    if (IS_ERR(thread_th2)) {
        pr_err("failed to create second thread\r\n");
        kthread_stop(thread_th1);
        return PTR_ERR(thread_th2);
    }

    return 0;
}

static void __exit mutex_module_exit(void)
{
    pr_info("mutex example module exit\r\n");

    if (thread_th1)
        kthread_stop(thread_th1);
    if (thread_th2)
        kthread_stop(thread_th2);
}

module_init(mutex_module_init);
module_exit(mutex_module_exit);

MODULE_DESCRIPTION("Kernel Mutex Tutorial");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
```

---

##  7. Output Example (Kernel Log)

```
mutex example module init
thread1_callback is executing
thread2_callback is executing
thread1 accessing the precious resource
operation on precious resource: 0
thread2 accessing the precious resource
operation on precious resource: 1
thread1 accessing the precious resource
operation on precious resource: 2
...
```

 No race condition occurs — the global resource increments sequentially.

---

##  8. Clean Up

```bash
sudo rmmod mutex_example
make
sudo insmod mutex_example.ko
dmesg
```

---

## 🔧 9. Detailed Description of Mutex API

### 🧱 `DEFINE_MUTEX(name)`
- Defines and initializes a mutex statically at compile time.
- Example:
  ```c
  DEFINE_MUTEX(my_mutex);
  ```
- Automatically initialized — no need to call `mutex_init()`.

---

###  `mutex_trylock(struct mutex *lock)`
- Attempts to acquire the mutex **without blocking**.
- Returns:
  - `1` → Success (lock acquired)
  - `0` → Failure (lock busy)
- Example:
  ```c
  if (mutex_trylock(&my_mutex)) {
      access_precious_resource();
      mutex_unlock(&my_mutex);
  } else {
      pr_info("Resource is busy, skipping access\n");
  }
  ```

---

###  `mutex_destroy(struct mutex *lock)`
- Cleans up dynamically allocated mutex objects.
- Optional for statically allocated mutexes.
- Example:
  ```c
  mutex_destroy(&my_mutex);
  ```

---

##  10. Key Takeaways

- Mutex ensures **exclusive access** to shared resources.
- Use `mutex_lock()` for blocking access, `mutex_trylock()` for non-blocking.
- Always call `mutex_unlock()` after your critical section.
- Prefer mutex over spinlocks for sleepable contexts.
- Mutexes prevent race conditions and maintain kernel stability.
