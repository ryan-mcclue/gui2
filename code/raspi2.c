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
#include <stdlib.h>
#include <errno.h>

// look in tools/spi/ for example source files
static void __bp(char const *file_name, char const *func_name, int line_num,
                   char const *optional_message)
{ 
  fprintf(stderr, "BREAKPOINT TRIGGERED! (%s:%s:%d)\n\"%s\"\n", file_name, func_name, 
          line_num, optional_message);
  exit(1);
}
static void __ebp(char const *file_name, char const *func_name, int line_num)
{ 
  char *errno_msg = strerror(errno);
  fprintf(stderr, "ERRNO BREAKPOINT TRIGGERED! (%s:%s:%d)\n\"%s\"\n", file_name, 
          func_name, line_num, errno_msg);
  exit(1);
}
#define BP_MSG(msg) __bp(__FILE__, __func__, __LINE__, msg)
#define BP() __bp(__FILE__, __func__, __LINE__, "")
#define EBP() __ebp(__FILE__, __func__, __LINE__)
#define ASSERT(cond) if (!(cond)) {BP();}

#define SECONDS_NS(seconds) ((seconds) * 1000000000LL)

static void
gpio_example(void)
{
  // NOTE(Ryan): Investigate with $(gpioinfo)
  int gpio_chip0_fd = open("/dev/gpiochip0", O_RDONLY);
  if (gpio_chip0_fd >= 0)
  {
    int led_gpio_number = 17;

    struct gpiohandle_request led_gpio_request = {0};

    led_gpio_request.lines = 1;
    led_gpio_request.lineoffsets[0] = led_gpio_number;

    led_gpio_request.flags = GPIOHANDLE_REQUEST_OUTPUT;
    led_gpio_request.default_values[0] = 0;

    strcpy(led_gpio_request.consumer_label, "led-gpio-output");

    int led_gpio_request_status = ioctl(gpio_chip0_fd, GPIO_GET_LINEHANDLE_IOCTL, 
                                            &led_gpio_request);
    if (led_gpio_request_status >= 0)
    {
      struct gpiohandle_data led_gpio_data = {0};
      led_gpio_data.values[0] = 0;

      int led_data_status = ioctl(led_gpio_request.fd, 
                                      GPIOHANDLE_SET_LINE_VALUES_IOCTL, 
                                      &led_gpio_data);
      if (led_data_status == -1)
      {
        EBP();
      }
    }
    else
    {
      close(gpio_chip0_fd);
      EBP();
    }

    close(gpio_chip0_fd);
  }
  else
  {
    EBP();
  }
}

static void
pwm_example(void)
{
  // for this to be present must add to config.txt: 
  // dtoverlay=pwm,pin=12,func=4 

  // check if exists first
  int pwmchip0_fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
  // int pwmchip0_unexport_fd = open("/sys/class/pwm/pwmchip0/unexport", O_RDWR);
  if (pwmchip0_fd >= 0)
  {
    char *pwm_enable = "0";
    if (write(pwmchip0_fd, pwm_enable, 2) == 2)
    {
      if (access("/sys/class/pwm/pwmchip0/pwm0", F_OK) == 0)
      {
        int pwm0_period_fd = open("/sys/class/pwm/pwmchip0/pwm0/period", O_WRONLY);
        int pwm0_duty_cycle_fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);

        // write 1 for enable, 0 for disable
        int pwm0_enable_fd = open("/sys/class/pwm/pwmchip0/pwm0/enable", O_WRONLY);

        if (pwm0_period_fd >= 0 && pwm0_duty_cycle_fd >= 0 && pwm0_enable_fd >= 0)
        {
          char period_ns[16] = {};
          snprintf(period_ns, sizeof(period_ns), "%d", SECONDS_NS(1));

          char duty_cycle_ns[16] = {};
          snprintf(duty_cycle_ns, sizeof(duty_cycle_ns), "%d", SECONDS_NS(1) * 0.5f);

          ssize_t period_write_status = write(pwm0_period_fd, period_ns, strlen(period_ns));
          ssize_t duty_cycle_write_status = \
            write(pwm0_duty_cycle_fd, duty_cycle_ns, strlen(duty_cycle_ns));
          if (period_write_status == strlen(period_ns) && 
              duty_cycle_write_status == strlen(duty_cycle_ns))
          {
            char *enable = "1";
            if (write(pwm0_enable_fd, enable, 2) != 2)
            {
              EBP();
            }
          }
          else
          {
            EBP();
          }
        }
        else
        {
          EBP();
        }
      }
      else
      {
        BP_MSG("Failed to activate pwmchip0");
      }
    }
    else
    {
      EBP();
    }
  }
  else
  {
    EBP();
  }
  // pwm has sysfs interface, as opposed to modern gpio-cdev interface
  // so, ls /sys/class/pwm/pwmchip0 to list things to read/write to
  // echo 0 > /sys/class/pwm/pwmchip0/export (successful if new directory pwm0 created)
  // period, duty cycle set in nanoseconds
  // echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/period
  // echo 50000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
  // echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable
  
  // echo 0 > /sys/class/pwm/pwmchip0/unexport
}

int
main(int argc, char *argv[])
{
  pwm_example();
#if 0
  // spi is master-slave. no maximum clock speed? (is this for high data rate, hence used in flash?)
  // 4 wires annoyingly differently named. simplisticly:
  // MOSI (master output slave input), MISO (master input slave output), 
  // SCLK (serial clock), SS (slave select)
  // Seems that userspace SPI api is mainly for test, as we use SPI for speed, so want in kernel



  
#endif

  return 0;
}
