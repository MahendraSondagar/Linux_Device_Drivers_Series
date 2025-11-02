
# 🧠 SeqLock in Linux Kernel — Complete In-Depth Tutorial

## 1. Introduction

In concurrent kernel programming, maintaining **data consistency** between **readers and writers** is critical.  
The Linux kernel provides various synchronization primitives such as **spinlocks, mutexes, RCU, and seqlocks**.

👉 **SeqLock (Sequential Lock)** is a special synchronization mechanism optimized for cases where:
- **Reads are more frequent** than writes.
- **Writers can tolerate redoing work** if readers detect inconsistency.

It is widely used in kernel areas like **timekeeping, VDSO**, and **statistical counters**.

---

## 2. Why SeqLock is Needed

Imagine a scenario where:
- A shared data structure (say `system_time`) is updated periodically by a writer.
- Many readers are reading it frequently.

If we use a **spinlock**:
- Writers and readers both block each other → high contention.

If we use **RCU**:
- Readers can read concurrently.
- But writers need to create a new copy, which is memory-heavy.

Hence → **SeqLock** is introduced to:
✅ Allow multiple readers to read **without taking a lock**,  
✅ Let the writer **serialize updates**,  
✅ And detect if a reader saw inconsistent data.

---

## 3. How SeqLock Works Internally

### 3.1 Key Idea

- Writers **increment a sequence number** (even→odd→even).
- Readers **check the sequence** before and after reading data.

If the sequence changed → a writer modified data → reader retries.

---

### 3.2 Sequence Flow

| Step | Action | Sequence Number |
|------|---------|----------------|
| Idle (no write) |  | Even (e.g., 100) |
| Writer begins | seq++ | Odd (101) |
| Writer updates data | | Odd (101) |
| Writer ends | seq++ | Even (102) |

Readers only trust data when sequence number is **even** and **unchanged**.

---

## 4. Linux Kernel API for SeqLock

Declared in:
```c
#include <linux/seqlock.h>
```

### 4.1 Data Types

```c
seqlock_t my_seqlock;         
seqcount_t my_seqcount;       
```

Use `seqlock_t` when both readers and writers exist.  
Use `seqcount_t` when you already have your own locking mechanism.

---

### 4.2 API Functions

#### Initialization
```c
seqlock_t my_seqlock = __SEQLOCK_UNLOCKED(my_seqlock);
seqlock_init(&my_seqlock);
```

#### Writer Side
```c
write_seqlock(&my_seqlock);
/* modify shared data */
write_sequnlock(&my_seqlock);
```

#### Reader Side
```c
unsigned int seq;
do {
    seq = read_seqbegin(&my_seqlock);
    /* read shared data */
} while (read_seqretry(&my_seqlock, seq));
```

---

## 5. Example — Timekeeping Example (Practical Use Case)

SeqLocks are heavily used in the kernel's **timekeeping** code because time values are read extremely frequently (user `gettimeofday()`, `clock_gettime()`) while updates are relatively rare and short. This section shows a practical, compact example that mirrors how the kernel keeps a coherent snapshot of time across readers and writers.

### 5.1 Problem Context

- A writer (timer interrupt or periodic kernel thread) updates `xtime`/`monotonic` fields.
- Many readers (user-space via VDSO or kernel callers) need a consistent snapshot to compute time.
- Blocking readers with spinlocks hurts performance; SeqLock allows lockless reads with validation.

### 5.2 Data Structures

```c
#include <linux/seqlock.h>

struct tk_clock {
    unsigned long seconds;
    unsigned long nsec;
    seqlock_t seq; /* protects seconds/nsec */
};

static struct tk_clock tk;
```

### 5.3 Writer (timer-like update)

```c
void tk_update(unsigned long delta_nsec)
{
    write_seqlock(&tk.seq);
    tk.nsec += delta_nsec;
    if (tk.nsec >= 1000000000UL) {
        tk.seconds++;
        tk.nsec -= 1000000000UL;
    }
    write_sequnlock(&tk.seq);
}
```

**Notes:**
- `write_seqlock()` marks start (seq becomes odd).
- `write_sequnlock()` marks end (seq becomes even).
- `smp_wmb()` inside macros ensures write ordering.

