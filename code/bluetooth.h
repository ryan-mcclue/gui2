// SPDX-License-Identifier: zlib-acknowledgement
#define MAX_DBUS_METHOD_STR_LEN 128

typedef (void *dbus_method_callback_t)(struct l_dbus_message *message);

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
    (dbus_method_callback_t)user_data(reply_message);
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

