
#Linux Kernel Dynamic Memory Allocation Tutorial

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

##  GFP (Get Free Page) Flags

These flags tell the kernel **how** to allocate memory.

| Flag | Meaning |
|------|----------|
| `GFP_KERNEL` | Normal allocation, may sleep |
| `GFP_ATOMIC` | Non-sleeping allocation (interrupt context) |
| `GFP_NOWAIT` | Don’t wait at all |
| `GFP_NOIO` | Don’t initiate I/O |
| `GFP_NOFS` | Don’t initiate filesystem ops |
| `GFP_DMA` | Allocate memory for DMA (low memory zone) |
| `GFP_HIGHUSER` | Allocate high memory |

###  Simple Rule
> If you can sleep — use `GFP_KERNEL`  
> If you can’t sleep — use `GFP_ATOMIC`

---

##  GFP Flags Explained with Examples

### `GFP_KERNEL`
Used in normal process context (can sleep).
```c
void *buf = kmalloc(1024, GFP_KERNEL);
```

### `GFP_ATOMIC`
Used in interrupt or atomic context (cannot sleep).
```c
irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    char *data = kmalloc(256, GFP_ATOMIC);
    if (!data)
        printk(KERN_ERR "Failed in IRQ!
");
    return IRQ_HANDLED;
}
```

### `GFP_NOIO`
Used in block drivers to avoid recursive I/O.
```c
void *ptr = kmalloc(1024, GFP_NOIO);
```

### `GFP_NOFS`
Used in filesystem code to avoid recursion.
```c
void *ptr = kmalloc(2048, GFP_NOFS);
```

### `GFP_DMA`
Used for legacy DMA hardware.
```c
void *dma_buf = kmalloc(4096, GFP_KERNEL | GFP_DMA);
```

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

##  Summary of GFP Flags

| Flag | Can Sleep | Context | Use Case |
|------|-------------|----------|-----------|
| `GFP_KERNEL` | ✅ | Process | Default safe flag |
| `GFP_ATOMIC` | ❌ | Interrupt | IRQ / Spinlocks |
| `GFP_NOIO` | ✅ | Block I/O | Block drivers |
| `GFP_NOFS` | ✅ | Filesystem | FS code |
| `GFP_DMA` | ✅ | DMA | Legacy DMA devices |
| `GFP_HIGHUSER` | ✅ | User space | Page cache |

---

###  Tip
> - **If you can sleep, use `GFP_KERNEL`.** 
> - **If you can’t sleep, use `GFP_ATOMIC`.**

---

## Author : Mahendra Sondagar <mahendrasondagar08@gmail.com>
##  End of Tutorial

