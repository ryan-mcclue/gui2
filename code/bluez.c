// SPDX-License-Identifier: zlib-acknowledgement

#include <ell/ell.h>

#include "types.h"
#include "bluez.h"

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

#include <time.h>

#include <signal.h>


INTERNAL void
dbus_callback_wrapper(struct l_dbus_message *reply_message, void *user_data)
{
  const char *error_name = NULL;
  const char *error_text = NULL;

  if (l_dbus_message_is_error(reply_message))
  {
    l_dbus_message_get_error(reply_message, &error_name, &error_text);

    char error_message[128] = {0};
    snprintf(error_message, sizeof(error_message), "(DBUS ERROR): %s: %s", error_name, 
             error_text);
    BP_MSG(error_message);

    l_free(error_name);
    l_free(error_text);
  }
  else
  {
    dbus_method_callback_t callback = (dbus_method_callback_t)user_data;
    callback(reply_message);
  }
}

INTERNAL DBusMethod
create_dbus_method(struct l_dbus *connection, char *service, char *object, char *interface, 
                   char *method, dbus_method_callback_t callback)
{
  DBusMethod result = {0};

  result.connection = connection;

  strncpy(result.service, service, MAX_DBUS_METHOD_STR_LEN);
  strncpy(result.object, object, MAX_DBUS_METHOD_STR_LEN);
  strncpy(result.interface, interface, MAX_DBUS_METHOD_STR_LEN);
  strncpy(result.method, method, MAX_DBUS_METHOD_STR_LEN);

  result.message = l_dbus_message_new_method_call(result.connection, result.service, result.object, 
                                                  result.interface, result.method);

  result.callback = callback;

  return result; 
}

INTERNAL void
call_dbus_method(DBusMethod *method)
{
  l_dbus_send_with_reply(method->connection, method->message, dbus_callback_wrapper, 
                         method->callback, NULL);

  l_dbus_message_unref(method->message);
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
    result = cur_timespec.tv_nsec + (cur_timespec.tv_sec * (SECONDS_NS(1)));
  }
  else
  {
    EBP();
  }

  return result;
}

#if 0
INTERNAL void 
interfaces_added_callback(struct l_dbus_message *reply_message, void *user_data)
{
  const char *error_name = NULL;
  const char *error_text = NULL;

  if (l_dbus_message_is_error(reply_message))
  {
    l_dbus_message_get_error(reply_message, &error_name, &error_text);

    char error_message[128] = {0};
    snprintf(error_message, sizeof(error_message), "(DBUS ERROR): %s: %s", error_name, 
             error_text);
    BP_MSG(error_message);

    l_free(error_name);
    l_free(error_text);
  }
  else
  {
    const char *unique_device_path = l_dbus_message_get_path(reply_message);
    printf("(SIGNAL RECIEVED) %s\n", unique_device_path);
  }

  //struct l_hashmap *hashmap = l_hashmap_string_new();

  //if (l_hashmap_insert(hashmap, "something", some_struct))
  //{

  //}

#endif


static void debug_callback(const char *str, void *user_data) {
  const char *prefix = user_data;
  printf("%s%s\n", str, prefix);
}


INTERNAL void 
interfaces_added_callback(struct l_dbus_message *reply_message, void *user_data)
{
  const char *error_name = NULL;
  const char *error_text = NULL;
  if (!l_dbus_message_get_error(reply_message, &error_name, &error_text))
  {
    const char *member = l_dbus_message_get_member(reply_message);

    struct l_dbus_message_iter root_dict_keys_iter, root_dict_values_iter = {0};
    const char *object = NULL;
    if (l_dbus_message_get_arguments(reply_message, "oa{sa{sv}}", &object, &root_dict_keys_iter))
    {
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
            }
            if (strcmp(device_dict_key, "RSSI") == 0)
            {
              s16 rssi = 0;
              l_dbus_message_iter_get_variant(&device_dict_values_iter, "n", &rssi);
            }
          }
        }
      }
    }
    else
    {
      BP_MSG("InterfacesAdded dict expected but not recieved");
    }
  }
  else
  {
    char error_message[128] = {0};
    snprintf(error_message, sizeof(error_message), "(DBUS ERROR): %s: %s", error_name, 
             error_text);
    BP_MSG(error_message);

    l_free(error_name);
    l_free(error_text);
  }


}

  //struct l_hashmap *hashmap = l_hashmap_string_new();

  //if (l_hashmap_insert(hashmap, "something", some_struct))
  //{

  //}

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


INTERNAL void 
dbus_request_name_callback(struct l_dbus *dbus_connection, bool success, bool queued, void *user_data)
{
  if (!success)
  {
    BP_MSG("Failed to acquire dbus name"); 
  }
}

INTERNAL void 
dbus_start_discovery_callback(struct l_dbus_message *reply_message, void *user_data)
{
  printf("Searching for unmanaged bluetooth devices...\n");
}

INTERNAL void 
dbus_stop_discovery_callback(struct l_dbus_message *reply_message, void *user_data)
{
  // printf("Searching for unmanaged bluetooth devices...\n");
}

GLOBAL b32 global_want_to_run = true;

INTERNAL void
falsify_global_want_to_run(int signum)
{
  global_want_to_run = false;
}

typedef struct BluetoothDevice
{
  char dbus_path[128];
  char address[128];
  s32 rssi;
} BluetoothDevice;

int main(int argc, char *argv[])
{
  if (l_main_init())
  {
    struct l_dbus *dbus_connection = l_dbus_new_default(L_DBUS_SYSTEM_BUS);
    if (dbus_connection != NULL)
    {
      signal(SIGINT, falsify_global_want_to_run);

      l_dbus_name_acquire(dbus_connection, "my.bluetooth.app", false, false, false, dbus_request_name_callback, NULL);
      
      int dbus_interfaces_added_id = l_dbus_add_signal_watch(dbus_connection, "org.bluez", "/", 
          "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", L_DBUS_MATCH_NONE, 
          dbus_interfaces_added_callback, NULL);
      // IMPORTANT(Ryan): InterfacesRemoved could be called during this discovery time
      if (dbus_interfaces_added_id != 0)
      {
        l_dbus_remove_signal_watch(dbus_connection, dbus_interfaces_added_id);
      }

      struct l_dbus_message *dbus_start_discovery_msg = l_dbus_message_new_method_call(dbus_connection, "org.bluez", "/org/bluez/hci0", 
                                                                                       "org.bluez.Adapter1", "StartDiscovery");
      if (dbus_start_discovery_msg != NULL)
      {

      }

      if (l_dbus_message_set_arguments(dbus_start_discovery_msg, ""))
      {
        if (l_dbus_send_with_reply(dbus_connection, dbus_start_discovery_msg, dbus_start_discovery_callback, NULL, NULL) != 0)
        {

        }
      }

      u64 start_time = get_ns();
      u64 discovery_time = SECONDS_NS(5);
      while (global_want_to_run)
      {
        if (get_ns() - start_time >= discovery_time)
        {
          struct l_dbus_message *dbus_stop_discovery_msg = l_dbus_message_new_method_call(dbus_connection, "org.bluez", "/org/bluez/hci0", 
                                                                                          "org.bluez.Adapter1", "StopDiscovery");
          l_dbus_message_set_arguments(dbus_stop_discovery_msg, "");
          l_dbus_send_with_reply(dbus_connection, dbus_stop_discovery_msg, dbus_stop_discovery_callback, NULL, NULL);
        }

        int timeout = l_main_prepare();
        l_main_iterate(timeout);
      }

      l_dbus_destroy(dbus_connection);
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
