#include "json.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char TRUE_STR[] = "true";
static char FALSE_STR[] = "false";
static char NULL_STR[] = "null";

static bool json_parse_pair(JsonPair *, const char **);
static bool json_parse_str(JsonStr *, const char **);
static bool json_parse_arr(JsonArr **, const char **);
// bool json_parse_val(JsonVal *, const char **);

static void json_skip_whitespace(const char **ptr) {
  while (isspace((unsigned char)**ptr))
    (*ptr)++;
}

static bool json_parse_obj(JsonObj **res, const char **text) {
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

static bool json_parse_str(JsonStr *str, const char **text) {
  str->needs_dealloc = false;
  if (**text != '"')
    return false;

  str->start = (char *)++*text;
  bool esc = false;
  bool needs_decoding = false;
  for (;; (*text)++) {
    if (**text == '\0' || (unsigned char)**text < 0x20)
      return false;
    if (!esc) {
      if (**text == '"')
        break;
      esc = **text == '\\';
      needs_decoding |= esc;
    } else
      esc = false;
  }
  str->len = *text - str->start;

  str->needs_dealloc = needs_decoding;
  if (needs_decoding) {
    if (!json_decode_str((const char **)&str->start, &str->len, str->start,
                         str->len))
      return false;
  }

  if (**text != '"')
    return false;

  (*text)++;
  return true;
}

static bool json_parse_pair(JsonPair *res, const char **text) {
  // Initialize res->value for errorprone freeing
  res->value.type = JSON_TYPE_NUL;

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

static bool json_parse_arr(JsonArr **res, const char **text) {
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

      json_skip_whitespace(text);
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

typedef enum {
  NUM_INT,
  NUM_FRC,
  NUM_INV,
} NumResult;

static NumResult is_fractional(const char *text) {
  if (*text == '-')
    text++;
  if (*text == '0' && isdigit((unsigned char)*(text + 1)))
    return NUM_INV;
  while (isdigit((unsigned char)*text))
    text++;
  if (*text == '.' && isdigit((unsigned char)*(text + 1)))
    return NUM_FRC;
  if (*text == 'e' || *text == 'E') {
    if (isdigit((unsigned char)*(text + 1)) ||
        ((*(text + 1) == '-' || *(text + 1) == '+') &&
         isdigit((unsigned char)*(text + 2))))
      return NUM_FRC;
  }
  return NUM_INT;
}

bool json_parse_val(JsonVal *res, const char **text) {
  res->type = JSON_TYPE_NUL;
  if (**text == '"') {
    res->as.str_ptr = malloc(sizeof(JsonStr));
    res->type = JSON_TYPE_STR;
    if (!json_parse_str(res->as.str_ptr, text)) {
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
    switch (is_fractional(*text)) {
    case NUM_INT:
      res->type = JSON_TYPE_INT;
      errno = 0;
      res->as.integer = strtoll(*text, &end, 10);
      if (errno == ERANGE)
        return false;
      break;
    case NUM_FRC:
      res->type = JSON_TYPE_FRC;
      errno = 0;
      res->as.fract = strtod(*text, &end);
      if (errno == ERANGE || !isfinite(res->as.fract))
        return false;
      break;
    default:
      return false;
    }
    *text = end;
  } else if (0 == strncmp(*text, TRUE_STR, sizeof(TRUE_STR) - 1)) {
    // Bool: true
    res->as.boolean = true;
    res->type = JSON_TYPE_BOL;
    (*text) += sizeof(TRUE_STR) - 1;
  } else if (0 == strncmp(*text, FALSE_STR, sizeof(FALSE_STR) - 1)) {
    // Bool: false
    res->as.boolean = false;
    res->type = JSON_TYPE_BOL;
    (*text) += sizeof(FALSE_STR) - 1;
  } else if (0 == strncmp(*text, NULL_STR, sizeof(NULL_STR) - 1)) {
    // Null
    res->type = JSON_TYPE_NUL;
    (*text) += sizeof(NULL_STR) - 1;
  } else {
    return false;
  }
  return true;
}

static int hexval(unsigned char c) {
  if (c >= '0' && c <= '9')
    return (int)(c - '0');
  if (c >= 'a' && c <= 'f')
    return (int)(c - 'a' + 10);
  if (c >= 'A' && c <= 'F')
    return (int)(c - 'A' + 10);
  return -1;
}

static bool parse_u16_4hex(const char *p, const char *end, uint16_t *out) {
  if (end - p < 4)
    return false;
  int h0 = hexval((unsigned char)p[0]);
  int h1 = hexval((unsigned char)p[1]);
  int h2 = hexval((unsigned char)p[2]);
  int h3 = hexval((unsigned char)p[3]);
  if (h0 < 0 || h1 < 0 || h2 < 0 || h3 < 0)
    return false;
  *out = (uint16_t)((h0 << 12) | (h1 << 8) | (h2 << 4) | h3);
  return true;
}

static size_t utf8_encode(uint32_t cp, char out[4]) {
  if (cp <= 0x7F) {
    out[0] = (char)cp;
    return 1;
  } else if (cp <= 0x7FF) {
    out[0] = (char)(0xC0 | (cp >> 6));
    out[1] = (char)(0x80 | (cp & 0x3F));
    return 2;
  } else if (cp <= 0xFFFF) {
    out[0] = (char)(0xE0 | (cp >> 12));
    out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
    out[2] = (char)(0x80 | (cp & 0x3F));
    return 3;
  } else {
    out[0] = (char)(0xF0 | (cp >> 18));
    out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
    out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
    out[3] = (char)(0x80 | (cp & 0x3F));
    return 4;
  }
}

bool json_decode_str(const char **res, size_t *res_len, char *src, size_t len) {
  *res = malloc(len);
  char *cur = (char *)*res;
  const char *end = src + len;

  while (src < end) {
    if (*src == '\\') {
      if (++src >= end)
        return false;
      switch (*src++) {
      case 'b':
        *cur++ = '\b';
        break;
      case 'f':
        *cur++ = '\f';
        break;
      case 'n':
        *cur++ = '\n';
        break;
      case 'r':
        *cur++ = '\r';
        break;
      case 't':
        *cur++ = '\t';
        break;
      case '\\':
        *cur++ = '\\';
        break;
      case '"':
        *cur++ = '"';
        break;
      case '/':
        *cur++ = '/';
        break;
      case 'u': {
        uint16_t u1;
        if (!parse_u16_4hex(src, end, &u1))
          return false;
        src += 4; // Cause four HEX-digits
        uint32_t cp;

        if (u1 >= 0xD800 && u1 <= 0xDBFF) { // High surrogate
          if (end - src < 6)
            return false; // Need \uXXXX
          if (src[0] != '\\' || src[1] != 'u')
            return false;
          uint16_t u2;
          src += 2;
          if (!parse_u16_4hex(src, end, &u2))
            return false;
          if (u2 < 0xDC00 || u2 > 0xDFFF) // Must be low surrogate
            return false;
          src += 4;
          cp = 0x10000u +
               (((uint32_t)(u1 - 0xD800) << 10) | (uint32_t)(u2 - 0xDC00));
        } else if (u1 >= 0xDC00 && u1 <= 0xDFFF)
          return false;
        else
          cp = (uint32_t)u1;

        if (cp > 0x10FFFFu)
          return false;

        char utf8[4];
        size_t utf8_len = utf8_encode(cp, utf8);

        memcpy(cur, utf8, utf8_len);
        cur += utf8_len;
        break;
      }
      default:
        return false;
      }
    } else
      *cur++ = *src++;
  }
  *res_len = cur - *res;
  return true;
}

static void json_free_obj(JsonObj *);
static void json_free_arr(JsonArr *);

void json_free_val(JsonVal *val) {
  switch (val->type) {
  case JSON_TYPE_OBJ:
    json_free_obj(val->as.obj_ptr);
    break;
  case JSON_TYPE_ARR:
    json_free_arr(val->as.arr_ptr);
    break;
  case JSON_TYPE_STR:
    if (val->as.str_ptr->needs_dealloc)
      free(val->as.str_ptr->start);
    free(val->as.str_ptr);
    break;
  case JSON_TYPE_NUL:
  case JSON_TYPE_INT:
  case JSON_TYPE_FRC:
  case JSON_TYPE_BOL:
    break; // Nothing
  }
}

static void json_free_obj(JsonObj *obj) {
  for (size_t i = 0; i < obj->len; i++) {
    if (obj->pairs[i].key.needs_dealloc)
      free(obj->pairs[i].key.start);
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

JsonVal *json_value_by_key(JsonObj *obj, char *to_find) {
  JsonStr *key_str;
  for (size_t i = 0; i < obj->len; i++) {
    key_str = &obj->pairs[i].key;
    if (0 == strncmp(to_find, key_str->start, key_str->len))
      return &obj->pairs[i].value;
  }
  return NULL;
}
