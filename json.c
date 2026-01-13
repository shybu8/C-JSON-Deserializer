#include "json.h"
#include "stdlib.h"

bool json_is_whitespace(char *ptr) {
  return *ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n';
}

void json_skip_whitespace(char **ptr) {
  while (json_is_whitespace(*ptr))
    (*ptr)++;
}

JsonObj json_parse_obj(char *text) {
  JsonObj res = {
      .pairs = NULL,
      .len = 0,
  };
  assert(*text == '{');
  text++;
  json_skip_whitespace(&text);
  while (*text != '}') {
    if (res.len == 0) {
      res.pairs = malloc(sizeof(JsonPair));
      res.len = 1;
    } else {
      res.pairs = realloc(res.pairs, sizeof(JsonPair) * (res.len + 1));
      res.len += 1;
    }
    res.pairs[res.len - 1] = json_parse_pair(&text);
    if (*(++text) != ',') {
      json_skip_whitespace(&text);
      break;
    } else {
      text++;
      json_skip_whitespace(&text);
    }
  }
  assert(*text == '}');
  return res;
}

JsonStr json_parse_str(char **text) {
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

JsonPair json_parse_pair(char **text) {
  JsonStr *key = malloc(sizeof(JsonStr));
  *key = json_parse_str(text);
  (*text)++;
  // Key string done

  json_skip_whitespace(text);
  assert(**text == ':');
  (*text)++;
  json_skip_whitespace(text);

  assert(**text != '{'); // Not implemented
  assert(**text != '['); // Not implemented
  JsonStr *value = malloc(sizeof(JsonStr));
  *value = json_parse_str(text);

  JsonPair res = {
      .key = key,
      .value = value,
      .value_type = JSON_TYPE_STR,
  };
  return res;
}
