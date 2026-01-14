#pragma once
#include <assert.h>
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

typedef struct {
  char *start;
  size_t len;
} JsonStr;

typedef struct JsonObj JsonObj;
typedef struct JsonArr JsonArr;

typedef struct {
  // void *ptr;
  union {
    JsonObj *obj_ptr;
    JsonArr *arr_ptr;
    JsonStr *str_ptr;
    long long integer;
    double fract;
    bool boolean;
  } as;
  JsonType type;
} JsonVal;

typedef struct {
  JsonStr key;
  JsonVal value;
} JsonPair;

struct JsonObj {
  JsonPair *pairs;
  size_t len;
};

struct JsonArr {
  JsonVal *values;
  size_t len;
};

void json_parse_obj(JsonObj **, char **);
void json_free_obj(JsonObj *);
