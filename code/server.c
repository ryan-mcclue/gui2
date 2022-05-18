// SPDX-License-Identifier: zlib-acknowledgement
#include "types.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

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

typedef struct NameAndMac
{
  char *name;
  char *mac;
} NameAndMac;

typedef struct ReadEntireCommandResult
{
  char *contents;
  u32 size;
  u32 len;
} ReadEntireCommandResult;

INTERNAL ReadEntireCommandResult 
read_entire_command(char *command_str, u32 buf_size)
{
  ReadEntireCommandResult result = {0};
  result.contents = calloc(1, buf_size);
  if (result.contents != NULL)
  {
    // have to have pclose() to wait for it to finish
    FILE *command = popen(command_str, "r");
    if (command != NULL)
    {
      while (fread(result.contents
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


  return result;
}

int 
main(int argc, char *argv[])
{
  NameAndMac name_and_macs[4] = {0};
  name_and_macs[0].name = "Ryan";
  name_and_macs[0].mac = "";
  name_and_macs[0].name = "Glen";
  name_and_macs[0].mac = "";
  name_and_macs[0].name = "Jennifer";
  name_and_macs[0].mac = "";
  name_and_macs[0].name = "Lachlan";
  name_and_macs[0].mac = "";

  // IMPORTANT(Ryan): If serving images, will have to handle their specific GET requests
  char html[1024] = {
    "<h1> Hi There! </h1>"
  };

  ReadCommandResult read_ping_command_result = read_entire_command("fping --generate 192.168.0.0/24 --retry=1 --alive --quiet");
  ReadCommandResult read_arp_command_result = read_entire_command("arp -n | awk '{ if (NR>1) print $1, $3}'");

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd != -1)
  {
    int opt_val = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt_val, sizeof(opt_val)) == -1)
    {
      EBP();
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(18000); // 80 might conflict with OS?

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != -1)
    {
      u32 max_num_connections = 10;
      if (listen(server_fd, max_num_connections) != -1)
      {
        while (true)
        {
          struct sockaddr_in client_addr = {0}; 
          u32 client_size = sizeof(client_addr);
          int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_size);
          if (client_fd != -1)
          {
            char send_buf[2048] = {0};
            snprintf(send_buf, sizeof(send_buf), "HTTP/1.0 200 OK\r\n\r\n%s", html);

            write(client_fd, send_buf, sizeof(send_buf));

            close(client_fd);
          }
          else
          {
            EBP();
          }
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


  return 0;
}
