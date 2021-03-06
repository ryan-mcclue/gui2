// SPDX-License-Identifier: zlib-acknowledgement
#include "types.h"

#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


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

typedef struct Camera
{
  int fd;
  u8 *buffer;
  u32 buffer_size;
  u32 buffer_len;
} Camera;

INTERNAL Camera
camera_init(const char *camera_path, u32 aperture_width, u32 aperture_height)
{
  Camera result = {0};

  result.fd = open(camera_path, O_RDWR);
  if (result.fd >= 0)
  {
    struct v4l2_format camera_format = {0};
    camera_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    camera_format.fmt.pix.width = aperture_width;
    camera_format.fmt.pix.height = aperture_height;
    // NOTE(Ryan): Determine with $(v4l2-ctl --list-formats-ext) 
    camera_format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    camera_format.fmt.pix.field = V4L2_FIELD_NONE;
    int camera_format_status = ioctl(result.fd, VIDIOC_S_FMT, &camera_format);
    if (camera_format_status >= 0)
    {
      struct v4l2_requestbuffers camera_buffer_request = {0};
      camera_buffer_request.count = 1;
      camera_buffer_request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      camera_buffer_request.memory = V4L2_MEMORY_MMAP;
      int camera_buffer_request_status = ioctl(result.fd, VIDIOC_REQBUFS, 
                                               &camera_buffer_request);
      if (camera_buffer_request_status >= 0)
      {
        struct v4l2_buffer camera_buffer_info = {0};
        camera_buffer_info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        camera_buffer_info.memory = V4L2_MEMORY_MMAP;
        camera_buffer_info.index = 0;
        int camera_buffer_info_status = ioctl(result.fd, VIDIOC_QUERYBUF, &camera_buffer_info);
        if (camera_buffer_info_status >= 0)
        {
          result.buffer = mmap(NULL, camera_buffer_info.length, PROT_READ | PROT_WRITE, 
                               MAP_SHARED, result.fd, camera_buffer_info.m.offset);
          if (result.buffer != NULL)
          {
            result.buffer_size = camera_buffer_info.length;

            int camera_streamon_status = ioctl(result.fd, VIDIOC_STREAMON, 
                                               &camera_buffer_info.type);
            if (camera_streamon_status == -1)
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

  return result;
}

INTERNAL void
camera_take_picture(Camera *camera)
{
  struct v4l2_buffer camera_capture_buffer = {0};
  camera_capture_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  camera_capture_buffer.memory = V4L2_MEMORY_MMAP;
  camera_capture_buffer.index = 0;
  int camera_query_capture_buffer_status = ioctl(camera->fd, VIDIOC_QBUF, 
                                                 &camera_capture_buffer);
  if (camera_query_capture_buffer_status >= 0)
  {
    fd_set camera_fd_set;
    FD_ZERO(&camera_fd_set);
    FD_SET(camera->fd, &camera_fd_set);

    struct timeval camera_wait_for_capture_frame_time = {0};
    camera_wait_for_capture_frame_time.tv_sec = 2;
    int num_returned_fds = select(camera->fd + 1, &camera_fd_set, NULL, NULL, 
                                  &camera_wait_for_capture_frame_time);
    if (num_returned_fds > 0)
    {
      int camera_dequeue_capture_buffer_status = ioctl(camera->fd, VIDIOC_DQBUF, 
                                                       &camera_capture_buffer);
      if (camera_dequeue_capture_buffer_status >= 0)
      {
        camera->buffer_len = camera_capture_buffer.bytesused;
      }
      else
      {
        EBP();
      }
    }
    else if (num_returned_fds == -1)
    {
      EBP();
    }
  }
  else
  {
    EBP();
  }
  //int webcam_streamoff_status = ioctl(webcam_fd, VIDIOC_STREAMOFF, &webcam_buffer_info.type);
  //ASSERT(webcam_streamoff_status != -1);
}

#define MAX_COMMAND_RESULT_COUNT 4096
INTERNAL char * 
read_entire_command(char *command_str)
{
  char *result = calloc(MAX_COMMAND_RESULT_COUNT, 1);
  if (result != NULL)
  {
    int stdout_pair[2] = {0};

    if (pipe(stdout_pair) != -1)
    {
      // NOTE(Ryan): With forks, can also used shared memory...
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

          u32 bytes_read = read(stdout_pair[0], result, MAX_COMMAND_RESULT_COUNT);
          if (bytes_read != -1)
          {
            result[bytes_read] = '\0';
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
  }
  else
  {
    EBP("Calloc failed");
  }


  return result;
}

// NOTE(Ryan):
//  GET /path?query_string HTTP/1.0\r\n
//  \r\n
//  ....
//  POST /path HTTP/1.0\r\n
//  Content-Type: text/plain\r\n
//  Content-Length: 12\r\n
//  \r\n
//  query_string
typedef struct HTTPRequestInfo
{
  b32 is_get; 
  char uri[64];
  u8 *payload;
  u32 payload_size;
} HTTPRequestInfo;

INTERNAL void
consume_whitespace(char **at)
{
  while (isspace((*at)[0]))
  {
    (*at)++;
  }
}

INTERNAL u32
consume_identifier(char **at)
{
  u32 result = 0;

  while (isalnum((*at)[0]) || (*at)[0] == ':' || (*at)[0] == '-' || (*at)[0] == '/' ||
         (*at)[0] == '.')
  {
    (*at)++;
    result++;
  }

  return result;
}

INTERNAL u32
consume_value(char **at)
{
  u32 result = 0;

  while ((*at)[0] != '\r' && ((*at)[1] != '\0' && (*at)[1] != '\n'))
  {
    (*at)++;
    result++;
  }

  return result;
}

INTERNAL b32
end_of_header(char **at)
{
  b32 result = false;

  result = ((*at)[0] == '\r' && 
           ((*at)[1] != '\0' && (*at)[1] == '\n') &&
           ((*at)[2] != '\0' && (*at)[2] == '\r') &&
           ((*at)[3] != '\0' && (*at)[3] == '\n'));

  return result;
}

INTERNAL HTTPRequestInfo
get_http_request_info(int sock_fd)
{
  HTTPRequestInfo result = {0};

  char tmp_buf[4096] = {0};
  int bytes_read = read(sock_fd, tmp_buf, sizeof(tmp_buf));
  if (bytes_read != -1)
  { 
    tmp_buf[bytes_read] = '\0';

    char *at = tmp_buf;
    
    consume_whitespace(&at);
    char *method = at; 
    u32 method_len = consume_identifier(&at);
    result.is_get = (strncmp(method, "GET", method_len) == 0);
    if (!result.is_get)
    {
      ASSERT(strncmp(method, "POST", method_len) == 0);
    }

    consume_whitespace(&at);
    char *uri = at; 
    u32 uri_len = consume_identifier(&at);
    memcpy(result.uri, uri, uri_len + 1);
    result.uri[uri_len] = '\0';

    consume_whitespace(&at);
    char *protocol = at; 
    u32 protocol_len = consume_identifier(&at);

    //printf("Method: %.*s, URI: %.*s, Protocol: %.*s\n", method_len, method, uri_len, uri, 
    //                                                    protocol_len, protocol);

    while (true)
    {
      if (end_of_header(&at))
      {
        result.payload = at + 4;
        break;
      }

      consume_whitespace(&at);
      char *key = at;
      u32 key_len = consume_identifier(&at);
      consume_whitespace(&at);
      char *value = at;
      u32 value_len = consume_value(&at);

      //printf("Key: %.*s, Value: %.*s\n", key_len, key, value_len, value);

      if (strncmp(key, "Content-Length:", key_len) == 0)
      {
        char *value_at = value;
        while (isdigit(value_at[0]))
        {
          result.payload_size *= 10; 
          result.payload_size = value_at[0] - '0';
          value_at++;
        }
      }
    }

  }
  else
  {
    EBP("Read failed");
  }
  

  return result;
}

int 
main(int argc, char *argv[])
{
  char html[] = {
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n\r\n"
    "<style> body { background-color: #efefef; } </style>\r\n"
    "<h1> Hi There! </h1>\r\n"
    "<img src='camera.jpeg' />\r\n"
    "<form method='post'>\r\n"
    "  <button name='LED1' value='1'> LED ON </button>\r\n"
    "  <button name='LED2' value='0'> LED OFF </button>\r\n"
    "</form>\r\n"
  };

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd != -1)
  {
    int opt_val = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt_val, sizeof(opt_val)) == -1)
    {
      EBP();
    }

    u32 server_port = 18000;
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != -1)
    {
      u32 max_num_connections = 1;
      if (listen(server_fd, max_num_connections) != -1)
      {
        while (true)
        {
          struct sockaddr_in client_addr = {0}; 
          u32 client_size = sizeof(client_addr);
          int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_size);
          if (client_fd != -1)
          {
            HTTPRequestInfo request_info = get_http_request_info(client_fd); 

            if (request_info.is_get)
            {
              if (strcmp(request_info.uri, "/") == 0)
              {
                char send_buf[2048] = {0};
                snprintf(send_buf, sizeof(send_buf), "%s", html);
                write(client_fd, send_buf, sizeof(send_buf));
                close(client_fd);
              }
              else if (strcmp(request_info.uri, "/camera.jpeg") == 0)
              {
                Camera camera = camera_init("/dev/video0", 1280, 720);
                ASSERT(camera.fd != -1);

                char camera_multipart_header[] = {
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: multipart/x-mixed-replace; boundary=myboundary\r\n\r\n"
                };
                write(client_fd, camera_multipart_header, sizeof(camera_multipart_header));

                u32 camera_picture_boundary_size = camera.buffer_size + 256;
                char *camera_picture_boundary = malloc(camera_picture_boundary_size);
                ASSERT(camera_picture_boundary != NULL);

                while (true)
                {
                  camera_take_picture(&camera);

                  snprintf(camera_picture_boundary, camera_picture_boundary_size,
                  "--myboundary\r\nContent-Type: image/jpeg\r\nContent-length: %d\r\n\r\n",
                  camera.buffer_len);

                  u32 camera_picture_boundary_header_size = strlen(camera_picture_boundary);

                  memcpy(camera_picture_boundary + camera_picture_boundary_header_size,
                         camera.buffer, camera.buffer_len);
                  
                  write(client_fd, camera_picture_boundary, 
                        camera_picture_boundary_header_size + camera.buffer_len);
                }
              }
            }
            else
            {
              if (strcmp(request_info.uri, "/") == 0)
              {
                printf("%.*s\n", request_info.payload_size, request_info.payload);
                char send_buf[2048] = {0};
                snprintf(send_buf, sizeof(send_buf), "%s", html);
                write(client_fd, send_buf, sizeof(send_buf));
                close(client_fd);
              }
            }

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

// relay converts low voltage to high?
// can handle higher currents that a transistor

// extension cable (so has male and female connectors) has ground, netural, high
// just wire up the high wire to the relay (however mains, so dangerous...)

// optoisolator in effect just a simple transistor?
#if 0
void
route("/")
{
  int fd = open("index.html", O_RDONLY);
  if (fd == -1)
  {
    send_404();
    sizeof("LED1=");
    atoi(ptr) ? "on" : "off"
  }
  else
  {

  }
}
#endif
