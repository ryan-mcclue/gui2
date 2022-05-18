// SPDX-License-Identifier: zlib-acknowledgement

// sudo apt install raspberrypi-kernel-headers

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define ZLIB_LICENSE "Proprietary"
MODULE_LICENSE(ZLIB_LICENSE);
MODULE_AUTHOR("Ryan McClue <re.mcclue@protonmail.com>");
MODULE_DESCRIPTION("Example");

#define INTERNAL static
#define GLOBAL static

INTERNAL int
open_device_file(struct inode *device_file, struct file *instance)
{
  int result = 0;

  return result;
}

INTERNAL int
close_device_file(struct inode *device_file, struct file *instance)
{
  int result = 0;

  return result;
}

INTERNAL int __init
init_point(void)
{
  struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .open = open_device_file,
    .release = close_device_file,
  };

  // IMPORTANT(Ryan): Obtained this unused number with $(cat /proc/devices | grep 90)
  // Could get the kernel to return to us an unused number...
  u32 major_number = 90;
  int char_dev_register_status = register_chrdev(major_number, "Device Name", &file_ops);
  if (char_dev_register_status == 0)
  {
    
  }
  else if (char_dev_register_status > 0)
  {
    s32 major_number = char_dev_register_status >> 20;
    s32 minor_number = char_dev_register_status & 0xfffff; // is this right?
  }
  else
  {
    // error
  }

  // Once loaded, create a device file
  // $(mknod /dev/mydevice c my_major 0 && chmod 666 /dev/mydevice) 
  // then can open("/dev/mydevice");

  return 0;
}

INTERNAL void __exit
exit_point(void)
{
  unregister_chrdev(my_major, "Device Name");
  printk("Goodbye kernel!\n");
}

module_init(init_point);
module_exit(exit_point);
