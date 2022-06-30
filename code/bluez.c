// SPDX-License-Identifier: zlib-acknowledgement

#include <ell/ell.h>

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
  #define ASSERTE(cond) if (!(cond)) {EBP();}
#else
  #define BP_MSG(msg)
  #define BP()
  #define EBP()
  #define ASSERT(cond)
  #define ASSERTE(cond)
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <time.h>

#include <signal.h>

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
    result = cur_timespec.tv_nsec + (cur_timespec.tv_sec * (SECONDS_NS(1)));
  }
  else
  {
    EBP();
  }

  return result;
}


typedef void (*dbus_callback_t)(struct l_dbus_message *msg);

INTERNAL void
dbus_callback_wrapper(struct l_dbus_message *reply_message, void *user_data)
{
  const char *path = l_dbus_message_get_path(reply_message);
  const char *interface = l_dbus_message_get_interface(reply_message);
  const char *member = l_dbus_message_get_member(reply_message);
  const char *destination = l_dbus_message_get_destination(reply_message);
  const char *sender = l_dbus_message_get_sender(reply_message);
  const char *signature = l_dbus_message_get_signature(reply_message);

  const char *error_name = NULL;
  const char *error_text = NULL;

  if (l_dbus_message_is_error(reply_message))
  {
    l_dbus_message_get_error(reply_message, &error_name, &error_text);

    char error_message[128] = {0};
    snprintf(error_message, sizeof(error_message), "%s: %s", error_name, 
             error_text);
    BP_MSG(error_message);

    l_free(error_name);
    l_free(error_text);
  }
  else
  {
    dbus_callback_t callback = (dbus_callback_t)user_data;
    callback(reply_message);
  }
}

INTERNAL void 
dbus_request_name_callback(struct l_dbus *dbus_connection, bool success, bool queued, void *user_data)
{
  if (!success)
  {
    BP_MSG("Failed to acquire dbus name"); 
  }
}

typedef struct BluetoothDevice
{
  char dbus_path[128];
  char address[128];
  s32 rssi;
} BluetoothDevice;

GLOBAL b32 global_want_to_run = true;

GLOBAL struct l_hashmap *global_bluetooth_devices_map = NULL;
#define MAX_BLUETOOTH_DEVICES_COUNT 16
GLOBAL BluetoothDevice global_bluetooth_devices[MAX_BLUETOOTH_DEVICES_COUNT];
GLOBAL u32 global_bluetooth_device_count = 0;

GLOBAL struct l_dbus *global_dbus_connection = NULL;


INTERNAL void 
bluez_interfaces_added_callback(struct l_dbus_message *reply_message)
{
  BluetoothDevice *active_bluetooth_device = NULL;

  struct l_dbus_message_iter root_dict_keys_iter, root_dict_values_iter = {0};
  const char *dbus_path = NULL;
  if (l_dbus_message_get_arguments(reply_message, "oa{sa{sv}}", &dbus_path, &root_dict_keys_iter))
  {
    printf("Found device: %s\n", dbus_path);
    const char *root_dict_key = NULL;
    while (l_dbus_message_iter_next_entry(&root_dict_keys_iter, &root_dict_key, &root_dict_values_iter))
    {
      if (strcmp(root_dict_key, "org.bluez.Device1") == 0)
      {
        const char *device_dict_key = NULL;
        struct l_dbus_message_iter device_dict_values_iter = {0};
        while (l_dbus_message_iter_next_entry(&root_dict_values_iter, &device_dict_key, &device_dict_values_iter))
        {
          if (strcmp(device_dict_key, "Address") == 0)
          {
            const char *address = NULL;
            l_dbus_message_iter_get_variant(&device_dict_values_iter, "s", &address);
            printf("Found device: %s\n", address);
            
            ASSERT(global_bluetooth_device_count != MAX_BLUETOOTH_DEVICES_COUNT);
            active_bluetooth_device = &global_bluetooth_devices[global_bluetooth_device_count++];
            strcpy(active_bluetooth_device->dbus_path, dbus_path); 
            strcpy(active_bluetooth_device->address, address); 

            bool bluetooth_device_insert_status = l_hashmap_insert(global_bluetooth_devices_map, address, active_bluetooth_device);
            ASSERT(bluetooth_device_insert_status);
          }
          if (strcmp(device_dict_key, "RSSI") == 0)
          {
            s16 rssi = 0;
            l_dbus_message_iter_get_variant(&device_dict_values_iter, "n", &rssi);

            ASSERT(active_bluetooth_device != NULL);
            active_bluetooth_device->rssi = rssi;
          }
        }
      }
      else if (strcmp(root_dict_key, "org.bluez.GattService1") == 0)
      {
        const char *service_dict_key = NULL;
        struct l_dbus_message_iter service_dict_values_iter = {0};
        while (l_dbus_message_iter_next_entry(&root_dict_values_iter, &service_dict_key, &service_dict_values_iter))
        {
// SIG UUIDs --> 0000xxxx-0000-1000-8000-00805f9b34fb (https://www.bluetooth.com/specifications/assigned-numbers/)
// 0x2a05 --> 00002a05-0000-1000-8000-00805f9b34fb
// 0x1801 -- > 00001801-0000-1000-8000-00805f9b34fb
          // probably want dbus path as well
          // NOTE(Ryan): We could determine UUID from d-feet (allows calling methods as well)
          if (strcmp(service_dict_key, "UUID") == 0)
          {
            
          }
          // TODO(Ryan): ServicesResolved when done
        }
      }
      else if (strcmp(root_dict_key, "org.bluez.GattCharacteristic1") == 0)
      {
        // UUID and Flags (ensure has 'read' flag)
        // ReadValue({})
        // WriteValue(bytes, {}) (if throughput becomes an issue, may have to specify write command instead of request?)
        // better to just write less than 20 bytes at a time?
      }
      else if (strcmp(root_dict_key, "org.bluez.GattDescriptor1") == 0)
      {
        // UUID
      }
    }
  }
  else
  {
    BP_MSG("InterfacesAdded dict expected but not recieved");
  }
}

