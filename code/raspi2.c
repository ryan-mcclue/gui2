// SPDX-License-Identifier: zlib-acknowledgement

// amstudio on youtube for making cases etc.

// os already defines device tree for us to have access

#include <linux/gpio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>

// look in tools/spi/ for example source files

int
main(int argc, char *argv[])
{
  puts("Hello world!");
#if 0
  // spi is master-slave. no maximum clock speed? (is this for high data rate, hence used in flash?)
  // 4 wires annoyingly differently named. simplisticly:
  // MOSI (master output slave input), MISO (master input slave output), 
  // SCLK (serial clock), SS (slave select)
  // Seems that userspace SPI api is mainly for test, as we use SPI for speed, so want in kernel


  // pwm has sysfs interface, as opposed to modern gpio-cdev interface
  // so, ls /sys/class/pwm/pwmchip0 to list things to read/write to
  // echo 0 > /sys/class/pwm/pwmchip0/export (successful if new directory pwm0 created)
  // period, duty cycle set in nanoseconds
  // echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/period
  // echo 50000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
  // echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable

  
  // /dev/gpiochip*
  struct gpiohandle_request req = {0};
  struct gpiohandle_data data = {0};

  int f = open("/dev/gpiochip1", O_RDONLY);
  req.lineoffsets[0] = 21; // pin number
  req.flags = GPIOHANDLE_REQUEST_OUTPUT;
  req.default_values[0] = 0; // initially we are setting it to 0
  strcpy(req.consumer_label, "gpio-output"); // name for pin
  req.lines = 1; // suppose number of pins?

  int ret = ioctl(f, GPIO_GET_LINEHANDLE_IOCTL, &req);
  // now in req.fd, we have the fd to use

  data.values[0] = 1;
  ret = ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
  close(f);
#endif

  return 0;
}
