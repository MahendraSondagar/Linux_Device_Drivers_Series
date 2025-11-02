
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

## 5. Sequence Example — Step by Step

This section shows an explicit step-by-step change of the sequence counter across reader and writer actions, using small, discrete events so you can trace exact values.

Initial state (after `seqlock_init()`):
- `sequence = 0` (even) → no writer active.

Writer A starts:
1. `write_seqlock()` increments sequence: `0 -> 1` (odd) → writer active.
2. Writer A updates protected data.
3. `write_sequnlock()` increments sequence: `1 -> 2` (even) → writer finished.

Reader R runs (between steps):
- Reads `seq = 2` (even), reads data, then re-reads `seq` and sees `2` → success.

Writer B starts:
1. `write_seqlock()` increments sequence: `2 -> 3` (odd).
2. Update data.
3. `write_sequnlock()` increments: `3 -> 4` (even).

Summary table:

| Event | Sequence before | Sequence after | Note |
|-------|------------------|----------------|------|
| Init | - | 0 | stable |
| Writer A start | 0 | 1 | odd → writer active |
| Writer A end | 1 | 2 | even → stable |
| Writer B start | 2 | 3 | odd |
| Writer B end | 3 | 4 | even |

---

## 6. What Readers Do With This Sequence

Reader pattern (your code uses this pattern):

```c
do {
    seq_no = read_seqbegin(&my_seqlock);
    g_copy = global_var;
} while (read_seqretry(&my_seqlock, seq_no));
```

Explanation:
- `read_seqbegin()` returns the current `sequence` (READ_ONCE) and executes a read memory barrier so subsequent reads see consistent memory.
- The reader copies shared data.
- `read_seqretry()` checks two things:
  1. Was the initial `sequence` odd? If yes → a writer was active when reader started → retry.
  2. Is the `sequence` value changed since the initial read? If yes → writer updated during read → retry.
- Only when the sequence was even at start *and* unchanged at end does the reader accept the data.

So readers never block writers — they **retry** if they detect a concurrent write.

---

## 7. Real-Time Example Using Your Code

Using the kernel module example from earlier:

```c
seqlock_t my_seqlock;
int global_var = 0;

/* writer thread */
write_seqlock(&my_seqlock);
global_var++;
write_sequnlock(&my_seqlock);

/* reader thread */
seq_no = read_seqbegin(&my_seqlock);
g_copy = global_var;
if (read_seqretry(&my_seqlock, seq_no)) { retry; }
```

Walkthrough (timeline):

1. Start: `global_var=0`, `seq=0`.
2. Writer increments: `seq=1`, `global_var=1`, `seq=2`.
3. Reader calls `read_seqbegin()` and sees `seq=2` → reads `global_var=1` → `read_seqretry()` returns false → print `1`.
4. Next writer run repeats, reader reads `2`, etc.

If reader started while writer was mid-update:
- Reader's `read_seqbegin()` returns odd `seq` or `read_seqretry()` finds change → reader retries until it sees an even unchanged `seq`.

This behavior guarantees readers see a consistent snapshot of `global_var` without acquiring locks.

---

## 8. Visual Summary

```
Time axis:
t0: seq=0 (even) - stable
t1: writer starts -> write_seqlock() -> seq=1 (odd)
t2: writer updates data (global_var++)
t3: writer ends -> write_sequnlock() -> seq=2 (even)
t4: reader reads: read_seqbegin() sees seq=2 -> read data -> read_seqretry() sees seq still 2 -> success
```

Key visual points:
- Odd sequence = writer active (reader must not trust data)
- Even sequence = stable snapshot (reader may trust if unchanged)
- Writers increment twice per update (start/end) — so sequence monotonically increases.

---

## 9. Key Takeaways

- **Even sequence** indicates no writer is active and data is potentially stable.
- **Odd sequence** indicates a writer is active; readers must retry.
- **Writers increment sequence twice** per update: once at start (even→odd), once at end (odd→even).
- **Readers are lockless**: they don't block writers; instead they detect concurrent writes and retry.
- **Best use cases**: timekeeping, counters, statistics — scenarios where reads are far more frequent than writes and writers can tolerate being retried.

---

## 10. Example — Timekeeping Example (Practical Use Case)

SeqLocks are heavily used in the kernel's **timekeeping** code because time values are read extremely frequently (user `gettimeofday()`, `clock_gettime()`) while updates are relatively rare and short. This section shows a practical, compact example that mirrors how the kernel keeps a coherent snapshot of time across readers and writers.

### 10.1 Problem Context

- A writer (timer interrupt or periodic kernel thread) updates `xtime`/`monotonic` fields.
- Many readers (user-space via VDSO or kernel callers) need a consistent snapshot to compute time.
- Blocking readers with spinlocks hurts performance; SeqLock allows lockless reads with validation.

### 10.2 Data Structures

```c
#include <linux/seqlock.h>

struct tk_clock {
    unsigned long seconds;
    unsigned long nsec;
    seqlock_t seq; /* protects seconds/nsec */
};

static struct tk_clock tk;
```

### 10.3 Writer (timer-like update)

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

### 10.4 Reader (fast, lockless read)

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

### 10.5 Integration with VDSO / user-space fast-paths

- Kernel provides values via VDSO so user-space can read time without syscalls.
- The VDSO code performs the same `read_seqbegin()` / `read_seqretry()` logic to ensure consistent snapshots.
- SeqLock here reduces syscall overhead and improves `gettimeofday()` performance.

---

## 11. Practical Kernel Module Example

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
MODULE_VERSION("1.0.2");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahendra Sondagar <mahendrasondagar08@gmail.com>");
```

---

## 12. How the Sequence Counter Changes (Even ↔ Odd)

### 12.1 Internals of `write_seqlock()`
```c
#define write_seqlock(lock) do {     spin_lock(&(lock)->lock);     (lock)->seqcount.sequence++;     smp_wmb(); } while (0)
```
- Sequence increments → **even → odd**
- Odd → means writer active

### 12.2 Internals of `write_sequnlock()`
```c
#define write_sequnlock(lock) do {     smp_wmb();     (lock)->seqcount.sequence++;     spin_unlock(&(lock)->lock); } while (0)
```
- Sequence increments again → **odd → even**
- Even → means writer done, data stable

---

## 13. Sequence Example (Short Table)

| Step | Action | Seq Number | Meaning |
|------|---------|-------------|----------|
| Init | 0 | No writer active |
| Writer starts | 1 | Writing (odd) |
| Writer ends | 2 | Done (even) |
| Next write starts | 3 | Writing (odd) |
| Next write ends | 4 | Done (even) |

---

## 14. Reader Validation Logic

Reader uses:
```c
seq_no = read_seqbegin(&lock);
read data;
while (read_seqretry(&lock, seq_no));
```

If a writer modified during read → sequence changes → retry.

---

## 15. Expected Output

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

## 16. Summary

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
