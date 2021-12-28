/*
 * Based upon the RPi example by Stefan Wendler (devnull@kaltpost.de)
 * from:
 *   https://github.com/wendlers/rpi-kmod-samples
 */

#include <linux/module.h>    
#include <linux/gpio.h>

#define LED    14

static struct timer_list blink_timer;
static int led_status = 0;

static void blink_timer_func(struct timer_list *unused)
{
    gpio_set_value(LED, led_status); 
    
    led_status = !led_status;

    blink_timer.expires = jiffies + (1*HZ);
    add_timer(&blink_timer);
}

int init_module(void)
{
    int ret = 0;

    // register, turn off 
    ret = gpio_request_one(LED, GPIOF_OUT_INIT_LOW, "led1");

    if (ret) {
        printk(KERN_ERR "Unable to request GPIOs: %d\n", ret);
        return ret;
    }

    timer_setup(&blink_timer, blink_timer_func, 0);

    led_status = 0;
    blink_timer.expires = jiffies + (1*HZ);
    add_timer(&blink_timer);

    return ret;
}

void cleanup_module(void)
{
    del_timer_sync(&blink_timer);

    // turn LED off
    gpio_set_value(LED, 0); 
    
    // unregister GPIO 
    gpio_free(LED);
}

MODULE_LICENSE("GPL");
