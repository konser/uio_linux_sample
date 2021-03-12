#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/delay.h>

#define FIRST_MINOR 1
#define MINOR_CNT 2

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
DEFINE_SPINLOCK(my_lock);

static int my_open(struct inode *i, struct file *f)
{
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    return 0;
}

static char c = 'A';
static struct semaphore my_sem;

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    // Acquire the Semaphore
    spin_lock(&my_lock);
	ssleep(2);
    return 0;
}
static ssize_t my_write(struct file *f, const char __user *buf, size_t len,
        loff_t *off)
{
    // Release the semaphore
	spin_unlock(&my_lock);
    if (copy_from_user(&c, buf + len - 1, 1))
    {
        return -EFAULT;
    }
    return len;
}

static struct file_operations driver_fops =
{
 .owner = THIS_MODULE,
 .open = my_open,
 .release = my_close,
 .read = my_read,
 .write = my_write
};

static int __init sem_init(void)
{
    int ret;
    struct device *dev_ret;

    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "my_sem")) < 0)
    {
        return ret;
    }

    cdev_init(&c_dev, &driver_fops);

    if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        unregister_chrdev_region(dev, MINOR_CNT);
        return ret;
    }

    if (IS_ERR(cl = class_create(THIS_MODULE, "chardev")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }

    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "mysem%d", FIRST_MINOR)))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }

    sema_init(&my_sem, 0);
    return 0;
}

static void __exit sem_exit(void)
{
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
}

module_init(sem_init);
module_exit(sem_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pradeep");
MODULE_DESCRIPTION("Binary Semaphore Demonstration");
