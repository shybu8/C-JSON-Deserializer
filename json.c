#include "json.h"
#include "stdlib.h"
#include <ctype.h>
#include <string.h>

static char TRUE_STR[] = "true";
static char FALSE_STR[] = "false";
static char NULL_STR[] = "null";

static JsonPair json_parse_pair(char **);
static JsonStr json_parse_str(char **);
static void json_parse_arr(JsonArr **, char **);
static JsonVal json_parse_val(char **);

static void json_skip_whitespace(char **ptr) {
  while (isspace((unsigned char)**ptr))
    (*ptr)++;
}

void json_parse_obj(JsonObj **res, char **text) {
  *res = malloc(sizeof(JsonObj));
  (*res)->pairs = NULL;
  (*res)->len = 0;

  assert(**text == '{');
  (*text)++;
  json_skip_whitespace(text);
  size_t actual_len = 0;
  while (**text != '}') {
    if (actual_len == 0) {
      (*res)->pairs = malloc(sizeof(JsonPair) * 2);
      actual_len = 2;
    } else if ((*res)->len == actual_len) {
      (*res)->pairs =
          realloc((*res)->pairs, sizeof(JsonPair) * (actual_len * 2));
      actual_len *= 2;
    }
    (*res)->len += 1;
    (*res)->pairs[(*res)->len - 1] = json_parse_pair(text);

    json_skip_whitespace(text);
    if (**text == ',') {
      (*text)++;
      json_skip_whitespace(text);
    } else
      break;
  }
  json_skip_whitespace(text);
  assert(**text == '}');
  (*text)++;
}

static JsonStr json_parse_str(char **text) {
  assert(**text == '"');
  JsonStr str = {
      .start = ++*text,
  };
  bool esc = false;
  for (;; (*text)++) {
    if (!esc) {
      if (**text == '"')
        break;
      esc = **text == '\\';
    } else
      esc = false;
  }
  str.len = *text - str.start;
  assert(**text == '"');
  (*text)++;
  return str;
}

static JsonPair json_parse_pair(char **text) {
  JsonStr key = json_parse_str(text);

  json_skip_whitespace(text);
  assert(**text == ':');
  (*text)++;
  json_skip_whitespace(text);

  JsonVal value = json_parse_val(text);

  JsonPair res = {
      .key = key,
      .value = value,
  };
  return res;
}

static void json_parse_arr(JsonArr **res, char **text) {
  *res = malloc(sizeof(JsonArr));
  (*res)->values = NULL;
  (*res)->len = 0;

  (*text)++;
  json_skip_whitespace(text);
  size_t actual_len = 0;
  while (**text != ']') {
    if (actual_len == 0) {
      (*res)->values = malloc(sizeof(JsonVal) * 2);
      actual_len = 2;
    } else if ((*res)->len == actual_len) {
      (*res)->values =
          realloc((*res)->values, sizeof(JsonVal) * (actual_len * 2));
      actual_len *= 2;
    }
    (*res)->len += 1;

    (*res)->values[(*res)->len - 1] = json_parse_val(text);
    if (**text == ',') {
      (*text)++;
      json_skip_whitespace(text);
    } else
      break;
  }
  json_skip_whitespace(text);
  assert(**text == ']');
  (*text)++;
}

static bool is_fractional(char *num) {
  if (*num == '-')
    num++;
  while (isdigit((unsigned char)*num))
    num++;
  return *num == '.' && isdigit((unsigned char)*(++num));
}

static JsonVal json_parse_val(char **text) {
  JsonVal res = {};
  if (**text == '"') {
    res.as.str_ptr = malloc(sizeof(JsonStr));
    *res.as.str_ptr = json_parse_str(text);
    res.type = JSON_TYPE_STR;
  } else if (**text == '{') {
    json_parse_obj(&res.as.obj_ptr, text);
    res.type = JSON_TYPE_OBJ;
  } else if (**text == '[') {
    json_parse_arr(&res.as.arr_ptr, text);
    res.type = JSON_TYPE_ARR;
  } else if (isdigit((unsigned char)**text) ||
             **text == '-' && isdigit((unsigned char)*(*text + 1))) {
    char *end;
    if (is_fractional(*text)) {
      // Double (maybe exponential)
      res.as.fract = strtod(*text, &end);
      res.type = JSON_TYPE_FRC;
    } else {
      // Integer
      res.as.integer = strtoll(*text, &end, 10);
      res.type = JSON_TYPE_INT;
    }
    *text = end;
  } else if (0 == memcmp(*text, TRUE_STR, sizeof(TRUE_STR) - 1)) {
    // Bool: true
    res.as.boolean = true;
    res.type = JSON_TYPE_BOL;
    (*text) += sizeof(TRUE_STR) - 1;
  } else if (0 == memcmp(*text, FALSE_STR, sizeof(FALSE_STR) - 1)) {
    // Bool: false
    res.as.boolean = false;
    res.type = JSON_TYPE_BOL;
    (*text) += sizeof(FALSE_STR) - 1;
  } else if (0 == memcmp(*text, NULL_STR, sizeof(NULL_STR) - 1)) {
    // Null
    res.type = JSON_TYPE_NUL;
    (*text) += sizeof(NULL_STR) - 1;
  } else {
    assert(false);
  }
  return res;
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
