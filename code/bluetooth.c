// SPDX-License-Identifier: zlib-acknowledgement

// gateway --> $(route -n)
// mac -> $(ip address show)
// create static ip lease 
// add in hostname in /etc/hosts (however if DHCP on server, just reconnect to get this?)

// RFCOMM protocol. 
// Serial Port Profile is based on RFCOMM protocol
// profile will have a UUID
//
// to connect to bluetooth socket, require mac address like AB:12:4B:59:23:0A
// so, convert from "Connecting to /org/bluez/hci0/dev_5C_03_39_C5_BA_C7"

// L2CAP, MGMT, HCI sockets?

// investigate $(btmon) $(btmgt)

#include "types.h"

#if defined(GUI_INTERNAL)
  INTERNAL void __bp(char const *file_name, char const *func_name, int line_num,
                     char const *optional_message)
  { 
    fprintf(stderr, "BREAKPOINT TRIGGERED! (%s:%s:%d)\n\"%s\"\n", file_name, func_name, 
            line_num, optional_message);
#if !defined(GUI_DEBUGGER)
    exit(1);
#endif
  }
  INTERNAL void __ebp(char const *file_name, char const *func_name, int line_num)
  { 
    char *errno_msg = strerror(errno);
    fprintf(stderr, "ERRNO BREAKPOINT TRIGGERED! (%s:%s:%d)\n\"%s\"\n", file_name, 
            func_name, line_num, errno_msg);
#if !defined(GUI_DEBUGGER)
    exit(1);
#endif
  }
  #define BP_MSG(msg) __bp(__FILE__, __func__, __LINE__, msg)
  #define BP() __bp(__FILE__, __func__, __LINE__, "")
  #define EBP() __ebp(__FILE__, __func__, __LINE__)
  #define ASSERT(cond) if (!(cond)) {BP();}
#else
  #define BP_MSG(msg)
  #define BP()
  #define EBP()
  #define ASSERT(cond)
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <gio/gio.h>
#include <gio/gnetworking.h>
#include <glib.h>

// IMPORTANT(Ryan): may have to "sudo chmod 666 /dev/ttyUSB*" 

// Johannes 4GNU_Linux

INTERNAL int
obtain_serial_connection(char *path)
{
  int result = 0;

  int fd = open(path, O_RDWR | O_NDELAY | O_NOCTTY);
  if (fd > 0)
  {
    struct termios serial_options = {0};
    u32 config_8n1 = CS8 | CLOCAL | CREAD;
    serial_options.c_cflag = config_8n1 | B9600;
    serial_options.c_iflag = IGNPAR;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &serial_options);

    //write(bluetooth_serial_fd, data, len);
    //if (read(tty_fd,&c,1)>0)

    ////read a webpage that i have downloaded link https://www.cmrr.umn.edu/~strupp/serial.html
    //// 8n1, see termios.h for more 
    //bluetooth_serial_port.c_cc[VMIN] = 1; //timeout period it should be set to zero if you want to print 0 if nothing is received before timeout, 
    //bluetooth_serial_port.c_cc[VTIME] = 5; //time out period in units of 1/10th of a second, so here the period is 500ms
  }
  else
  {
    EBP();
  }

  result = fd;

  return result;
}


INTERNAL void
sleep_ms(int ms)
{
  struct timespec sleep_time = {0};
  sleep_time.tv_nsec = ms * 1000000;
  struct timespec leftover_sleep_time = {0};
  nanosleep(&sleep_time, &leftover_sleep_time);
}

#define MAX_BLUETOOTH_DEVICE_COUNT 32
GLOBAL u32 global_bluetooth_device_count;
GLOBAL char global_bluetooth_devices[MAX_BLUETOOTH_DEVICE_COUNT][64];

