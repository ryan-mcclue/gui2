// SPDX-License-Identifier: zlib-acknowledgement
#include "types.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
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
  #define BP_MSG(name, msg) __bp(__FILE__, __func__, __LINE__, msg)
  #define BP(name) __bp(__FILE__, __func__, __LINE__, "")
  #define EBP(name) __ebp(__FILE__, __func__, __LINE__)
  #define ASSERT(cond) if (!(cond)) {BP();}
#else
  #define BP_MSG(name, msg)
  #define BP(name)
  #define EBP(name)
  #define ASSERT(cond)
#endif

typedef struct NameAndMac
{
  char *name;
  char *mac;
} NameAndMac;

#define MAX_COMMAND_RESULT_COUNT 4096
typedef struct ReadEntireCommandResult
{
  char contents[MAX_COMMAND_RESULT_COUNT];
  u32 len;
} ReadEntireCommandResult;

INTERNAL ReadEntireCommandResult 
read_entire_command(char *command_str)
{
  ReadEntireCommandResult result = {0};

  int stdout_pair[2] = {0};

  if (pipe(stdout_pair) != -1)
  {
    // with forks, can also used shared memory...
    pid_t pid = vfork();
    if (pid != -1)
    {
      if (pid == 0)
      {
        dup2(stdout_pair[1], STDOUT_FILENO);
        close(stdout_pair[1]);
        close(stdout_pair[0]);

        execl("/bin/bash", "bash", "-c", command_str, NULL);

        EBP("Execl failed");
        exit(127);
      }
      else
      {
        wait(NULL);

        u32 bytes_read = read(stdout_pair[0], result.contents, MAX_COMMAND_RESULT_COUNT);
        if (bytes_read != -1)
        {
          result.len = bytes_read;
          result.contents[result.len] = '\0';
        }
        else
        {
          EBP("Reading from pipe failed");
        }

        close(stdout_pair[0]);
        close(stdout_pair[1]);
      }
    }
    else
    {
      close(stdout_pair[0]);
      close(stdout_pair[1]);
      EBP("Forking failed");
    }
  }
  else
  {
    EBP("Creating pipes failed");
  }

  return result;
}

int 
main(int argc, char *argv[])
{
  NameAndMac name_and_macs[4] = {0};
  name_and_macs[0].name = "Ryan";
  name_and_macs[0].mac = "5c:03:39:c5:b8:c9";
  name_and_macs[1].name = "Glen";
  name_and_macs[1].mac = "";
  name_and_macs[2].name = "Jennifer";
  name_and_macs[2].mac = "";
  name_and_macs[3].name = "Lachlan";
// d8:4c:90:0e:59:da
// 82:C4:D8:67:64:46
  name_and_macs[3].mac = "";

  // IMPORTANT(Ryan): If serving images, will have to handle their specific GET requests
  char html[] = {
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n\r\n"
    "<style> body { background-color: #efefef; } </style>\r\n"
    "<h1> Hi There! </h1>\r\n"
  };

  ReadEntireCommandResult read_ping_command_result = \
    read_entire_command("fping --generate 192.168.0.0/24 --retry=1 --alive --quiet");

  ReadEntireCommandResult read_arp_command_result = \
    read_entire_command("arp -n | awk '{ if (NR>1) print $1, $3}'");
  

#if 0
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
            read_from_client();
            if (client_msg == "GET /favicon.ico")
            {
              img = open("favicon.ico");
              sendfile(client, img, size_of_image);
            }

            char send_buf[2048] = {0};
            snprintf(send_buf, sizeof(send_buf), "%s", html);

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

GET /tutorials/other/top-20-mysql-best-practices/ HTTP/1.1
char    *method,    // "GET" or "POST"
        *uri,       // "/index.html" things before '?'
        *qs,        // "a=1&b=2"     things after  '?'
        *prot;      // "HTTP/1.1"

char    *payload;     // for POST
int      payload_size;

  clients[slot] = accept();
  if (fork() == 0)
  {
    respond(slot);
    .....
    buf = malloc(65535);
    rcvd=recv(clients[n], buf, 65535, 0);
    parsing.... 


    strcmp("/", uri) == 0 && strcmp("GET", method) == 0

    .....
    exit(0);
  }

  slot = (slot + 1) % max;
#endif


  return 0;
}
