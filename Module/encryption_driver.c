/*
 * Caesar Cipher Device Driver
 *
 * This device driver provides a simple interface for encrypting and decrypting
 * strings using the Caesar Cipher algorithm. It demonstrates the use of open,
 * release, read, write, and ioctl functions in a character device driver.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "caesardev"
#define MAX_BUFFER_SIZE 1024

static dev_t dev_num;
static struct cdev c_dev;
static struct class *cl;
static char *kernel_buffer;
static int buffer_length;
static int shift_key;

/*
 * Caesar Cipher Encryption/Decryption Function
 * This function performs Caesar Cipher encryption/decryption on the provided buffer.
 */
static void caesar_cipher(char *buffer, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        if (buffer[i] >= 'A' && buffer[i] <= 'Z')
        {
            buffer[i] = 'A' + (buffer[i] - 'A' + shift_key) % 26;
        }
        else if (buffer[i] >= 'a' && buffer[i] <= 'z')
        {
            buffer[i] = 'a' + (buffer[i] - 'a' + shift_key) % 26;
        }
    }
}

/*
 * Open Function
 * Called when the device file is opened.
 */
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device file opened\n");
    return 0;
}

/*
 * Release Function
 * Called when the device file is closed.
 */
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device file closed\n");
    return 0;
}

/*
 * Read Function
 * Called when data is read from the device file.
 */
static ssize_t dev_read(struct file *file, char __user *user_buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;
    if (*offset >= buffer_length)
    {
        return 0;
    }
    bytes_read = buffer_length - *offset;
    if (copy_to_user(user_buffer, kernel_buffer + *offset, bytes_read))
    {
        return -EFAULT;
    }
    *offset += bytes_read;
    return bytes_read;
}

/*
 * Write Function
 * Called when data is written to the device file.
 */
static ssize_t dev_write(struct file *file, const char __user *user_buffer, size_t length, loff_t *offset)
{
    int bytes_written = 0;
    if (*offset >= buffer_length)
    {
        return -ENOMEM;
    }
    if (copy_from_user(kernel_buffer + *offset, user_buffer, length))
    {
        return -EFAULT;
    }
    bytes_written = length;
    *offset += bytes_written;
    buffer_length = max(buffer_length, (int)(*offset));
    encrypt_decrypt(kernel_buffer, buffer_length);
    return bytes_written;
}

/*
 * IOCTL Function
 * Called when an IOCTL command is issued to the device file.
 */
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
    case 1:
        shift_key = arg % 26;
        printk(KERN_INFO "Shift key set to %d\n", shift_key);
        caesar_cipher(kernel_buffer, buffer_length);
        printk(KERN_INFO "Buffer encrypted/decrypted\n");
        break;
    default:
        printk(KERN_INFO "Invalid IOCTL command\n");
        return -EINVAL;
    }
    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
};

/*
 * Module Init Function
 * Called when the module is loaded.
 */
static int __init dev_init(void)
{
    int ret;
    kernel_buffer = kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
    if (!kernel_buffer)
    {
        return -ENOMEM;
    }
    buffer_length = 0;
    shift_key = 0;
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ERR "Failed to allocate device number\n");
        goto failed_alloc;
    }
    cdev_init(&c_dev, &fops);
    ret = cdev_add(&c_dev, dev_num, 1);
    if (ret < 0)
    {
        printk(KERN_ERR "Failed to add cdev\n");
        goto failed_cdev;
    }
    cl = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(cl))
    {
        printk(KERN_ERR "Failed to create class\n");
        goto failed_class;
    }
    device_create(cl, NULL, dev_num, NULL, DEVICE_NAME);
    printk(KERN_INFO "Device driver loaded\n");
    return 0;

failed_class:
    cdev_del(&c_dev);
failed_cdev:
    unregister_chrdev_region(dev_num, 1);
failed_alloc:
    kfree(kernel_buffer);
    return ret;
}

/*
 * Module Exit Function
 * Called when the module is unloaded.
 */
static void __exit dev_exit(void)
{
    device_destroy(cl, dev_num);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev_num, 1);
    kfree(kernel_buffer);
    printk(KERN_INFO "Device driver unloaded\n");
}

module_init(dev_init);
module_exit(dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Doe");
MODULE_DESCRIPTION("Caesar Cipher Device Driver");