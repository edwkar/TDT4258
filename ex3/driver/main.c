#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "ap7000.h"
#include "switches.h"
#include "leds.h"


#define SWITCHES_DEV_NAME  "stk1000io"
#define PIO_REQ_START_ADDR AVR32_PIOB_ADDRESS
#define PIO_REQ_CNT        100

#define NUM_DEVICES        2



static ssize_t switches_dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t leds_dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations switches_fops = {
    .owner = THIS_MODULE,
    .read = switches_dev_read,
};

static struct file_operations leds_fops = {
    .owner = THIS_MODULE,
    .write = leds_dev_write
};

static dev_t dev_num;
static struct cdev switches_cdev;
static struct cdev leds_cdev;

static int __init init_switches_dev(void);
static int __init init_leds_dev(void);

static int __init init(void)
{
    if (alloc_chrdev_region(&dev_num, 0, NUM_DEVICES, "stk1000io") < 0) {
        printk(KERN_ALERT "alloc_chrdev_region failed");
        return -1;
    }

    if (request_region(PIO_REQ_START_ADDR, PIO_REQ_CNT, "stk1000io") == NULL) {
        printk(KERN_ALERT "request_region failed");
        return -1;
    }

    /* Initialise switches and register switches device.
     */
    if (init_switches_dev() < 0)
        return -1;

    /* Initialise LEDs and register LEDs device.
     */
    if (init_leds_dev() < 0)
        return -1;

    printk("STK1000 IO driver up. Driver number: %d %d\n",
           MAJOR(dev_num), MINOR(dev_num));

    return 0;
}

static int __init init_switches_dev(void)
{
    switches_init();

    cdev_init(&switches_cdev, &switches_fops);
    switches_cdev.owner = THIS_MODULE;
    if (cdev_add(&switches_cdev, dev_num, 1) < 0) {
        printk(KERN_ALERT "*** cdev_add for switches device failed ***\n");
        return -1;
    }

    return 0;
}

static int __init init_leds_dev(void)
{
    dev_t leds_dev_num;

    leds_init();

    leds_dev_num = MKDEV(MAJOR(dev_num), MINOR(dev_num)+1);

    cdev_init(&leds_cdev, &leds_fops);
    leds_cdev.owner = THIS_MODULE;
    if (cdev_add(&leds_cdev, leds_dev_num, 1) < 0) {
        printk(KERN_ALERT "*** cdev_add for leds device failed ***\n");
        return -1;
    }

    return 0;
}

static void __exit exit_(void)
{
    cdev_del(&switches_cdev);
    cdev_del(&leds_cdev);
    unregister_chrdev_region(dev_num, NUM_DEVICES);
    release_region(PIO_REQ_START_ADDR, PIO_REQ_CNT);
}

static ssize_t switches_dev_read(struct file *filp,
                                 char *buffer,
                                 size_t length,
                                 loff_t * offset)
{
    for (int i = 0 ; i < length; ++i)
        *buffer++ = switches_read();
    return length;
}

static ssize_t leds_dev_write(struct file *filp,
                              const char *buffer,
                              size_t length,
                              loff_t * offset)
{
    unsigned char v[1];

    if (length == 0)
        return 0;

    copy_from_user(v, buffer, 1);
    leds_set((unsigned int) v[0]);

    return 1;
}

module_init(init);
module_exit(exit_);

MODULE_LICENSE("Dual BSD/GPL");
