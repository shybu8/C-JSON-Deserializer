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

bool json_parse_obj(JsonObj **, char **);
void json_free_obj(JsonObj *);
bool json_decode_str(char **, size_t *, char *, size_t);
