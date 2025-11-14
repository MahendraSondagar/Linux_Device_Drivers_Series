

---

## Introduction  
Wait queues in the Linux kernel allow a process to **sleep** until a certain condition becomes true.  
They are essential when dealing with:  

- Synchronization  
- Device drivers  
- Blocking read/write operations  
- Event-based wake-ups  

Wait queues prevent CPU wastage and enable smooth thread scheduling.

---

#  What Is a Wait Queue?

A **wait queue** is a kernel mechanism that lets a thread sleep until an event occurs.

### Two main components:

1. **Wait Queue Head**  
2. **Wait Queue Entry (one or more processes)**  

---

#  Basic Structure

```c
wait_queue_head_t my_queue;
init_waitqueue_head(&my_queue);
```

A process adds itself to the wait queue when it needs to sleep until a condition becomes true.

---

#  How Sleeping Works

Sleeping thread uses:  

```c
wait_event(my_queue, condition);
```

Waking is triggered by:

```c
wake_up(&my_queue);
```

---

#  Types of Wait Macros

### 1. Uninterruptible Sleep  
```c
wait_event(queue, condition);
```

### 2. Interruptible Sleep  
```c
wait_event_interruptible(queue, condition);
```

### 3. Sleep With Timeout  
```c
wait_event_timeout(queue, condition, timeout);
```

### 4. Interruptible Sleep with Timeout  
```c
wait_event_interruptible_timeout(queue, condition, timeout);
```

---

#  Detailed API Explanations with Examples

---

## 1️⃣ **wait_event(queue, condition)**  
Uninterruptible sleep until condition becomes true.

```c
wait_event(my_queue, data_ready == 1);
```

---

## 2️⃣ **wait_event_interruptible(queue, condition)**  
Interruptible by signals (e.g., Ctrl+C).

```c
if (wait_event_interruptible(my_queue, data_ready))
    return -ERESTARTSYS;
```

---

## 3️⃣ **wait_event_timeout(queue, condition, timeout)**  
Waits until condition or timeout (in jiffies).

```c
ret = wait_event_timeout(my_queue, flag == 1, 5 * HZ);
```

---

## 4️⃣ **wait_event_interruptible_timeout(queue, condition, timeout)**  
Combination of interruptible sleep + timeout.

```c
ret = wait_event_interruptible_timeout(my_queue, flag, 3 * HZ);
```

---

#  Wake-up APIs

---

## 5️⃣ **wake_up(&queue)**  
Wakes all blocking sleepers (interruptible + uninterruptible).

```c
data_ready = 1;
wake_up(&my_queue);
```

---

## 6️⃣ **wake_up_interruptible(&queue)**  
Wakes only interruptible waiters.

```c
data_ready = 1;
wake_up_interruptible(&my_queue);
```

---

## 7️⃣ **wake_up_all(&queue)**  
Wakes every task sleeping on this queue.

```c
wake_up_all(&my_queue);
```

---

## 8️⃣ **wake_up_sync(&queue)**  
Avoids rescheduling immediately — performance benefit.

```c
wake_up_sync(&my_queue);
```

Cannot be used in interrupt context.

---

## 9️⃣ **wake_up_interruptible_sync(&queue)**  
Interruptible wake + safe for interrupts.

```c
wake_up_interruptible_sync(&my_queue);
```

---

#  Practical Example — Producer/Consumer

---

##  Kernel Module Example

```c
static DECLARE_WAIT_QUEUE_HEAD(my_queue);
static int data_ready = 0;

ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    wait_event_interruptible(my_queue, data_ready == 1);
    data_ready = 0;
    return 1;
}

ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    data_ready = 1;
    wake_up_interruptible(&my_queue);
    return len;
}
```

---

#  Summary Table

| API | Sleep Type | Interruptible | Timeout | Who Wakes | Use Case |
|-----|-------------|---------------|---------|------------|----------|
| `wait_event()` | Uninterruptible | ❌ No | ❌ No | — | Internal processes |
| `wait_event_interruptible()` | Interruptible | ✅ Yes | ❌ No | — | User-space ops |
| `wait_event_timeout()` | Uninterruptible | ❌ No | ✅ Yes | — | Time-based events |
| `wake_up()` | — | — | — | All sleepers | Normal wake |
| `wake_up_interruptible()` | — | — | — | Interruptible sleepers | Safe user wake |
| `wake_up_all()` | — | — | — | Everyone | Broadcast wake |
| `wake_up_sync()` | — | — | — | All sleepers | Avoid reschedule |
| `wake_up_interruptible_sync()` | — | — | — | Interruptible | ISR-safe |

---

#  Final Notes

- Wait queues are foundational in Linux synchronization.
- Best suited when threads must sleep waiting for an event.
- More efficient than busy-looping.
- Essential in drivers, file operations, blocking I/O.

---

# © Author  
**Mahendra Sondagar**  
📧 *mahendrasondagar08@gmail.com*
