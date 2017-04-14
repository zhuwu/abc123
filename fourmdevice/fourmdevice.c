#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/string.h>

#define MAJOR_NUMBER 61
#define LIMIT 4194304
#define SCULL_IOC_MAGIC 'k'
#define SCULL_HELLO _IO(SCULL_IOC_MAGIC, 1)
#define SCULL_MSG_WRITE _IOW(SCULL_IOC_MAGIC, 2, int)
#define SCULL_MSG_READ _IOR(SCULL_IOC_MAGIC, 3, int)
#define SCULL_MSG_READ_WRITE _IOWR(SCULL_IOC_MAGIC, 4, int)
#define SCULL_IOC_MAXNR 14
#define DEV_MSG_LIMIT 1024

/* forward declaration */
int fourm_open(struct inode *inode, struct file *filep);
int fourm_release(struct inode *inode, struct file *filep);
ssize_t fourm_read(struct file *filep, char *buf, size_t count, loff_t *f_pos);
ssize_t fourm_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos);
static void fourm_exit(void);
loff_t fourm_llseek(struct file *filep, loff_t off, int whence);
long fourm_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

/* definition of file_operation structure */
struct file_operations fourm_fops = {
  read: fourm_read,
  write: fourm_write,
  open: fourm_open,
  release: fourm_release,
  llseek: fourm_llseek,
  unlocked_ioctl: fourm_ioctl
};

char *fourm_data = NULL;
int data_size = 0;
char *dev_msg = NULL;
int dev_msg_size = 0;
char *temp_msg = NULL;
int temp_msg_size = 0;

long fourm_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
  int err = 0, tmp;
  int retval = 0;

  /*
   * extract the type and number bitfields, and don't decode
   * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
   */
  if (_IOC_TYPE(cmd) != SCULL_IOC_MAGIC) return -ENOTTY;
  if (_IOC_NR(cmd) > SCULL_IOC_MAXNR) return -ENOTTY;

  /*
   * the direction is a bitmask, and VERIFY_WRITE catches R/W
   * transfers. `Type' is user‐oriented, while
   * access_ok is kernel‐oriented, so the concept of "read" and
   * "write" is reversed
   */
  if (_IOC_DIR(cmd) & _IOC_READ)
    err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
  else if (_IOC_DIR(cmd) & _IOC_WRITE)
    err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
  if (err) return -EFAULT;
  switch(cmd) {
    case SCULL_HELLO:
      printk(KERN_WARNING "hello\n");
      break;
    case SCULL_MSG_WRITE:
      printk(KERN_WARNING "MSG write %d\n", arg);
      memset(dev_msg, '\0', DEV_MSG_LIMIT);
      dev_msg_size = strlen((char __user *) arg);
      retval = copy_from_user(dev_msg, (char __user *) arg, dev_msg_size);
      break;
    case SCULL_MSG_READ:
      printk(KERN_WARNING "MSG read %d\n", arg);
      retval = copy_to_user((char __user *) arg, dev_msg, dev_msg_size);
      break;
    case SCULL_MSG_READ_WRITE:
      printk(KERN_WARNING "MSG read write %d\n", arg);
      memset(temp_msg, '\0', DEV_MSG_LIMIT);
      temp_msg_size = dev_msg_size;
      strncpy(temp_msg, dev_msg, dev_msg_size);

      dev_msg_size = strlen((char __user *) arg);
      memset(dev_msg, '\0', DEV_MSG_LIMIT);
      int copy_from_user_result = copy_from_user(dev_msg, (char __user *) arg, dev_msg_size);

      if (copy_from_user_result == 0) {
        retval = copy_to_user((char __user *) arg, temp_msg, temp_msg_size);
        printk(KERN_WARNING "lalala%d\n", retval);
      } else {
        retval = copy_from_user_result;
      }

      printk(KERN_WARNING "%s", dev_msg);
      break;
    default:  /* redundant, as cmd was checked against MAXNR */
      return -ENOTTY;
  }
  return retval;
}

