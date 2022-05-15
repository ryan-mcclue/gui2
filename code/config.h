// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

// call when activate menu item
INTERNAL void
write_config(b32 val)
{
  char buf[256] = {0};
  char *at = buf;
  for (u32 variable_i = 0;
       variable_i < ARRAY_COUNT(variables);
       ++variable_i)
  {
    at += snprintf(at, sizeof(buf) - at, "#define %s %d // b32", var->name, var->value, var->type);
  }

  write_entire_file("../config.h", buf, sizeof(buf));

  execute_system_command("../code", "bash ../build");
  // FILE *file = popen("../code/build", "r");
  // if (file != NULL)
  // {
  //   if (fgets(temp, sizeof(temp), file) == NULL)
  //   {
  //      process has finished running, so close handle
  //      therefore, require check state function to be called every frame
  //      perhaps printf("compiling...\n")?
  //      s32 return_code = (s32)pclose(file);
  //   }
  // }
}

typedef enum DEBUG_VARIABLE_TYPE
{
  DEBUG_VARIABLE_TYPE_BOOL,
} DEBUG_VARIABLE_TYPE;

typedef struct Variable
{
  DEBUG_VARIABLE_TYPE type;
  char *name;
  union
  {
    b32 value;
  }
} Variable;

#define VARIABLE_LISTING(name) \
  {DEBUG_VARIABLE_TYPE_BOOL, #name, 1}

Variable variable_listing[] = {
  VARIABLE_LISTING(DEBUG_PROFILER),
};
