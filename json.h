#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
  JSON_TYPE_OBJ,
  JSON_TYPE_ARR,
  JSON_TYPE_STR,
  JSON_TYPE_INT,
  JSON_TYPE_DBL,
  JSON_TYPE_BOL,
  JSON_TYPE_NUL,
} JsonType;

typedef struct {
  char *start;
  size_t len;
} JsonStr;

typedef struct {
  void *ptr;
  JsonType type;
} JsonVal;

typedef struct {
  JsonStr key;
  JsonVal value;
} JsonPair;

typedef struct {
  JsonPair *pairs;
  size_t len;
} JsonObj;

typedef struct {
  JsonVal *values;
  size_t len;
} JsonArr;

void json_parse_obj(JsonObj **, char **);
void json_free_obj(JsonObj *);
