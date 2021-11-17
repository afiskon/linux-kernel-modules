#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/irq.h>

#define DEVICE_NAME "chardev"

static int sensor_value = 0;
static DEFINE_MUTEX(sensor_value_mtx);

static int major;
static struct class *cls;

typedef struct ChardevPrivateData {
    char buff[32];
    int cnt;
} ChardevPrivateData;

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *,
                           size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *,
                            size_t, loff_t *);

static struct file_operations chardev_fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
};

int init_module(void) {
    struct device* dev;
    major = register_chrdev(0, DEVICE_NAME, &chardev_fops);

    if(major < 0) {
        pr_alert("register_chrdev() failed: %d\n", major);
        return -EINVAL;
    }

    pr_info("major = %d\n", major);

    cls = class_create(THIS_MODULE, DEVICE_NAME);
    if(IS_ERR(cls)) {
        pr_alert("class_create() failed: %ld\n", PTR_ERR(cls));
        return -EINVAL;
    }

    dev = device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if(IS_ERR(dev)) {
        pr_alert("device_create() failed: %ld\n", PTR_ERR(dev));
        return -EINVAL;
    }

    pr_info("/dev/%s created\n", DEVICE_NAME);
    return 0;
}

void cleanup_module(void) {
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file) {
    ChardevPrivateData* pd;
    int val;

    if(!try_module_get(THIS_MODULE)) {
        pr_alert("try_module_get() failed\n");
        return -EINVAL;
    }

    pd = kmalloc(sizeof(ChardevPrivateData), GFP_KERNEL);
    if(pd == NULL) {
        pr_alert("kmalloc() failed\n");
        module_put(THIS_MODULE);
        return -EINVAL;
    }

    mutex_lock(&sensor_value_mtx);
    val = sensor_value;
    sensor_value++;
    mutex_unlock(&sensor_value_mtx);

    sprintf(pd->buff, "Dummy sensor value: %d\n", val);
    pd->cnt = 0;
    file->private_data = pd;
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    kfree(file->private_data);
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *file,
                           char __user *buffer,
                           size_t length,
                           loff_t *offset) {
    int bytes_read = 0;
    ChardevPrivateData* pd = file->private_data;

    while(length && (pd->buff[pd->cnt] != '\0')) {
        if(put_user(pd->buff[pd->cnt], buffer++) != 0)
            return -EINVAL;
        pd->cnt++;
        bytes_read++;
        length--;
    }

    return bytes_read;
}

static ssize_t device_write(struct file *filp,
                            const char __user *buff,
                            size_t len,
                            loff_t *off) {
    pr_alert("device_write() is not implemented\n");
    return -EINVAL;
}

MODULE_LICENSE("GPL");
