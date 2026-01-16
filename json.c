#include "json.h"
#include "stdlib.h"
#include <ctype.h>
#include <string.h>

static char TRUE_STR[] = "true";
static char FALSE_STR[] = "false";
static char NULL_STR[] = "null";

static bool json_parse_pair(JsonPair *, char **);
static bool json_parse_str(JsonStr *, char **);
static bool json_parse_arr(JsonArr **, char **);
static bool json_parse_val(JsonVal *, char **);

static void json_skip_whitespace(char **ptr) {
  while (isspace((unsigned char)**ptr))
    (*ptr)++;
}

bool json_parse_obj(JsonObj **res, char **text) {
  *res = malloc(sizeof(JsonObj));
  (*res)->pairs = NULL;
  (*res)->len = 0;
  size_t actual_len = 2;
  (*res)->pairs = malloc(sizeof(JsonPair) * actual_len);

  if (**text != '{')
    return false;

  (*text)++;
  json_skip_whitespace(text);
  if (**text != '}')
    for (;;) {
      if ((*res)->len == actual_len) {
        (*res)->pairs =
            realloc((*res)->pairs, sizeof(JsonPair) * (actual_len * 2));
        actual_len *= 2;
      }

      (*res)->len += 1;
      if (!json_parse_pair(&(*res)->pairs[(*res)->len - 1], text))
        return false;

      json_skip_whitespace(text);
      if (**text == ',') {
        (*text)++;
        json_skip_whitespace(text);
      } else
        break;
    }
  json_skip_whitespace(text);

  if (**text != '}')
    return false;

  (*text)++;
  return true;
}

static bool json_parse_str(JsonStr *str, char **text) {
  if (**text != '"')
    return false;

  str->start = ++*text;
  bool esc = false;
  bool needs_decoding = false;
  for (;; (*text)++) {
    if (**text == '\0')
      return false;
    if (!esc) {
      if (**text == '"')
        break;
      esc = **text == '\\';
    } else
      esc = false;
  }
  str->len = *text - str->start;

  if (**text != '"')
    return false;

  (*text)++;
  return true;
}

static bool json_parse_pair(JsonPair *res, char **text) {
  if (!json_parse_str(&res->key, text))
    return false;

  json_skip_whitespace(text);

  if (**text != ':')
    return false;

  (*text)++;
  json_skip_whitespace(text);

  if (!json_parse_val(&res->value, text))
    return false;
  return true;
}

static bool json_parse_arr(JsonArr **res, char **text) {
  *res = malloc(sizeof(JsonArr));
  (*res)->values = NULL;
  (*res)->len = 0;
  size_t actual_len = 2;
  (*res)->values = malloc(sizeof(JsonVal) * actual_len);

  if (**text != '[')
    return false;

  (*text)++;
  json_skip_whitespace(text);
  if (**text != ']')
    for (;;) {
      if ((*res)->len == actual_len) {
        (*res)->values =
            realloc((*res)->values, sizeof(JsonVal) * (actual_len * 2));
        actual_len *= 2;
      }

      (*res)->len += 1;
      if (!json_parse_val(&(*res)->values[(*res)->len - 1], text))
        return false;

      if (**text == ',') {
        (*text)++;
        json_skip_whitespace(text);
      } else
        break;
    }
  json_skip_whitespace(text);

  if (**text != ']')
    return false;

  (*text)++;
  return true;
}

static bool is_fractional(char *num) {
  if (*num == '-')
    num++;
  while (isdigit((unsigned char)*num))
    num++;
  return *num == '.' && isdigit((unsigned char)*(++num));
}

static bool json_parse_val(JsonVal *res, char **text) {
  if (**text == '"') {
    res->as.str_ptr = malloc(sizeof(JsonStr));
    res->type = JSON_TYPE_STR;
    if (!json_parse_str(res->as.str_ptr, text)) {
      free(res->as.str_ptr);
      return false;
    }
  } else if (**text == '{') {
    res->type = JSON_TYPE_OBJ;
    if (!json_parse_obj(&res->as.obj_ptr, text))
      return false;
  } else if (**text == '[') {
    res->type = JSON_TYPE_ARR;
    if (!json_parse_arr(&res->as.arr_ptr, text))
      return false;
  } else if (isdigit((unsigned char)**text) ||
             (**text == '-' && isdigit((unsigned char)*(*text + 1)))) {
    char *end;
    if (is_fractional(*text)) {
      // Double (maybe exponential)
      res->as.fract = strtod(*text, &end);
      res->type = JSON_TYPE_FRC;
    } else {
      // Integer
      res->as.integer = strtoll(*text, &end, 10);
      res->type = JSON_TYPE_INT;
    }
    *text = end;
  } else if (0 == memcmp(*text, TRUE_STR, sizeof(TRUE_STR) - 1)) {
    // Bool: true
    res->as.boolean = true;
    res->type = JSON_TYPE_BOL;
    (*text) += sizeof(TRUE_STR) - 1;
  } else if (0 == memcmp(*text, FALSE_STR, sizeof(FALSE_STR) - 1)) {
    // Bool: false
    res->as.boolean = false;
    res->type = JSON_TYPE_BOL;
    (*text) += sizeof(FALSE_STR) - 1;
  } else if (0 == memcmp(*text, NULL_STR, sizeof(NULL_STR) - 1)) {
    // Null
    res->type = JSON_TYPE_NUL;
    (*text) += sizeof(NULL_STR) - 1;
  } else {
    return false;
  }
  return true;
}

static void json_free_arr(JsonArr *);

static void json_free_val(JsonVal *val) {
  switch (val->type) {
  case JSON_TYPE_OBJ:
    json_free_obj(val->as.obj_ptr);
    break;
  case JSON_TYPE_ARR:
    json_free_arr(val->as.arr_ptr);
    break;
  case JSON_TYPE_STR:
    free(val->as.str_ptr);
    break;
  case JSON_TYPE_NUL:
  case JSON_TYPE_INT:
  case JSON_TYPE_FRC:
  case JSON_TYPE_BOL:
    break; // Nothing
  }
}

void json_free_obj(JsonObj *obj) {
  for (size_t i = 0; i < obj->len; i++) {
    json_free_val(&obj->pairs[i].value);
  }
  free(obj->pairs);
  free(obj);
}

static void json_free_arr(JsonArr *arr) {
  for (size_t i = 0; i < arr->len; i++) {
    json_free_val(&arr->values[i]);
  }
  free(arr->values);
  free(arr);
}
