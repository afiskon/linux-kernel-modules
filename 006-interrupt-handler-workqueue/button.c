#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/module.h>

static int button_irq = -1;

static struct gpio buttons[] = {
    { 14 /* pin number */, GPIOF_IN, "BUTTON1" },
};

static void bottomhalf_work_fn(struct work_struct *work) {
    pr_info("Bottom half work starts\n");
    msleep(500);
    pr_info("Bottom half work ends\n");
}

DECLARE_WORK(buttonwork, bottomhalf_work_fn);

static irqreturn_t button_isr(int irq, void *data) {
    if (irq == button_irq) {
        schedule_work(&buttonwork);
    }

    return IRQ_HANDLED;
}

int init_module(void) {
    int ret;
    pr_info("%s\n", __func__);

    ret = gpio_request_array(buttons, ARRAY_SIZE(buttons));
    if (ret) {
        pr_err("gpio_request_array() failed: %d\n", ret);
        return ret;
    }

    ret = gpio_to_irq(buttons[0].gpio);
    if (ret < 0) {
        pr_err("gpio_to_irq() failed: %d\n", ret);
        gpio_free_array(buttons, ARRAY_SIZE(buttons));
        return ret;
    }

    button_irq = ret;
    ret = request_irq(button_irq, button_isr,
                      IRQF_TRIGGER_RISING,
                      "gpiomod#button1", NULL);
    if (ret) {
        pr_err("request_irq() failed: %d\n", ret);
        gpio_free_array(buttons, ARRAY_SIZE(buttons));
        return ret;
    }

    return 0;
}

void cleanup_module(void) {
    pr_info("%s\n", __func__);

    cancel_work_sync(&buttonwork);
    free_irq(button_irq, NULL);
    gpio_free_array(buttons, ARRAY_SIZE(buttons));
}

MODULE_LICENSE("GPL");
