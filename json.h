#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

bool json_is_whitespace(char *);
void json_skip_whitespace(char **);

typedef enum {
  JSON_TYPE_OBJ,
  JSON_TYPE_ARR,
  JSON_TYPE_STR,
} JsonType;

typedef struct {
  char *start;
  size_t len;
} JsonStr;

typedef struct {
  JsonStr *key;
  void *value;
  JsonType value_type;
} JsonPair;

typedef struct {
  JsonPair *pairs;
  size_t len;
} JsonObj;

JsonObj json_parse_obj(char *);
JsonPair json_parse_pair(char **);
JsonStr json_parse_str(char **);
