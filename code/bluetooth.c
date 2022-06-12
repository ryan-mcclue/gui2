// SPDX-License-Identifier: zlib-acknowledgement

// gateway --> $(route -n)
// mac -> $(ip address show)
// create static ip lease 
// add in hostname in /etc/hosts (however if DHCP on server, just reconnect to get this?)

#include "types.h"
#include "bluetooth.h"

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


#include <time.h>

#include <signal.h>

#include <ell/ell.h>

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
proxy_added(struct l_dbus_proxy *proxy, void *user_data)
{
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);

	printf("(PROXY ADDED) Path: %s, Interface: %s\n", path, interface);

	//if (!strcmp(interface, "org.bluez.Adapter1") ||
	//			!strcmp(interface, "org.bluez.Device1")) {
	//	char *str;

	//	if (!l_dbus_proxy_get_property(proxy, "Address", "s", &str))
	//		return;

	//	l_info("   Address: %s", str);
	//}
}

INTERNAL void 
proxy_removed(struct l_dbus_proxy *proxy, void *user_data)
{
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);
	printf("(PROXY REMOVED) Path: %s, Interface: %s\n", path, interface);
}

INTERNAL void 
property_changed(struct l_dbus_proxy *proxy, const char *name,
				struct l_dbus_message *msg, void *user_data)
{
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);
	printf("(PROPERTY CHANGED) Path: %s, Interface: %s\n", path, interface);

	//if (!strcmp(name, "Address")) {
	//	char *str;

	//	if (!l_dbus_message_get_arguments(msg, "s", &str)) {
	//		return;
	//	}

	//	l_info("   Address: %s", str);
	//}
}

INTERNAL void
start_discovery_cb(struct l_dbus_message *reply, void *user_data)
{
  printf("StartDiscovery() returned\n");

  const char *err_name, *err_text = NULL;
  if (l_dbus_message_is_error(reply))
  {
    l_dbus_message_get_error(reply, &err_name, &err_text);

    l_free(err_name);
    l_free(err_text);
  }
}

INTERNAL void
get_hostname_cb(struct l_dbus_message *reply, void *user_data)
{
	struct l_dbus_message_iter iter = {0};
	const char *hostname = NULL;

	l_dbus_message_get_arguments(reply, "v", &iter);
  printf("Container type: %c ..... ", iter.container_type);

	l_dbus_message_iter_get_variant(&iter, "s", &hostname);

  printf("Hostname: %s\n", hostname);

  // l_free()
}

INTERNAL void
parse_array(struct l_dbus_message *reply, void *user_data)
{
	struct l_dbus_message_iter iter = {0};
	const char *arr = NULL;

  l_dbus_message_get_arguments(reply, "as", &iter);

  b32 elem_recieved = false;
  do
  {
    elem_recieved = l_dbus_message_iter_next_entry(&iter, &arr);
  } while (elem_recieved);

  // need to l_dbus_message_unref(msg)?
}

INTERNAL void
parse_dict(struct l_dbus_message *reply, void *user_data)
{
	struct l_dbus_message_iter dict = {0};
  l_dbus_message_get_arguments(reply, "a{sv}", &dict);

	struct l_dbus_message_iter iter = {0};
	const char *key = NULL;
	l_dbus_message_iter_next_entry(&dict, &key, &iter);

  b32 elem_recieved = false;
  //do
  //{
  //  elem_recieved = l_dbus_message_iter_next_entry(&iter, &arr);
  //} while (elem_recieved);
}

GLOBAL b32 global_want_to_run = true;

INTERNAL void
falsify_global_want_to_run(int signum)
{
  global_want_to_run = false;
}

#define SECONDS_MS(sec) (sec * 1000LL)
#define SECONDS_US(sec) (SECONDS_MS(sec) * 1000LL)
#define SECONDS_NS(sec) (SECONDS_US(sec) * 1000LL)

INTERNAL u64
get_ns(void)
{
  u64 result = 0;

  struct timespec cur_timespec = {0}; 

  if (clock_gettime(CLOCK_MONOTONIC, &cur_timespec) != -1)
  {
    result = cur_timespec.tv_nsec + (cur_timespec.tv_sec * (1 * TERA));
  }
  else
  {
    EBP();
  }

  return result;
}

