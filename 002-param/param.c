#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aleksander Alekseev");
MODULE_DESCRIPTION("A simple driver");

static char* name = "%username%";

// module_param(name, charp, 0);

// S_IRUGO: everyone can read this sysfs entry
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "Enter your name");

static int __init init(void) {
    pr_info("Hello, %s\n", name);
    return 0;
}

static void __exit cleanup(void) {
    pr_info("Goodbye, %s\n", name);
}

module_init(init);
module_exit(cleanup);

