#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <asm/uaccess.h>
#include "ap7000.h"
#include "switches.h"

#define DEV_NAME           "stk1000io"
#define DEV_PATHNAME       "stk1000io"
#define PIO_REQ_START_ADDR AVR32_PIOB_ADDRESS
#define PIO_REQ_CNT        100

static ssize_t switches_dev_read(struct file *, char *, size_t, loff_t *);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = switches_dev_read,
};

dev_t dev_num;
static struct cdev cdev;

static int __init init(void)
{
    if (alloc_chrdev_region(&dev_num, 0, 1, "stk1000io") < 0) {
        printk(KERN_ALERT "alloc_chrdev_region failed");
        return -1;
    }

    if (request_region(PIO_REQ_START_ADDR, PIO_REQ_CNT, "stk1000io") == NULL) {
        printk(KERN_ALERT "request_region failed");
        return -1;
    }

    switches_init();

    cdev_init(&cdev, &fops);
    cdev.owner = THIS_MODULE;
    cdev_add(&cdev, dev_num, 1);

    printk("%s driver up. Driver number: %d %d\n", DEV_NAME,
           MAJOR(dev_num), MINOR(dev_num));

    return 0;
}

static void __exit exit_(void)
{
    cdev_del(&cdev);
    unregister_chrdev_region(dev_num, 1);
    release_region(PIO_REQ_START_ADDR, PIO_REQ_CNT);
}

volatile int z = 0;
static ssize_t switches_dev_read(struct file *filp,
                                 char *buffer,
                                 size_t length,
                                 loff_t * offset)
{
    for (int i = 0 ; i < length; ++i)
        *buffer++ = switches_read();
    return length;
}

/*
static ssize_t
dev_write(struct file *filp, const char *buf, size_t len, loff_t * off)
{
    const size_t num_to_read = 1;
    int num_not_read;

    char input[num_to_read];
    if (len < num_to_read) {
        printk(KERN_WARNING "Not enough data to read.");
        return 0;
    }

    if ((num_not_read = copy_from_user(input, buf, num_to_read)) > 0) {
        printk(KERN_WARNING "stk1000io::dev_write() - %d bytes *not* copied.\n");
        return num_to_read - num_not_read;
    }

    leds_set((unsigned int) input[0]);
    printk("%u\n", (unsigned int) input[0]);

    *off += num_to_read;

    return num_to_read;
}
*/

module_init(init);
module_exit(exit_);

MODULE_LICENSE("Dual BSD/GPL");