INTERNAL void 
on_adapter_changed(GDBusConnection *conn, const gchar *sender_name, const gchar *object_path, 
                   const gchar *interface_name, const gchar *signal_name, GVariant *parameters,
                   gpointer user_data)
{
  char *adapter_path = (char *)user_data;

  if (strcmp(object_path, adapter_path) != 0)
  {
    ASSERT(global_bluetooth_device_count < MAX_BLUETOOTH_DEVICE_COUNT);

    if (strstr(object_path, adapter_path) != NULL)
    {
      // do we need to check for unique devices?
      strcpy(global_bluetooth_devices[global_bluetooth_device_count++], object_path);
    }
  }
}

int 
main(int argc, char *argv[])
{
// stackoverflow user: ukBaz

// systemctl status bluetooth (determine actual binary from /lib/systemd/system/bluetooth.service, so $(man bluetoothd). might have to pass --experimental?)
// $(sudo bluetoothctl; list; show; select; power on;)
// 'devices' command will list devices found through 'scan on/off'. then do a 'connect' to list peripheral characteristics.
// Once we are connected, will print out DBUS paths to the characteristics and services we want (or just do 'list-attributes')
// Now do 'select-attribute <dbus-path>'. Subsequently running 'attribute-info' will give UUID and flags
// Now could do 'write 0x12 0x23 ...'
// Finally run 'disconnect'
// For more informative error information use $(sudo btmgmt)
// Discover that by default bluetooth is soft blocked by rfkill $(sudo rfkill list), so $(sudo rfkill unblock bluetooth), 
// $(sudo systemctl restart bluetooth). For some reason this might require $(pulseaudio -k)

// bluetooth LE was a part of 4.0 specification 
// used for control signals or sensor data. point-to-point not mesh

// Classic bluetooth requires a connection to send data. used for mice, keyboards, etc.
// Has fixed set of profiles to use, e.g. serial, audio, etc.

// Does BLE only advertise or can it connect directly?
// GATT is a hierarchy of data a device exposes (profile -> service -> characteristic (actual data))
// So when talking to a device, we are really dealing with a particular characteristic of a service
// So, we say our central looks at a peripherals GATT services that it exposes.
// TODO(Ryan): GATT is a type of service? Other types of services may include 'Tx Power', 'Battery Service'
// We will modify the characteristics of that service
// There are standard services that we would expect to find

// Interestingly, without extensions, bluetooth has no encryption or authentication so can just connect to any?

// BLE transmitter only on if being read or written to (so it just broadcasts data that others listen to?)
// subscribe to data changes in GATT database?
// e.g. GATT keys are 128bit UUIDs, 95DDA90-251D-470A-A062-FA1922DFA9A8
// peripheral advertises; central scans and connects
// also have Beacon (only transmitting) and Observer
// can create custom profiles (generic architecture; is this a gatt?)

// Special interest group has reserved values for official profiles
// Serial port profile referred to by 0x1101 (in actuality 128bits, just shortened because official)

// async by nature?

// pairing only for secure connection, not necessary

// little-endian except for beacons?
  
// dbus service (org.bluez), object path (/org/bluez/hci0)

#if 0
  // print out RSSI https://www.youtube.com/watch?v=W8TQONjd6kw&t=544s

  //int bluetooth_fd = obtain_serial_connection("/dev/ttyACM0");
  //close(bluetooth_fd);

  int adapter_id = hci_get_route(NULL);
  if (adapter_id >= 0)
  {
    struct hci_dev_info hci_info = {0};
    if (hci_devinfo(adapter_id, &hci_info) >= 0)
    {
      char *bus_name = "org.bluez";
      char *interface_name = "org.bluez.Adapter1";
      char object_path[256] = {"/org/bluez/"};
      strcat(object_path, hci_info.name);

      // IMPORTANT(Ryan): May have to be root or alter DBUS permissions to access system bus
      GError *g_error = NULL;
      GDBusConnection *dbus_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &g_error);
      if (dbus_connection != NULL)
      {
        //start_async_device_discovery();
        // IMPORTANT(Ryan): This will look for both classic and LE
        // Look into Parthiban gists to filter just LE
        u32 scan_subscription_id = \
          g_dbus_connection_signal_subscribe(dbus_connection, "org.bluez",
                                              "org.freedesktop.DBus.Properties",
                                              "PropertiesChanged", NULL, NULL,
                                              G_DBUS_SIGNAL_FLAGS_NONE,
                                              on_adapter_changed, (void *)object_path, NULL);
        // method call
        GVariant *res = g_dbus_connection_call_sync(dbus_connection, bus_name, object_path,
              interface_name, "StartDiscovery", NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
              -1, NULL, &g_error);
        g_variant_unref(res);

        u32 seconds_to_scan = 10;
        printf("Waiting %d seconds\n ...", seconds_to_scan); 
        sleep_ms(1000 * seconds_to_scan);
        
        //stop_async_device_discovery();
        g_dbus_connection_signal_unsubscribe(dbus_connection, scan_subscription_id);
        res = g_dbus_connection_call_sync(dbus_connection, bus_name, object_path,
              interface_name, "StopDiscovery", NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
              -1, NULL, &g_error);
        g_variant_unref(res);
        
        printf("Found ...\n");
        for (u32 bluetooth_device_i = 0;
             bluetooth_device_i < global_bluetooth_device_count;
             ++bluetooth_device_i)
        {
          printf("%s\n", global_bluetooth_devices[bluetooth_device_i]);
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
    EBP();
  }
#endif

  return 0;
}

#if 0

#define BT_BLUEZ_NAME "org.bluez"
#define BT_MANAGER_PATH "/"
#define BT_ADAPTER_INTERFACE    "org.bluez.Adapter1"
#define BT_DEVICE_IFACE     "org.bluez.Device1"
#define BT_MANAGER_INTERFACE "org.freedesktop.DBus.ObjectManager"
#define BT_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

int main(void)
{
    char *known_address = "2C:F0:A2:26:D7:F5"; /*This is your address to search */

        conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
        proxy =  g_dbus_proxy_new_sync(conn, G_DBUS_PROXY_FLAGS_NONE, NULL, BT_BLUEZ_NAME, BT_MANAGER_PATH, BT_MANAGER_INTERFACE, NULL, &err);
        result = g_dbus_proxy_call_sync(proxy, "GetManagedObjects", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &err);

    g_variant_get(result, "(a{oa{sa{sv}}})", &iter);

        char *device_path = NULL;
        char device_address[18] = { 0 };
        /* Parse the signature:  oa{sa{sv}}} */
        while (g_variant_iter_loop(iter, "{&oa{sa{sv}}}", &device_path, NULL)) {
        {
            char address[BT_ADDRESS_STRING_SIZE] = { 0 };
            char *dev_addr;

            dev_addr = strstr(device_path, "dev_");
            if (dev_addr != NULL) {
                char *pos = NULL;
                dev_addr += 4;
                g_strlcpy(address, dev_addr, sizeof(address));

                while ((pos = strchr(address, '_')) != NULL) {
                    *pos = ':';
                }

                g_strlcpy(device_address, address, BT_ADDRESS_STRING_SIZE);
            }

        }

        if (g_strcmp0(known_address, device_address) == 0) {
            /* Find the name of the device */
            device_property_proxy = g_dbus_proxy_new_sync(conn, G_DBUS_PROXY_FLAGS_NONE, NULL, BT_BLUEZ_NAME, &device_path, BT_PROPERTIES_INTERFACE, NULL, NULL);
            result = g_dbus_proxy_call_sync(proxy, "Get", g_variant_new("(ss)", BT_DEVICE_IFACE, "Alias"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

            const char *local = NULL;
            g_variant_get(result, "(v)", &temp);
            local = g_variant_get_string(temp, NULL);
            printf("Your desired name : %s\n", local);
        }
        }
}

#endif