INTERNAL void 
bluez_start_discovery_callback(struct l_dbus_message *reply_message)
{
  printf("Searching for unmanaged bluetooth devices...\n");
}

INTERNAL void
bluez_connect_callback(struct l_dbus_message *reply_message)
{
}


INTERNAL void 
bluez_stop_discovery_callback(struct l_dbus_message *reply_message)
{
  printf("Found devices:\n");

  BluetoothDevice *device = l_hashmap_lookup(global_bluetooth_devices_map, "E4:15:F6:5F:FA:9D");
  if (device != NULL)
  {
    // TODO(Ryan): Ensure connection successful by looking at Connected property
    struct l_dbus_message *bluez_connect_msg = l_dbus_message_new_method_call(global_dbus_connection, "org.bluez", device->dbus_path, 
                                                                              "org.bluez.Device1", "Connect");
    ASSERT(bluez_connect_msg != NULL);

    bool bluez_connect_msg_set_argument_status = l_dbus_message_set_arguments(bluez_connect_msg, "");
    ASSERT(bluez_connect_msg_set_argument_status);

    l_dbus_send_with_reply(global_dbus_connection, bluez_connect_msg, dbus_callback_wrapper, bluez_connect_callback, NULL);
  }
  else
  {
    printf("HMSoft not found!\n");
  }

  global_want_to_run = false;
}

INTERNAL void
bluez_get_managed_objects_callback(struct l_dbus_message *reply_message)
{
  struct l_dbus_message_iter root_dict_keys_iter = {0}, root_dict_values_iter = {0};
  if (l_dbus_message_get_arguments(reply_message, "a{oa{sa{sv}}}", &root_dict_keys_iter))
  {
    const char *root_dict_key = NULL;
    while (l_dbus_message_iter_next_entry(&root_dict_keys_iter, &root_dict_key, &root_dict_values_iter))
    {
      // IMPORTANT(Ryan): Our device: /org/bluez/hci0, remote device: /org/bluez/hci0/dev_*
      if (strncmp(root_dict_key, "/org/bluez/hci0/", sizeof("/org/bluez/hci0/") - 1) == 0)
      {
        const char *objects_dict_key = NULL;
        struct l_dbus_message_iter objects_dict_values_iter = {0};
        while (l_dbus_message_iter_next_entry(&root_dict_values_iter, &objects_dict_key, &objects_dict_values_iter))
        {
          if (strcmp(objects_dict_key, "org.bluez.Device1") == 0)
          {
            const char *properties_dict_key = NULL;
            struct l_dbus_message_iter properties_dict_values_iter = {0};
            while (l_dbus_message_iter_next_entry(&objects_dict_values_iter, &properties_dict_key, &properties_dict_values_iter))
            {
              if (strcmp(properties_dict_key, "Address") == 0)
              {
                const char *address = NULL;
                l_dbus_message_iter_get_variant(&properties_dict_values_iter, "s", &address);

                ASSERT(global_bluetooth_device_count != MAX_BLUETOOTH_DEVICES_COUNT);
                BluetoothDevice *bluetooth_device = &global_bluetooth_devices[global_bluetooth_device_count++];
                strcpy(bluetooth_device->dbus_path, root_dict_key); 
                strcpy(bluetooth_device->address, address); 

                bool bluetooth_device_insert_status = l_hashmap_insert(global_bluetooth_devices_map, address, bluetooth_device);
                ASSERT(bluetooth_device_insert_status);
              }
            }
          }
        }
      }
    }
  }
  else
  {
    BP_MSG("GetManagedObjects dict expected but not recieved");
  }
}


