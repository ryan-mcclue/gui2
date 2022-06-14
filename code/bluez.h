// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#define MAX_DBUS_METHOD_STR_LEN 128

typedef void (* dbus_method_callback_t)(struct l_dbus_message *message);

typedef struct DBusMethod
{
  struct l_dbus *connection;
  char service[MAX_DBUS_METHOD_STR_LEN];
  char object[MAX_DBUS_METHOD_STR_LEN];
  char interface[MAX_DBUS_METHOD_STR_LEN];
  char method[MAX_DBUS_METHOD_STR_LEN];
  struct l_dbus_message *message;
  dbus_method_callback_t callback;
} DBusMethod;
