// SPDX-License-Identifier: zlib-acknowledgement

// amstudio on youtube for making cases etc.

// os already defines device tree for us to have access


#include <linux/gpio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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

typedef struct PWMFileDescriptors
{
  int export;
  int enable;
  int period;
  int duty_cycle;
  int errno_code;
} PWMFileDescriptors;

// for this to be present must add to config.txt: 
// dtoverlay=pwm,pin=12,func=4 
static PWMFileDescriptors
pwm0_file_descriptors_init(void)
{
  PWMFileDescriptors result = {0};

  result.export = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
  if (result.export >= 0)
  {
    if ((access("/sys/class/pwm/pwmchip0/pwm0", F_OK) != 0))
    {
      if (write(result.export, "0", 2) == 2)
      {
        if (access("/sys/class/pwm/pwmchip0/pwm0", F_OK) != 0)
        {
          result.errno_code = errno;
          BP_MSG("Failed to export pwmchip0");
        }
      }
      else
      {
        result.errno_code = errno;
        EBP();
      }
    }
  }
  else
  {
    result.errno_code = errno;
    EBP();
  }

  if (result.errno_code == 0)
  {
    result.period = open("/sys/class/pwm/pwmchip0/pwm0/period", O_WRONLY);
    if (result.period >= 0)
    {
      result.duty_cycle = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
      if (result.duty_cycle >= 0)
      {
        result.enable = open("/sys/class/pwm/pwmchip0/pwm0/enable", O_WRONLY);
        if (result.enable == -1)
        {
          result.errno_code = errno;
          EBP();
        }
      }
      else
      {
        result.errno_code = errno;
        EBP();
      }

    }
    else
    {
      result.errno_code = errno;
      EBP();
    }
  }

  return result;
}

static void
i2c_example(void)
{

}

// said spi speed was in kb?
static void
spi00_example(void)
{
  int spi00_fd = open("/dev/spidev0.0", O_RDWR);
  if (spi00_fd >= 0)
  {
    // spi mode is setting a combination of clock polarity (CPOL; low or high) and 
    // clock phase (CPHA; leading or trailing edge) to sample data
    
    int mode = 0; // CPOL = 0, CPHA = 0
    // _WR means assign, _RD means return
    ioctl(spi00_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi00_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    ioctl(spi00_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);

    // IMPORTANT(Ryan): Wrap up SPI data transfer in function.
    // TODO(Ryan): What word size is the most efficient?

    // full-duplex, therefore more wires than I2C
    char spi00_tx_buf[32] = {0};
    char spi00_rx_buf[32] = {0};
    struct spi_ioc_transfer spi00_transfer[32] = {0};
    for (int buf_count = 0; buf_count < sizeof(spi00_tx_buf); ++buf_count)
    {
      spi00_transfer[buf_count].tx_buf = (unsigned long)(spi00_tx_buf + buf_count);
      spi00_transfer[buf_count].rx_buf = (unsigned long)(spi00_rx_buf + buf_count);
      spi00_transfer[buf_count].len = num_bytes;  // in this case 1
      spi00_transfer[buf_count].speed_hz = speed; 
      spi00_transfer[buf_count].bits_per_word = 8; 
    }

    ioctl(spi00_fd, SPI_IOC_MESSAGE(sizeof(spi00_tx_buf)), spi00_transfer);


    // address lines typically only for I2C as more master slave combinations?

  }
  else
  {
    EBP();
  }
}

int
main(int argc, char *argv[])
{
  PWMFileDescriptors pwm0_file_descriptors = pwm0_file_descriptors_init();
  if (pwm0_file_descriptors.errno_code == 0)
  {
    char pwm0_period_str[16] = {};
    int pwm0_period = SECONDS_NS(1);
    snprintf(pwm0_period_str, sizeof(pwm0_period_str), "%d", pwm0_period);
    int pwm0_period_size = strlen(pwm0_period_str);

    char pwm0_duty_cycle_str[16] = {};
    int pwm0_duty_cycle = pwm0_period * 0.5f;
    snprintf(pwm0_duty_cycle_str, sizeof(pwm0_duty_cycle_str), "%d", pwm0_duty_cycle);
    int pwm0_duty_cycle_size = strlen(pwm0_period_str);

    if (write(pwm0_file_descriptors.period, pwm0_period_str, pwm0_period_size) == pwm0_period_size)
    {
      if (write(pwm0_file_descriptors.duty_cycle, pwm0_duty_cycle_str, pwm0_duty_cycle_size) ==
          pwm0_duty_cycle_size)
      {
        if (write(pwm0_file_descriptors.enable, "1", 2) != 2)
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

#if 0
  // enable SPI kernel driver with $(sudo raspi-config)
  // So, may have to designate GPIO pin as PWM with dtoverlay, or load kernel driver as with SPI
  
  // SPI chip select if pulled low
  
  // look in tools/spi/ for example source files
  // spi is master-slave. no maximum clock speed? 
  // (is this for high data rate, hence used in flash?)
  // 4 wires annoyingly differently named. simplisticly:
  // MOSI (master output slave input), MISO (master input slave output), 
  // SCLK (serial clock), SS (slave select)
  // Seems that userspace SPI api is mainly for test, 
  // as we use SPI for speed, so want in kernel

  
#endif

  return 0;
}