### 5.4 Reader (fast, lockless read)

```c
void tk_read(unsigned long *sec, unsigned long *nsec)
{
    unsigned int seq;

    do {
        seq = read_seqbegin(&tk.seq);
        *sec = tk.seconds;
        *nsec = tk.nsec;
    } while (read_seqretry(&tk.seq, seq));
}
```

**Notes:**
- Reader retries if a writer was active during the read.
- This pattern ensures readers observe a coherent pair `(seconds,nsec)`.

### 5.5 Integration with VDSO / user-space fast-paths

- Kernel provides values via VDSO so user-space can read time without syscalls.
- The VDSO code performs the same `read_seqbegin()` / `read_seqretry()` logic to ensure consistent snapshots.
- SeqLock here reduces syscall overhead and improves `gettimeofday()` performance.

---

## 6. Practical Kernel Module Example

```c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/seqlock.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *thread_th1;
static struct task_struct *thread_th2;
seqlock_t my_seqlock;
int global_var = 0;

static int read_callback_func(void *)
{
    unsigned int seq_no;
    int g_copy;
    while(!kthread_should_stop())
    {
        do
        {
            seq_no = read_seqbegin(&my_seqlock);
            g_copy = global_var;
        }while(read_seqretry(&my_seqlock, seq_no));
        pr_info("read values : %d\r\n", g_copy);
        ssleep(1);
    }
    return 0;
}

static int write_callback_func(void *)
{
    while(!kthread_should_stop())
    {
        write_seqlock(&my_seqlock);
        global_var++;
        write_sequnlock(&my_seqlock);
        ssleep(1);
    }
    return 0;
}

static int __init module_seqlock_init(void)
{
    pr_info("module seqlock init fun");
    seqlock_init(&my_seqlock);

    thread_th1 = kthread_run(write_callback_func, NULL, "thread_1");
    thread_th2 = kthread_run(read_callback_func, NULL, "thread_2");
    return 0;
}

static void __exit module_seqlock_exit(void)
{
    pr_info("module seqlock exit function");
    if(thread_th1) kthread_stop(thread_th1);
    if(thread_th2) kthread_stop(thread_th2);
}

module_init(module_seqlock_init);
module_exit(module_seqlock_exit);

MODULE_DESCRIPTION("Linux kernel seqlock tutorial");
MODULE_VERSION("1.0.1");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
```

---

## 7. How the Sequence Counter Changes (Even ↔ Odd)

### 7.1 Internals of `write_seqlock()`
```c
#define write_seqlock(lock) do {     spin_lock(&(lock)->lock);     (lock)->seqcount.sequence++;     smp_wmb(); } while (0)
```
- Sequence increments → **even → odd**
- Odd → means writer active

### 7.2 Internals of `write_sequnlock()`
```c
#define write_sequnlock(lock) do {     smp_wmb();     (lock)->seqcount.sequence++;     spin_unlock(&(lock)->lock); } while (0)
```
- Sequence increments again → **odd → even**
- Even → means writer done, data stable

---

## 8. Sequence Example

| Step | Action | Seq Number | Meaning |
|------|---------|-------------|----------|
| Init | 0 | No writer active |
| Writer starts | 1 | Writing (odd) |
| Writer ends | 2 | Done (even) |
| Next write starts | 3 | Writing (odd) |
| Next write ends | 4 | Done (even) |

---

## 9. Reader Validation Logic

Reader uses:
```c
seq_no = read_seqbegin(&lock);
read data;
while (read_seqretry(&lock, seq_no));
```

If a writer modified during read → sequence changes → retry.

---

## 10. Expected Output

```
module seqlock init fun
read values : 0
read values : 1
read values : 2
read values : 3
...
```

Readers always get **consistent data snapshots**.

---

## 11. Summary

| Concept | Description |
|----------|--------------|
| **Even sequence** | No writer active |
| **Odd sequence** | Writer active |
| **Reader check** | Retries if sequence changes |
| **Best Use Case** | Read-mostly workloads |
| **Example Usage** | Timekeeping, stats, counters |

---

### 🧾 Author
**Mahendra Sondagar**  
IoT solution Architect  

---
