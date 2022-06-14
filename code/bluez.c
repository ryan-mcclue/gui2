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
    (DBusMethod *)user_data->callback(reply_message);
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

  result.msg = l_dbus_message_new_method_call(result.conn, result.service, result.object, 
                                              result.interface, result.method);

  result.callback = callback;

  return result; 
}

INTERNAL void
call_dbus_method(DBusMethod *method)
{
  l_dbus_send_with_reply(method->connection, method->message, dbus_callback_wrapper, 
                         method->callback, NULL);

  l_dbus_message_unref(method->msg);
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
interfaces_added_callback(struct l_dbus_message *message, void *user_data)
{
  const char *unique_device_path = l_dbus_message_get_path(message);
  printf("(SIGNAL RECIEVED) %s\n", unique_device_path);

  //struct l_hashmap *hashmap = l_hashmap_string_new();

  //if (l_hashmap_insert(hashmap, "something", some_struct))
  //{

  //}

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

INTERNAL void
start_discovery_callback(struct l_dbus_message *reply_message)
{
  printf("Starting discovery\n");
}

INTERNAL void
stop_discovery_callback(struct l_dbus_message *reply_message)
{
  global_want_to_run = false;
}

INTERNAL void 
request_dbus_name_callback(struct l_dbus *dbus, bool success, bool queued, void *user_data)
{
  if (!success)
  {
    BP_MSG("Failed to acquire dbus name"); 
  }
}

INTERNAL void
dbus(void)
{
  if (l_main_init())
  {
    struct l_dbus *dbus_conn = l_dbus_new_default(L_DBUS_SYSTEM_BUS);
    if (dbus_conn != NULL)
    {
      signal(SIGINT, falsify_global_want_to_run);

      l_dbus_name_acquire(dbus_conn, "my.bluetooth.app", false, false, false, 
                          request_dbus_name_callback, NULL);

      l_dbus_add_signal_watch(dbus_conn, "org.freedesktop.DBus.ObjectManager", 
          "org.bluez", "InterfacesAdded", L_DBUS_MATCH_NONE, 
          dbus_callback_wrapper, interfaces_added_callback);

      // IMPORTANT(Ryan): More elegant to obtain adapter object from GetManagedObjects()
      // However, this is the default in most circumstances 
      DBusMethod start_discovery = create_dbus_method("org.bluez", "/org/bluez/hci0", 
                                                      "org.bluez.Adapter1", "StartDiscovery",
                                                      start_discovery_callback);

      DBusMethod stop_discovery = create_dbus_method("org.bluez", "/org/bluez/hci0", 
                                                     "org.bluez.Adapter1", "StopDiscovery",
                                                     stop_discovery_callback);
      call_dbus_method(&start_discovery);

      u64 start_ns = get_ns();

      int ell_main_loop_timeout = l_main_prepare();
      while (global_want_to_run)
      {
        if ((get_ns() - start_ns) >= SECONDS_NS(5))
        {
          call_dbus_method(&stop_discovery);
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
}
