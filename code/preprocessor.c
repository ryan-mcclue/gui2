// SPDX-License-Identifier: zlib-acknowledgement

/* thing_to_preprocess.h
 
INTROSPECT(category:"something") typedef struct Variable
{
  
} Variable;


 */


// method of annotating code
#define INTROSPECT(params)

typedef enum TokenType {
	// reserve first 128 values for one char tokens
	TOKEN_LAST_CHAR = 127,
	TOKEN_INT,
	TOKEN_NAME
	// ..
} TokenType;

void next_token(void)
{
	token.start = stream;
	switch (*stream) {
	 case 0 ... 9:
	 {
		int val = 0;
		while (isdigit(*stream)) {
			val *= 10;
			val += *stream++ - '0'; // convert ascii to number
		}
		token.type = TOKEN_INT;
		token.val = val;
	 } break;
	 case 'a' ... 'z':
   case 'A' ... 'Z':
	 case '_':
	 {
		while (isalnum(*stream) || *stream == '_') {
			stream++;		
		}
		token.type = TOKEN_NAME;
		token.name = str_intern_range(token.start, stream);
	 } break;
	 default:
	 {
		token.type = *stream++;
	 } break;
	}	
	token.end = stream;
}

INTERNAL void
eat_all_whitespace_and_comments(char *stream)
{
  char *at = stream;
  while (true)
  {
    // could be \t, \n, \r etc.
    if (isspace(at[0]))
    {
      at++;
    }
    else if (at[0] == '/' && at[1] == '/')
    {
      at += 2;
      while (at[0] != '\0' && at[0] != '\n' && at[0] != '\r')
      {
        at++;
      }
    }
    else if (at[0] == '/' && at[1] == '*')
    {
      while (at[0] != '\0' && at[0] != '*' && at[1] != '\0' && at[1] != '/')
      {
        at++;
      }
    }
    else
    {
      break;
    }
  }
}

INTERNAL Token
get_token(char *stream)
{
  eat_all_whitespace_and_comments(stream);

  char *at = stream;

  Token result = {0};
  result.len = 1;
  result.text = at;

  switch (at[0])
  {
    case '\0': 
    {
      result.type = TOKEN_TYPE_EOS;
    } break;
    case '/':
    {

    } break;
    case '{': 
    {
      result.type = TOKEN_TYPE_OPEN_BRACKET;
      at++;
    } break;
    case '}': 
    {
      result.type = TOKEN_TYPE_CLOSE_BRACKET;
    } break;
    case '"': 
    {
      at++;
      result.text = at[0];
      while (at[0] != '\0' && at[0] != '"')
      {
        if (at[0] == '\\' && at[1] != '\0')
        {
          at++;
          result.len++;
        }
        at++;
        result.len++;
      }

      if (at[0] == '"')
      {
        at++;
      }
    } break;
    default
    {
      token.type = TOKEN_TYPE_UNKNOWN;
      if (isalpha(at[0]))
      {
        parse_identifier();
        while (isalpha(at[0]) || isnumeric(at[0]) || at[0] == '_')
      }
      else
      {
        parse_number();
      }
    } break;
  }

  return result;
}

INTERNAL b32
token_equals(Token *token, char *test)
{
  b32 result = false;

  char *token_at = token->text;

  char *test_at = test;
  u32 test_at_index = 0;
  while (test_at[test_at_index] != '\0')
  {
    if (test_at[test_at_index] != token_at[test_at_index])
    {
      break;
    }
    test_at_index++;
  }

  if (test_at[test_at_index] == '\0' &&
      token_at[test_at_index] == '\0')
  {
    result = true;
  }

  return result;
}

INTERNAL void
parse_instrospectable(char *stream)
{
  Token token = consume_next_token(stream);
  if (token_equals(&token, "struct"))
  {
    if (require_token(stream, TOKEN_TYPE_OPEN_BRACKET))
    {
      Token name_token = consume_next_token(stream);
      printf("Members of: %.*s = \n", name_token.len, name_token.name);
      // while(parsing) variant over != for enclosing parsing?
      // != for tokens? and while(true) for others to give better error handling?
      while (parsing)
      {
        Token new_token = consume_next_token(stream);
        if (token_equals(&new_token, TOKEN_TYPE_CLOSE_BRACKET))
        {
          break;
        }
        else
        {
          parse_member(stream, struct_token, type_token);
          // want offset into struct: offsetof(struct_token, member_token); 
        }
      }
    }
  }
  else
  {
    fprintf(stderr, "only parse structs");
  }

}

/* ALMOST EVERYTHING SHOULD BE METAPROGRAMMED?
 * meta.h
 * enum StructMemberType {
 *   type_u32,
 *   type_r32,
 * };
 * 
 * struct StructMember {
 *   Type type;
 *   char *name;
 *   u32 offset;
 * };
 
  printf("#define META_HANDLE_PRINT_TYPE(member_ptr, indent_level) \\\n")
  for (structs_parsed)
  {
    printf("case: meta_type%s: {dump_struct(indent_level + 1)}",
    struct->next != NULL ? "\\" : "");
  }

  StructMember members_of_Name[] = {
   "{type_u32(some enum), name, offsetof}"
  } > generated.h

  Instrospectable introspectable;
  introspectable.name = 10;
  introspectable.name2 = true;
  for (u32 intro_i = 0;
       intro_i < ARRAY_COUNT(members_of_Introspectable);
       ++intro_i)
  {
    switch (variables[intro_i].type) {
    case some_struct: dump_introspectable_struct(some_struct);
    (only have to write code for scalars)
    }
  }

  void dump_introspectable_struct(StructMember *members, u32 member_count, void *struct);

  

  char text_buffer_base[256];
  char *text_buffer = text_buffer_base;
  for (u32 indent = 0;
       indent < indent_level;
       ++indent)
  {
    *text_buffer++ = ' ';
    *text_buffer++ = ' ';
    *text_buffer++ = ' ';
    *text_buffer++ = ' ';
  }
  *text_buffer = '\0';
*/

int
main(int argc, char *argv[])
{
  ReadFileResult read_result = read_entire_file_and_null_terminate("thing_to_preprocess.h");
  char *contents = (char *)read_result.mem;

  char *tokeniser_stream = contents;

  b32 want_to_parse = true;
  while (want_to_parse)
  {
    Token token = consume_next_token(tokeniser_stream);

    switch (token.type)
    {
      case TOKEN_TYPE_EOS:
      {
        want_to_parse = false;
      } break;
      case TOKEN_TYPE_INT:
      {
        print_token_type();
      } break;
      case TOKEN_TYPE_IDENTIFIER:
      {
        if (token_equals(&token, "INTROSPECT"))
        {
          parse_instrospectable(at);
        }
      } break;
    }
  }

  return 0;
}
