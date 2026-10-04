/*
 * filelogger.c - A simple Linux character device driver (one-way logger)
 *
 * Project : Linux File Management Utility
 * Purpose : Acts as a logging device. The C++ client writes short
 *           messages into /dev/filelogger whenever it performs a file
 *           operation (create, delete, rename, etc.), and this driver
 *           records those messages into the kernel log (dmesg).
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DEVICE_NAME   "filelogger"
#define CLASS_NAME    "filelogger_class"
#define BUFFER_SIZE   256

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student Name");
MODULE_DESCRIPTION("A simple write-only character device driver that logs file operations");
MODULE_VERSION("1.0");

static int            major_number;
static struct class  *filelogger_class  = NULL;
static struct device *filelogger_device = NULL;

static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "filelogger: device opened\n");
    return 0;
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    char kernel_buffer[BUFFER_SIZE];
    size_t bytes_to_copy = len;

    if (bytes_to_copy >= BUFFER_SIZE) {
        bytes_to_copy = BUFFER_SIZE - 1;
    }

    memset(kernel_buffer, 0, BUFFER_SIZE);

    if (copy_from_user(kernel_buffer, buffer, bytes_to_copy) != 0) {
        printk(KERN_ALERT "filelogger: failed to receive data from user\n");
        return -EFAULT;
    }

    printk(KERN_INFO "filelogger: %s\n", kernel_buffer);

    return bytes_to_copy;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "filelogger: device closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .write   = dev_write,
    .release = dev_release,
};

static int __init filelogger_init(void)
{
    printk(KERN_INFO "filelogger: initializing\n");

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "filelogger: failed to register a major number\n");
        return major_number;
    }

    filelogger_class = class_create(CLASS_NAME);
    if (IS_ERR(filelogger_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(filelogger_class);
    }

    filelogger_device = device_create(filelogger_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(filelogger_device)) {
        class_destroy(filelogger_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(filelogger_device);
    }

    printk(KERN_INFO "filelogger: /dev/%s created successfully\n", DEVICE_NAME);
    return 0;
}

static void __exit filelogger_exit(void)
{
    device_destroy(filelogger_class, MKDEV(major_number, 0));
    class_unregister(filelogger_class);
    class_destroy(filelogger_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "filelogger: unloaded, goodbye\n");
}

module_init(filelogger_init);
module_exit(filelogger_exit);