INTERNAL void
falsify_global_want_to_run(int signum)
{
  global_want_to_run = false;
}

// IMPORTANT(Ryan): If we want to inspect the type information of a message, use
// $(sudo dbus-monitor --system)

// IMPORTANT(Ryan): Don't have $(bluetoothctl) open when running application, as it will intercept bluez devices first!!!

// IMPORTANT(Ryan): $(usermod -a -G dialout ryan) to get access to serial port

int main(int argc, char *argv[])
{
  if (l_main_init())
  {
    global_dbus_connection = l_dbus_new_default(L_DBUS_SYSTEM_BUS);
    if (global_dbus_connection != NULL)
    { 
      __sighandler_t prev_signal_handler = signal(SIGINT, falsify_global_want_to_run);
      ASSERTE(prev_signal_handler != SIG_ERR);

      global_bluetooth_devices_map = l_hashmap_string_new();
      ASSERT(global_bluetooth_devices_map != NULL);

      // NOTE(Ryan): Cannot acquire name on system bus without altering dbus permissions  
      // l_dbus_name_acquire(global_dbus_connection, "my.bluetooth.app", false, false, false, dbus_request_name_callback, NULL);
      
      // TODO(Ryan): Watch for InterfacesRemoved and PropertiesChanged during discovery phase
      unsigned int bluez_interfaces_added_id = l_dbus_add_signal_watch(global_dbus_connection, "org.bluez", "/", 
                                                                       "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", 
                                                                       L_DBUS_MATCH_NONE, dbus_callback_wrapper, bluez_interfaces_added_callback);
      ASSERT(bluez_interfaces_added_id != 0);

      struct l_dbus_message *bluez_start_discovery_msg = l_dbus_message_new_method_call(global_dbus_connection, "org.bluez", "/org/bluez/hci0", 
                                                                                       "org.bluez.Adapter1", "StartDiscovery");
      ASSERT(bluez_start_discovery_msg != NULL);

      bool bluez_start_discovery_msg_set_argument_status = l_dbus_message_set_arguments(bluez_start_discovery_msg, "");
      ASSERT(bluez_start_discovery_msg_set_argument_status);

      l_dbus_send_with_reply(global_dbus_connection, bluez_start_discovery_msg, dbus_callback_wrapper, bluez_start_discovery_callback, NULL);


      struct l_dbus_message *bluez_get_managed_objects_msg = l_dbus_message_new_method_call(global_dbus_connection, "org.bluez", "/", 
                                                                                            "org.freedesktop.DBus.ObjectManager", 
                                                                                            "GetManagedObjects");
      ASSERT(bluez_get_managed_objects_msg != NULL);

      bool bluez_get_managed_objects_msg_set_argument_status = l_dbus_message_set_arguments(bluez_get_managed_objects_msg, "");
      ASSERT(bluez_get_managed_objects_msg_set_argument_status);

      l_dbus_send_with_reply(global_dbus_connection, bluez_get_managed_objects_msg, dbus_callback_wrapper, bluez_get_managed_objects_callback, NULL);


      u64 start_time = get_ns();
      u64 discovery_time = SECONDS_NS(5);
      b32 are_discovering = true;
      while (global_want_to_run)
      {
        if (are_discovering)
        {
          if (get_ns() - start_time >= discovery_time)
          {
            are_discovering = false;

            struct l_dbus_message *bluez_stop_discovery_msg = l_dbus_message_new_method_call(global_dbus_connection, "org.bluez", "/org/bluez/hci0", 
                                                                                            "org.bluez.Adapter1", "StopDiscovery");
            ASSERT(bluez_stop_discovery_msg != NULL);

            bool bluez_stop_discovery_msg_set_argument_status = l_dbus_message_set_arguments(bluez_stop_discovery_msg, "");
            ASSERT(bluez_stop_discovery_msg_set_argument_status);

            l_dbus_send_with_reply(global_dbus_connection, bluez_stop_discovery_msg, dbus_callback_wrapper, bluez_stop_discovery_callback, NULL);

            l_dbus_remove_signal_watch(global_dbus_connection, bluez_interfaces_added_id);
          }
        }

        // NOTE(Ryan): This will hang when no events left, i.e. return -1
        // int timeout = l_main_prepare();
        l_main_iterate(0);
      }

      l_dbus_destroy(global_dbus_connection);
      l_main_exit();
    }
    else
    {
      BP_MSG("Unable to create dbus connection");
      l_main_exit();
    }

  }
  else
  {
    BP_MSG("Unable to initialise ELL main loop");
  }

}