INTERNAL void 
interfaces_added_cb(struct l_dbus_message *message, void *user_data)
{
  const char *unique_device_path = l_dbus_message_get_path(message);
  printf("(SIGNAL RECIEVED) %s\n", unique_device_path);
#if 0

  // We know is dictionary of variants.
  // So, print out top-level dictionary keys and progress further...

  // message is a dictionary:
  // we know Address is first, however order of other items are not fixed
  // so, iterate over whole dictionary...
  // we expect "org.bluez.Device1" : {
  //   "Address": ,
  //   ....
  //   "Name": ,
  //   "RSSI": ,
  // } 
  //l_dbus_message_get_arguments(message, "a{sv}",);

	const char *interface, *property, *value;
	struct l_dbus_message_iter variant, changed, invalidated;

	if (!signal_timeout)
		return;

	test_assert(l_dbus_message_get_arguments(message, "sa{sv}as",
							&interface, &changed,
							&invalidated));

	test_assert(l_dbus_message_iter_next_entry(&changed, &property,
							&variant));
	test_assert(!strcmp(property, "String"));
	test_assert(l_dbus_message_iter_get_variant(&variant, "s", &value));
	test_assert(!strcmp(value, "foo"));

	test_assert(!l_dbus_message_iter_next_entry(&changed, &property,
							&variant));
	test_assert(!l_dbus_message_iter_next_entry(&invalidated,
							&property));

	test_assert(!new_signal_received);
	new_signal_received = true;

	test_check_signal_success();
#endif
}

//GLOBAL b32 global_dbus_name_has_been_acquired = false;
//      //l_dbus_name_acquire(dbus_conn, "my.bluetooth.app", false, false, false, 
//      //                  set_name, NULL);
//INTERNAL void 
//request_name_callback(struct l_dbus *dbus, bool success, bool queued, void *user_data)
//{
//  global_dbus_name_has_been_acquired = success ? (queued ? "queued" : "success") : "failed";
//}

// $(d-feet) useful!!!!
INTERNAL void
dbus(void)
{
  if (l_main_init())
  {
    struct l_dbus *dbus_conn = l_dbus_new_default(L_DBUS_SYSTEM_BUS);
    if (dbus_conn != NULL)
    {
      signal(SIGINT, falsify_global_want_to_run);

      // NOTE(Ryan): Be notified of advertising packets from unknown devices
      // This will scan for both classic and le devices?
      l_dbus_add_signal_watch(dbus_conn, "org.freedesktop.DBus.ObjectManager", 
          "org.bluez", "InterfacesAdded", L_DBUS_MATCH_NONE, 
          _dbus_callback, interfaces_added_callback);

      // IMPORTANT(Ryan): More elegant to obtain adapter object from GetManagedObjects()
      // However, this is the default in most circumstances 
      DBusMethod start_discovery = create_dbus_method("org.bluez", "/org/bluez/hci0", 
                                                      "org.bluez.Adapter1", "StartDiscovery",
                                                      start_discovery_callback);
      call_dbus_method(dbus_conn, &start_discovery);

      u64 start_ns = get_ns();

      int ell_main_loop_timeout = l_main_prepare();
      while (global_want_to_run)
      {
        if ((get_ns() - start_ns) >= SECONDS_NS(5))
        {
          call_dbus_method(dbus_conn, &stop_discovery);
          // global_want_to_run = false; inside of callback
        }

        l_main_iterate(ell_main_loop_timeout);
      }

      printf("Exiting...\n");

      l_dbus_destroy(dbus_conn);
      l_main_exit();
    }
    else
    {
      EBP();
    }
  }
}

int 
main(int argc, char *argv[])
{
  dbus();
// build from Makefile.am: $(autoreconf -f -i; ./configure)

// FROM BLUETOOTH SIG
// profile is method of obtaining data from device
// GATT (Generic Attribute Profile) defines a table of data that lists
// state and operations that can be performed on them (Attribute Table)
// service -> characteristic -> optional descriptor  
// server and client

// GAP (Generic Access Profile) is how devices discover and connect to each other
// peripheral -> advertises and accepts
// broadcaster -> advertises
// observer -> scans and processes
// central -> scans and connects

// ATT (Attribute Protocol) is how GATT client and server communicate

// HCI is how to interact with bluetooth adapter directly.
// However, we want BlueZ to handle things like GATT/GAP for us

// Could also be part of a bluetooth mesh network

// Once connected to bus, get name starting with colon, e.g. :1.16 (bluetoothd has well known name org.bluez)
// Objects (/org/bluez/...) -> interfaces (org.bluez.GattManager1) -> methods
// Data returned is in another message
// interfaces can return signals (unprompted messages)
// interfaces also have properties which are interacted with Get() and Set() methods

// A proxy object emulates a remote object, and handles routing for us

// DBus messages also have data types

// Will have to explicitly allow connection to DBus bus in a configuration file

// stackoverflow user: ukBaz

// bluefruit examples datamode is server, cmdmode is client
// ble.print(data) sends data to device
// ble.available(); ble.read()
// differences with SPI and UART?
// services like UART may expose read and write characteristics?


// systemctl status bluetooth (determine actual binary from /lib/systemd/system/bluetooth.service, so $(man bluetoothd). might have to pass --experimental?)
// $(sudo bluetoothctl; list; show; select; power on;)
// 'devices' command will list devices found through 'scan on/off'. then do a 'connect' to list peripheral characteristics.
// Once we are connected, will print out DBUS paths to the characteristics and services we want 
// (or just do 'menu gatt' then 'list-attributes <dev>')
// Now do 'select-attribute <dbus-path>'. Subsequently running 'attribute-info' will give UUID and flags
// DBUS tree view $(busctl tree org.bluez)
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

