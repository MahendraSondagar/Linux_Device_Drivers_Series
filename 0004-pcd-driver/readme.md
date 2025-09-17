# Linux Kernel Helper APIs and Concepts

This document provides a detailed explanation of key Linux kernel APIs
and concepts such as `copy_from_user`, `copy_to_user`, `IS_ERR`, and the
`__user` macro. These are fundamental when writing Linux device drivers
that interact with user space.

------------------------------------------------------------------------

## 1. Kernel ↔ User Memory Access APIs

When writing kernel modules, we often need to transfer data between
**user space** and **kernel space** safely. Direct access is not allowed
for safety and security reasons, so Linux provides APIs like
`copy_from_user` and `copy_to_user`.

### 1.1 `copy_from_user`

``` c
unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
```

-   **Purpose**: Copies data from **user space** buffer into **kernel
    space** buffer.
-   **Arguments**:
    -   `to`: Destination address in **kernel space**.
    -   `from`: Source address in **user space** (`__user` marked
        pointer).
    -   `n`: Number of bytes to copy.
-   **Return value**:
    -   Returns `0` if all `n` bytes were copied successfully.
    -   Returns the number of bytes **not copied** if an error occurred
        (e.g., invalid user pointer).

✅ **Example Usage**

``` c
char kbuf[100];
if (copy_from_user(kbuf, ubuf, sizeof(kbuf))) {
    pr_err("Failed to copy data from user
");
    return -EFAULT;
}
```

------------------------------------------------------------------------

### 1.2 `copy_to_user`

``` c
unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
```

-   **Purpose**: Copies data from **kernel space** buffer into **user
    space** buffer.
-   **Arguments**:
    -   `to`: Destination address in **user space** (`__user` marked
        pointer).
    -   `from`: Source address in **kernel space**.
    -   `n`: Number of bytes to copy.
-   **Return value**:
    -   Returns `0` if all bytes were copied successfully.
    -   Returns the number of bytes **not copied** if an error occurred.

✅ **Example Usage**

``` c
char kbuf[100] = "Hello from kernel!";
if (copy_to_user(ubuf, kbuf, strlen(kbuf))) {
    pr_err("Failed to copy data to user
");
    return -EFAULT;
}
```

------------------------------------------------------------------------

## 2. Error Handling Macros

### 2.1 `IS_ERR`

``` c
IS_ERR(const void *ptr);
```

-   **Purpose**: Tests whether a returned pointer is actually an **error
    code** encoded as a pointer.
-   **Arguments**:
    -   `ptr`: Pointer returned by some kernel API (e.g., `kmalloc`,
        `filp_open`).
-   **Return value**:
    -   Returns **true (non-zero)** if `ptr` contains an error code.
    -   Returns **false (zero)** if `ptr` is a valid pointer.

✅ **Example Usage**

``` c
struct file *filp;
filp = filp_open("/etc/passwd", O_RDONLY, 0);
if (IS_ERR(filp)) {
    pr_err("Failed to open file, error: %ld
", PTR_ERR(filp));
    return PTR_ERR(filp);
}
```

Here, `PTR_ERR(filp)` extracts the actual error code (`-ENOENT`,
`-EACCES`, etc.) from the encoded pointer.

------------------------------------------------------------------------

## 3. The `__user` Annotation

-   The `__user` macro is used in function declarations to **mark
    pointers that belong to user space**.
-   It is **not a runtime instruction**; instead, it is a **hint for
    static analysis tools** like **Sparse** to detect unsafe memory
    accesses.
-   Helps kernel developers ensure that they don't directly dereference
    user pointers, but instead use safe functions like `copy_from_user`
    / `copy_to_user`.

✅ **Example**

``` c
ssize_t pcd_write(struct file *filp, const char __user *buff,
                  size_t count, loff_t *f_pos)
{
    char kbuf[128];
    if (copy_from_user(kbuf, buff, count))
        return -EFAULT;

    pr_info("Received from user: %s
", kbuf);
    return count;
}
```

Here, `buff` is marked as `__user`, meaning it comes from user space.

------------------------------------------------------------------------

## 4. Summary

  -------------------------------------------------------------------------
  API / Macro        Purpose                                  Return Value
  ------------------ ---------------------------------------- -------------
  `copy_from_user`   Copy data **user → kernel**              Bytes not
                                                              copied (0 =
                                                              success)

  `copy_to_user`     Copy data **kernel → user**              Bytes not
                                                              copied (0 =
                                                              success)

  `IS_ERR`           Check if a pointer is an encoded error   Boolean

  `__user`           Marks a user-space pointer (for safety)  N/A
  -------------------------------------------------------------------------

------------------------------------------------------------------------

## 5. Key Takeaways

-   Always use `copy_from_user` and `copy_to_user` instead of direct
    pointer dereference.
-   Always check return values to prevent crashes or memory corruption.
-   Use `IS_ERR` and `PTR_ERR` for safe error handling with kernel
    pointers.
-   Respect the `__user` annotation to ensure code safety and
    correctness.

------------------------------------------------------------------------

📌 This document serves as a quick reference for Linux device driver
developers working with **user ↔ kernel interactions**.