/*
 *
UUID_NAMES = {
    "00001801-0000-1000-8000-00805f9b34fb" : "Generic Attribute Service",
    "0000180a-0000-1000-8000-00805f9b34fb" : "Device Information Service",
    "e95d93b0-251d-470a-a062-fa1922dfa9a8" : "DFU Control Service",
    "e95d93af-251d-470a-a062-fa1922dfa9a8" : "Event Service",
    "e95d9882-251d-470a-a062-fa1922dfa9a8" : "Button Service",
    "e95d6100-251d-470a-a062-fa1922dfa9a8" : "Temperature Service",
    "e95dd91d-251d-470a-a062-fa1922dfa9a8" : "LED Service",
    "00002a05-0000-1000-8000-00805f9b34fb" : "Service Changed",
    "e95d93b1-251d-470a-a062-fa1922dfa9a8" : "DFU Control",
    "00002a05-0000-1000-8000-00805f9b34fb" : "Service Changed",
    "00002a24-0000-1000-8000-00805f9b34fb" : "Model Number String",
    "00002a25-0000-1000-8000-00805f9b34fb" : "Serial Number String",
    "00002a26-0000-1000-8000-00805f9b34fb" : "Firmware Revision String",
    "e95d9775-251d-470a-a062-fa1922dfa9a8" : "micro:bit Event",
    "e95d5404-251d-470a-a062-fa1922dfa9a8" : "Client Event",
    "e95d23c4-251d-470a-a062-fa1922dfa9a8" : "Client Requirements",
    "e95db84c-251d-470a-a062-fa1922dfa9a8" : "micro:bit Requirements",
    "e95dda90-251d-470a-a062-fa1922dfa9a8" : "Button A State",
    "e95dda91-251d-470a-a062-fa1922dfa9a8" : "Button B State",
    "e95d9250-251d-470a-a062-fa1922dfa9a8" : "Temperature",
    "e95d93ee-251d-470a-a062-fa1922dfa9a8" : "LED Text",
    "00002902-0000-1000-8000-00805f9b34fb" : "Client Characteristic Configuration",
}    

DEVICE_INF_SVC_UUID = "0000180a-0000-1000-8000-00805f9b34fb"
MODEL_NUMBER_UUID    = "00002a24-0000-1000-8000-00805f9b34fb"

TEMPERATURE_SVC_UUID = "e95d6100-251d-470a-a062-fa1922dfa9a8"
TEMPERATURE_CHR_UUID = "e95d9250-251d-470a-a062-fa1922dfa9a8"

LED_SVC_UUID = "e95dd91d-251d-470a-a062-fa1922dfa9a8"
LED_TEXT_CHR_UUID = "e95d93ee-251d-470a-a062-fa1922dfa9a8"
 *
 * HM-10 capabilties: http://www.martyncurrey.com/hm-10-bluetooth-4ble-modules/#HM-10_Services_and_Characteristics
 *
 * Values to TX/RX are only interpreted as AT commands prior to connection 
 *
 * On device connect, recieve: OK+CONN
 * 
 * AT+ADDR? 
 * AT+VERR?
 * AT+NAME? (AT+NAMERYAN)
 * AT+PASS? (AT+PASS123456)
 * AT+ROLE? (0 for peripheral, 1 for central). AT+ROLE0 to set
 *
 * AT+TYPE2 (set bond mode to authorise, i.e. require password and pair to connect)
 *
 * AT+UUID? (service UUID)
 * AT+CHAR? (characteristic value)
 *
 * So, if just reading bytes from TX/RX this is the default characteristic, i.e. can only have 1 characteristic on HM-10?
 * When writing raw bytes to the TX/RX we are altering the value of the default characteristic (however, does this send it or will remote only get it if they subscribe?)
 * (remote can directly read, however notification of change also)
 */

      // this won't work if device requires pairing or isn't advertising
      // IMPORTANT: Once connected, the Connected property of the device will be set, as will be seen in device PropertiesChanged
      // We could check the Connected property prior to attempting a connection using an appropriate Get()

      /*
        l_dbus_add_signal_watch(global_dbus_connection, "org.bluez", "/", 
                                "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", 
                                L_DBUS_MATCH_NONE, dbus_interfaces_removed_callback, NULL);
        IMPORTANT: This PropertiesChanged is only during discovery phase
        l_dbus_add_signal_watch(global_dbus_connection, "org.bluez", "/org/bluez/hci0", 
                                "org.freedesktop.DBus.Properties", "PropertiesChanged", 
                                L_DBUS_MATCH_NONE, dbus_properties_changed_callback, NULL);
       */

      // IMPORTANT(Ryan): InterfacesRemoved could be called during this discovery time
      //if (dbus_interfaces_added_id != 0)
      //{
      //  l_dbus_remove_signal_watch(global_dbus_connection, dbus_interfaces_added_id);
      //}
