// SPDX-License-Identifier: zlib-acknowledgement

// sudo apt install raspberrypi-kernel-headers

#include <linux/module.h>
#include <linux/init.h>

#define ZLIB_LICENSE "Proprietary"
MODULE_LICENSE(ZLIB_LICENSE);
MODULE_AUTHOR("Ryan McClue <re.mcclue@protonmail.com>");
MODULE_DESCRIPTION("Example");

static int __init
init_point(void)
{
  printk("Hello kernel!\n");

  return 0;
}

static void __exit
exit_point(void)
{
  printk("Goodbye kernel!\n");
}

module_init(init_point);
module_exit(exit_point);
