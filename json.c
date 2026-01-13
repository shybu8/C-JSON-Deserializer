#include "json.h"
#include "stdlib.h"
#include <ctype.h>
#include <string.h>

static char TRUE_STR[] = "true";
static char FALSE_STR[] = "false";
static char NULL_STR[] = "null";

static JsonPair json_parse_pair(char **);
static JsonStr json_parse_str(char **);
static JsonArr json_parse_arr(char **);
static JsonVal json_parse_val(char **);

static void json_skip_whitespace(char **ptr) {
  while (isspace(**ptr))
    (*ptr)++;
}

static void json_skip_digits(char **ptr) {
  while (isdigit(**ptr)) {
    (*ptr)++;
  }
}

JsonObj json_parse_obj(char **text) {
  JsonObj res = {
      .pairs = NULL,
      .len = 0,
  };
  assert(**text == '{');
  (*text)++;
  json_skip_whitespace(text);
  while (**text != '}') {
    if (res.len == 0) {
      res.pairs = malloc(sizeof(JsonPair));
      res.len = 1;
    } else {
      res.pairs = realloc(res.pairs, sizeof(JsonPair) * (res.len + 1));
      res.len += 1;
    }
    res.pairs[res.len - 1] = json_parse_pair(text);
    (*text)++;
    if (**text != ',') {
      json_skip_whitespace(text);
      break;
    } else {
      (*text)++;
      json_skip_whitespace(text);
    }
  }
  assert(**text == '}');
  return res;
}

static JsonStr json_parse_str(char **text) {
  assert(**text == '"');
  JsonStr str = {
      .start = ++*text,
  };
  while (**text != '"')
    (*text)++;
  str.len = *text - str.start;
  assert(**text == '"');
  return str;
}

static JsonPair json_parse_pair(char **text) {
  JsonStr key = json_parse_str(text);
  (*text)++;

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

static JsonArr json_parse_arr(char **text) {
  JsonArr res = {
      .values = NULL,
      .len = 0,
  };
  (*text)++;
  json_skip_whitespace(text);
  while (**text != ']') {
    if (res.len == 0) {
      res.values = malloc(sizeof(JsonVal));
      res.len = 1;
    } else {
      res.values = realloc(res.values, sizeof(JsonVal) * (res.len + 1));
      res.len += 1;
    }

    res.values[res.len - 1] = json_parse_val(text);
    (*text)++;
    if (**text != ',') {
      json_skip_whitespace(text);
      break;
    } else {
      (*text)++;
      json_skip_whitespace(text);
    }
  }
  assert(**text == ']');
  return res;
}

static bool is_fractional(char *num) {
  if (*num == '-')
    num++;
  while (isdigit(*num))
    num++;
  return *num == '.' && isdigit(*(++num));
}

static void skip_fractional(char **text) {
  if (**text == '-')
    (*text)++;
  while (isdigit(**text))
    (*text)++;
  if (**text == '.')
    (*text)++;
  while (isdigit(**text))
    (*text)++;
  if (**text == 'e' || **text == 'E')
    (*text)++;
  while (isdigit(**text))
    (*text)++;
}

static JsonVal json_parse_val(char **text) {
  JsonVal res = {
      .ptr = NULL,
  };
  if (**text == '"') {
    res.ptr = malloc(sizeof(JsonStr));
    *(JsonStr *)res.ptr = json_parse_str(text);
    res.type = JSON_TYPE_STR;
  } else if (**text == '{') {
    res.ptr = malloc(sizeof(JsonObj));
    *(JsonObj *)res.ptr = json_parse_obj(text);
    res.type = JSON_TYPE_OBJ;
  } else if (**text == '[') {
    res.ptr = malloc(sizeof(JsonArr));
    *(JsonArr *)res.ptr = json_parse_arr(text);
    res.type = JSON_TYPE_ARR;
  } else if (isdigit(**text) || **text == '-' && isdigit(*(*text)++)) {
    if (is_fractional(*text)) {
      // Double (maybe exponential)
      res.ptr = malloc(sizeof(double));
      *(double *)res.ptr = strtod(*text, NULL);
      res.type = JSON_TYPE_DBL;
      // TODO: Skip fractional number
      skip_fractional(text);
    } else {
      // Integer
      res.ptr = malloc(sizeof(long long));
      *(long long *)res.ptr = strtoll(*text, NULL, 10);
      res.type = JSON_TYPE_INT;
      while (isdigit(**text))
        (*text)++;
      (*text)--;
    }
  } else if (0 == memcmp(*text, TRUE_STR, sizeof(TRUE_STR) - 1)) {
    res.ptr = malloc(sizeof(bool));
    *(bool *)res.ptr = true;
    res.type = JSON_TYPE_BOL;
    (*text) += sizeof(TRUE_STR) - 2;
  } else if (0 == memcmp(*text, FALSE_STR, sizeof(FALSE_STR) - 1)) {
    res.ptr = malloc(sizeof(bool));
    *(bool *)res.ptr = false;
    res.type = JSON_TYPE_BOL;
    (*text) += sizeof(FALSE_STR) - 2;
  } else if (0 == memcmp(*text, NULL_STR, sizeof(NULL_STR) - 1)) {
    res.ptr = NULL;
    res.type = JSON_TYPE_NUL;
    (*text) += sizeof(NULL_STR) - 2;
  } else {
    assert(false);
  }
  return res;
}
