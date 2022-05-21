// SPDX-License-Identifier: zlib-acknowledgement

// sudo apt install raspberrypi-kernel-headers

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define ZLIB_LICENSE "Proprietary"
MODULE_LICENSE(ZLIB_LICENSE);
MODULE_AUTHOR("Ryan McClue <re.mcclue@protonmail.com>");
MODULE_DESCRIPTION("Example");

#define INTERNAL static
#define GLOBAL static

GLOBAL char global_buffer[1024];
GLOBAL char *global_buffer_ptr;

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

// called on head -n 1 /dev/devicename
INTERNAL ssize_t
driver_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
  char gpio_data[3] = {0};
  u32 to_copy = min(count, sizeof(gpio_data));
  gpio_data[0] = gpio_get_value(17) + '0';

  u32 bytes_not_copied = copy_to_user(user_buffer, gpio_data, to_copy);
  u32 bytes_copied = to_copy - bytes_not_copied;
  return bytes_copied;
}

// called on echo "some text here" > /dev/devicename
INTERNAL ssize_t
driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
  char value;
  u32 to_copy = min(count, sizeof(value);

  u32 bytes_not_copied = copy_from_user(&value, user_buffer, to_copy);
  if (value == '0')
  {
    // IMPORTANT(Ryan): This kernel interface of writing to gpio seems a lot better than
    // user space?
    gpio_set_value(4, 0);
  }



  u32 bytes_copied = to_copy - bytes_not_copied;
  return bytes_copied;
}

INTERNAL int __init
init_point(void)
{
  struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .open = open_device_file,
    .release = close_device_file,
    .read = driver_read,
    .write = driver_write,
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

  dev_t dev_num = 0;
  struct class *dev_class = NULL;
  struct cdev dev = {0};

  // will create /dev/DeviceName
  if (alloc_chrdev_region(&dev_num, 0, 1, "DeviceName") >= 0)
  {
    major = dev_num >> 20;
    minor = dev_num & 0xfffff;

    dev_class = class_create(THIS_MODULE, "Device Class");
    if (dev_class != NULL)
    {
      if (device_create(dev_class, NULL, dev_num, NULL, "Device Name") != NULL)
      {
        cdev_init(&dev, &file_ops);

        // add to kernel
        if (cdev_add(&dev, dev_num, 1) >= 0)
        {
          // IMPORTANT(Ryan): If another driver has requested this gpio pin, we will fail
          u32 pin_num = 4;
          gpio_request(pin_num, "rpi-gpio-4");
          gpio_direction_output(pin_num, 0); // gpio_direction_input();
          // gpio_free(pin_num);
          
          gpio_set_value(pin_num, 0);
        }
        else
        {
          // error
          device_destroy(dev_class, dev_num);  
        }
      }
      else
      {
        // error
        class_destroy(dev_class);
      }
    }
    else
    {
      // error
      unregister_chrdev(device_num, "Device Name");
    }
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
  cdev_del(&my_dev);
  device_destroy(dev_class, dev_num);  
  class_destroy(dev_class);
  unregister_chrdev(my_major, "Device Name");

  printk("Goodbye kernel!\n");
}

module_init(init_point);
module_exit(exit_point);
