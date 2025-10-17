
## Linux Kernel Dynamic Memory Allocation Tutorial

---

##  Introduction

In **user space**, we typically use functions like:
```c
malloc(), calloc(), realloc(), free()
```
But in the **Linux kernel**, these are **not available** because:
- Kernel doesn’t use libc (no standard C library).
- Kernel memory management must be *fast*, *deterministic*, and *non-swappable*.
- Memory allocations must respect interrupt and atomic context constraints.

Instead, the kernel provides specialized APIs for **dynamic memory allocation**:
- `kmalloc()`
- `kzalloc()`
- `kcalloc()`
- `krealloc()`
- `kfree()`

---

##  `kmalloc()`

```c
void *kmalloc(size_t size, gfp_t flags);
```
Allocates **physically contiguous** memory.

| Parameter | Description |
|------------|-------------|
| `size` | Number of bytes to allocate |
| `flags` | Memory allocation flags (see GFP flags section) |

- Returns pointer to allocated memory or `NULL` on failure.
- Memory is **uninitialized**.

Example:
```c
int *data = kmalloc(100 * sizeof(int), GFP_KERNEL);
if (!data)
    return -ENOMEM;
```

---

##  `kzalloc()`

```c
void *kzalloc(size_t size, gfp_t flags);
```
Same as `kmalloc()`, but initializes memory to **zero**.

Example:
```c
struct my_struct *ptr = kzalloc(sizeof(*ptr), GFP_KERNEL);
if (!ptr)
    return -ENOMEM;
```

---

##  `kcalloc()`

```c
void *kcalloc(size_t n, size_t size, gfp_t flags);
```
Allocates memory for an **array** and zeroes it out. It prevents integer overflow.

Example:
```c
int *arr = kcalloc(50, sizeof(int), GFP_KERNEL);
if (!arr)
    return -ENOMEM;
```

---

##  `krealloc()`

```c
void *krealloc(const void *p, size_t new_size, gfp_t flags);
```
Resizes memory previously allocated.

Behavior:
- `p == NULL` → acts like `kmalloc()`
- `new_size == 0` → frees memory and returns `NULL`
- Copies old data and frees old memory

Example:
```c
char *buf = kmalloc(128, GFP_KERNEL);
buf = krealloc(buf, 256, GFP_KERNEL);
if (!buf)
    return -ENOMEM;
```

---

##  `kfree()`

```c
void kfree(const void *p);
```
Frees memory allocated by `kmalloc()`, `kzalloc()`, `kcalloc()`, or `krealloc()`.

Example:
```c
kfree(buf);
```

---

#  Understanding GFP Flags in Linux Kernel

The **GFP flag** tells the kernel **how** and **from where** to allocate memory.

GFP = **Get Free Page** flags.  
They control:
- Whether the allocator can **sleep (block)**.
- Whether it can **trigger I/O or filesystem operations**.
- Which **memory zone** it can use.

---

##  The Core Concept — "Can the code sleep?"

The **biggest difference** between GFP flags is:

> ❓ Can the kernel put the current process to sleep while waiting for free memory?

- ✅ **If yes → Use `GFP_KERNEL`**
- ❌ **If no → Use `GFP_ATOMIC`**

---

##  1. `GFP_KERNEL`
Normal kernel memory allocation. Can sleep and perform I/O.

Example:
```c
void *buf = kmalloc(1024, GFP_KERNEL);
```

---

##  2. `GFP_ATOMIC`
Used in interrupt or atomic context where the code **cannot sleep**.

Example:
```c
irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    char *data = kmalloc(256, GFP_ATOMIC);
    if (!data)
        printk(KERN_ERR "Allocation failed in IRQ!\n");
    return IRQ_HANDLED;
}
```

---

##  3. `GFP_NOWAIT`
Do not wait at all — return immediately if memory is not available.

Example:
```c
void *ptr = kmalloc(512, GFP_NOWAIT);
if (!ptr)
    printk(KERN_WARNING "Could not allocate memory\n");
```

---

##  4. `GFP_NOIO`
Do not initiate any I/O operations while allocating memory.

Used in block I/O paths to avoid recursive calls.

Example:
```c
void *ptr = kmalloc(1024, GFP_NOIO);
```

---

##  5. `GFP_NOFS`
Avoid triggering filesystem operations during allocation.

Used inside filesystem code.

Example:
```c
void *ptr = kmalloc(2048, GFP_NOFS);
```

---

##  6. `GFP_DMA`
Allocate memory from DMA zone for devices requiring low memory addresses.

Example:
```c
void *dma_buf = kmalloc(4096, GFP_KERNEL | GFP_DMA);
```

---

##  7. `GFP_HIGHUSER`
Allocate memory from high memory, typically for user-space pages.

---

## 🧠 Quick Summary

| Flag | Can Sleep? | Context | Typical Use |
|------|-------------|----------|-------------|
| `GFP_KERNEL` | ✅ | Process | Default for normal allocations |
| `GFP_ATOMIC` | ❌ | Interrupt | Use in IRQ/atomic context |
| `GFP_NOWAIT` | ❌ | Any | Non-blocking try |
| `GFP_NOIO` | ✅ | Block I/O | Block drivers |
| `GFP_NOFS` | ✅ | Filesystem | FS operations |
| `GFP_DMA` | ✅ | DMA | Legacy DMA devices |
| `GFP_HIGHUSER` | ✅ | User memory | Page cache |

---

##  Tip for Remembering

> **If you can sleep — use `GFP_KERNEL`.**  
> **If you can’t sleep — use `GFP_ATOMIC`.**

---

##  Example Kernel Module

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>

static int __init mem_alloc_init(void)
{
    int *p1, *p2, *p3;

    printk(KERN_INFO "Memory Allocation Demo Module Loaded\n");

    p1 = kmalloc(100 * sizeof(int), GFP_KERNEL);
    p2 = kzalloc(50 * sizeof(int), GFP_KERNEL);
    p3 = kcalloc(25, sizeof(int), GFP_KERNEL);
    p1 = krealloc(p1, 200 * sizeof(int), GFP_KERNEL);

    kfree(p1);
    kfree(p2);
    kfree(p3);

    return 0;
}

static void __exit mem_alloc_exit(void)
{
    printk(KERN_INFO "Memory Allocation Demo Module Unloaded\n");
}

module_init(mem_alloc_init);
module_exit(mem_alloc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Payal Sondagar");
MODULE_DESCRIPTION("Kernel Dynamic Memory Allocation Tutorial");
```

---

##  Summary Table

| Function | Zero Init | Overflow Safe | Resizable | Typical Use |
|-----------|------------|----------------|------------|--------------|
| `kmalloc()` | ❌ | ❌ | ❌ | Basic allocation |
| `kzalloc()` | ✅ | ❌ | ❌ | Zeroed struct |
| `kcalloc()` | ✅ | ✅ | ❌ | Array allocation |
| `krealloc()` | ❌ | ✅ | ✅ | Resize buffer |
| `kfree()` | — | — | ✅ | Free memory |

---

###  Final Tip

> - **If you can sleep, use `GFP_KERNEL`.**  
> - **If you can’t sleep, use `GFP_ATOMIC`.**

---

## Author: MahendraSondagar <mahendrasondagar08@gmail.com>


