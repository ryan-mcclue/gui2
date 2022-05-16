// SPDX-License-Identifier: zlib-acknowledgement

// RFCOMM protocol. 
// Serial Port Profile is based on RFCOMM protocol
// profile will have a UUID

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
on_adapter_changed(GDBusConnection *conn,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data)
{
  const char *adapter_path = adapter_info.object_path;

  if (g_strcmp0(object_path, adapter_path) == 0)
    return;

  if (g_list_length(devices) > 10)
    return;

  if (object_path && strstr(object_path, adapter_path) != NULL)
  {
    GList *l;
    gboolean found = FALSE;

    for (l = devices; l != NULL; l = l->next)
    {
      if (g_strcmp0(l->data, object_path) == 0) {
        found = TRUE;
	break;
      }
    }

    if (found == FALSE)
    {
      devices = g_list_prepend(devices, g_strdup(object_path));
    }
  }
}

int 
main(int argc, char *argv[])
{
  // TODO(Ryan): Way to programmatically obtain this?
  //int bluetooth_fd = obtain_serial_connection("/dev/ttyACM0");
  //close(bluetooth_fd);

  int dev_id = hci_get_route(NULL);
  if (dev_id >= 0)
  {
    struct hci_dev_info hci_info = {0};
    if (hci_devinfo(dev_id, &hci_info) >= 0)
    {
      char *bus_name = "org.bluez";
      char *interface_name = "org.bluez.Adapter1";

      char object_path[256] = {"/org/bluez/"};
      strcat(object_path, hci_info.name);

      printf("object path: %s\n", object_path);
      
     /* estsablish a connection to D-Bus which in this case must be a system ty-
     pe. Accession the session bus results in bluez path's being unknown.
     Thus, you might want to adjust your DBus permission policy */
      GError *g_error = NULL;
      GDBusConnection *dbus_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &g_error);
      if (dbus_connection != NULL)
      {
        start_device_discovery();
        wait_sec(10);
        stop_device_discovery();

        char *selected_device = select_from_devices();

        connect_device(selected_device);

        disconnect_device(selected_device);

        // have a max timeout for scanning
        u32 scan_subscription_id = g_dbus_connection_signal_subscribe(dbus_connection,
                                              "org.bluez",
                                              "org.freedesktop.DBus.Properties",
                                              "PropertiesChanged",
                                              NULL,
                                              NULL,
                                              G_DBUS_SIGNAL_FLAGS_NONE,
                                              on_adapter_changed,
                                              NULL,
                                              NULL);

         // method call
         GVariant *res = g_dbus_connection_call_sync(dbus_connection,
               bus_name,
               object_path,
               interface_name,
               "StartDiscovery",
               NULL,
               NULL,
               G_DBUS_CALL_FLAGS_NONE,
               -1,
               NULL,
               &g_error);

         g_variant_unref(res);

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

  return 0;
}

// sudo apt install raspberrypi-kernel-headers
