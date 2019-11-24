#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function
#include <linux/thread_info.h>    // current macro
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/version.h>

#define  DEVICE_NAME "rtcloop0"
#define  CLASS_NAME  "rtcloop"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yue Liang");
MODULE_DESCRIPTION("A simple Linux char driver for rtc loop");
MODULE_VERSION("0.1");

static int    devId;
static struct class*  rtcloopClass  = NULL;
static struct device* rtcloopDevice = NULL;

static DEFINE_SEMAPHORE(mtx_sem);
#define MUTEX_LOCK    down(&mtx_sem);
#define MUTEX_UNLOCK  up(&mtx_sem);

static wait_queue_head_t wait_queue;
static struct timer_list timer;

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static long    dev_ioctl(struct file *, unsigned int, unsigned long);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .compat_ioctl = dev_ioctl,
   .unlocked_ioctl = dev_ioctl,
   .release = dev_release,
};

static void callback(unsigned long data) {
    printk(KERN_INFO "my rtc call %ld\n", jiffies);
    timer.expires = jiffies + 2*HZ;
    add_timer(&timer);
}

static int __init loop_init(void) {

    init_waitqueue_head(&wait_queue);

    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    devId = register_chrdev(0, DEVICE_NAME, &fops);
    if (devId<0){
        printk(KERN_ALERT "rtcloop failed to register a major number\n");
        return devId;
    }

    // Register the device class
    rtcloopClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(rtcloopClass)){                // Check for error and clean up if there is
        unregister_chrdev(devId, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(rtcloopClass);          // Correct way to return an error on a pointer
    }

    // Register the device driver
    rtcloopDevice = device_create(rtcloopClass, NULL, MKDEV(devId, 0), NULL, DEVICE_NAME);
    if (IS_ERR(rtcloopDevice)){               // Clean up if there is an error
        class_destroy(rtcloopClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(devId, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(rtcloopDevice);
    }


    init_timer(&timer);
    timer.expires = jiffies +2*HZ;
    timer.data = (unsigned long)rtcloopDevice;
    timer.function = callback;
    add_timer(&timer);

    return 0;
}

static void __exit loop_exit(void) {
    del_timer_sync(&timer);
    device_destroy(rtcloopClass, MKDEV(devId, 0));     // remove the device
    class_unregister(rtcloopClass);                    // unregister the device class
    class_destroy(rtcloopClass);                       // remove the device class
    unregister_chrdev(devId, DEVICE_NAME);             // unregister the major number
    printk(KERN_INFO "rtcloop: Goodbye from the Kernel!\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static long dev_ioctl(struct file *filp, unsigned int need, unsigned long identify) {
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {//get resource
    return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {//release resource
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    return 0;
}

module_init(loop_init);
module_exit(loop_exit);
