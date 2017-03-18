#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

static char *name = "A0134429E";
module_param(name, charp, S_IRUGO);

static int hello_init(void)
{
  printk(KERN_ALERT "Hello, %s\n", name);
  return 0;
}

static void hello_exit(void)
{
  printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);