loff_t fourm_llseek(struct file *filep, loff_t off, int whence)
{
  loff_t new_pos;
  switch(whence) {
    case 0: /* SEEK_SET */
      new_pos = off;
      break;
    case 1: /* SEEK_CUR */
      new_pos = filep->f_pos + off;
      break;
    case 2: /* SEEK_END */
      new_pos = data_size + off;
      break;
    default: /* Can't happen */
      return -EINVAL;
  }
  if (new_pos < 0) return -EINVAL;
  filep->f_pos = new_pos;
  return new_pos;
}

int fourm_open(struct inode *inode, struct file *filep)
{
  return 0; // always successful
}

int fourm_release(struct inode *inode, struct file *filep)
{
  return 0; // always successful
}

ssize_t fourm_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{
  printk(KERN_INFO "read pos: %d. count: %d\n", *f_pos, count);
  if (*f_pos >= data_size) {
    printk(KERN_INFO "End of output.\n");
    return 0;
  }
  int error_count = 0;
  int copy_size = (data_size > count) ? count : data_size;
  error_count = copy_to_user(buf, fourm_data + *f_pos, copy_size);
  if (error_count == 0) {
    printk(KERN_INFO "Sent data.\n");
    (*f_pos) += copy_size;
    printk(KERN_INFO "pos --- %d\n", *f_pos);
    return copy_size;
  } else {
    printk(KERN_INFO "Failed to send data.\n");
    return -EFAULT;
  }
}

ssize_t fourm_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos)
{
  printk(KERN_INFO "write pos: %d. count: %d\n", *f_pos, count);
  if (*f_pos == 0) {
    data_size = 0;
  }
  if (*f_pos >= LIMIT) {
    printk(KERN_INFO "End of input.\n");
    return -ENOSPC;
  }
  int error_count = 0;
  int copy_size = (count + data_size <= LIMIT ? count : LIMIT - data_size);
  error_count = copy_from_user(fourm_data + *f_pos, buf, copy_size);
  data_size += copy_size;
  if (error_count == 0) {
    printk(KERN_INFO "Received char. %d\n", copy_size);
    (*f_pos) += copy_size;
    return copy_size;
  } else {
    printk(KERN_INFO "Failed to receive char.\n");
    return -EFAULT;
  }
}

static int fourm_init(void)
{
  int result;
  // register the device
  result = register_chrdev(MAJOR_NUMBER, "fourm", &fourm_fops);
  if (result < 0) {
    return result;
  }
  // allocate one byte of memory for storage
  // kmalloc is just like malloc, the second parameter is
  // the type of memory to be allocated.
  // To release the memory allocated by kmalloc, use kfree.
  fourm_data = kmalloc(LIMIT * sizeof(char), GFP_KERNEL);
  if (!fourm_data) {
    fourm_exit();
    // cannot allocate memory
    // return no memory error, negative signify a failure
    return -ENOMEM;
  }
  dev_msg = kmalloc(DEV_MSG_LIMIT * sizeof(char), GFP_KERNEL);
  if (!dev_msg) {
    fourm_exit();
    return -ENOMEM;
  }
  temp_msg = kmalloc(DEV_MSG_LIMIT * sizeof(char), GFP_KERNEL);
  if (!temp_msg) {
    fourm_exit();
    return -ENOMEM;
  }
  // initialize the value to be X
  // *fourm_data = 'X';
  printk(KERN_ALERT "This is a fourm device module\n");
  return 0;
}

static void fourm_exit(void)
{
  // if the pointer is pointing to something
  if (fourm_data) {
  // free the memory and assign the pointer to NULL
    kfree(fourm_data);
    fourm_data = NULL;
  }
  if (dev_msg) {
    kfree(dev_msg);
    dev_msg = NULL;
  }
  if (temp_msg) {
    kfree(temp_msg);
    temp_msg = NULL;
  }
  // unregister the device
  unregister_chrdev(MAJOR_NUMBER, "fourm");
  printk(KERN_ALERT "Onebyte device module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(fourm_init);
module_exit(fourm_exit);

