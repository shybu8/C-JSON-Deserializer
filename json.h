#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef enum {
  JSON_TYPE_OBJ,
  JSON_TYPE_ARR,
  JSON_TYPE_STR,
  JSON_TYPE_INT,
  JSON_TYPE_FRC,
  JSON_TYPE_BOL,
  JSON_TYPE_NUL,
} JsonType;

typedef struct JsonStr JsonStr;
typedef struct JsonObj JsonObj;
typedef struct JsonArr JsonArr;
typedef struct JsonVal JsonVal;
typedef struct JsonPair JsonPair;

struct JsonStr {
  char *start;
  size_t len;
  bool needs_dealloc;
};

struct JsonVal {
  union {
    JsonObj *obj_ptr;
    JsonArr *arr_ptr;
    JsonStr *str_ptr;
    long long integer;
    double fract;
    bool boolean;
  } as;
  JsonType type;
};

struct JsonPair {
  JsonStr key;
  JsonVal value;
};

struct JsonObj {
  JsonPair *pairs;
  size_t len;
};

struct JsonArr {
  JsonVal *values;
  size_t len;
};

bool json_parse_val(JsonVal *, const char **);
void json_free_val(JsonVal *);
bool json_decode_str(const char **, size_t *, char *, size_t);

JsonVal *json_value_by_key(JsonObj *, char *);

typedef struct {
  bool minimal;
  size_t indentation_level;
  char *indentation_str;
} JsonStyle;

#define JSON_STYLE_MINIMAL                                                     \
  {                                                                            \
      .minimal = true,                                                         \
      .indentation_level = 0,                                                  \
      .indentation_str = "",                                                   \
  }

#define JSON_STYLE_PRETTY_PRINT_TABS                                           \
  {                                                                            \
      .minimal = false,                                                        \
      .indentation_level = 0,                                                  \
      .indentation_str = "\t",                                                 \
  }

#define JSON_STYLE_PRETTY_PRINT_DOUBLESPACES                                   \
  {                                                                            \
      .minimal = false,                                                        \
      .indentation_level = 0,                                                  \
      .indentation_str = "  ",                                                 \
  }

#define JSON_STYLE_PRETTY_PRINT_FOURSPACES                                     \
  {                                                                            \
      .minimal = false,                                                        \
      .indentation_level = 0,                                                  \
      .indentation_str = "    ",                                               \
  }

void json_serialize_val(JsonVal *, char **, size_t *, size_t *,
                        JsonStyle *style);

bool json_str_needs_encoding(const char *str, size_t *res_buf_size);

void json_str_encode_into_buf(const char *str, char *buf);
