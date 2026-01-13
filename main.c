#include "json.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  FILE *f = fopen("example.json", "r");
  fseek(f, 0, SEEK_END);
  size_t f_size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *f_content = malloc(f_size);
  fread(f_content, sizeof(char), f_size, f);

  JsonObj obj = json_parse_obj(f_content);
  printf("Key1 is '%.*s'\n", obj.pairs[0].key->len, obj.pairs[0].key->start);
  JsonStr *value1 = obj.pairs[0].value;
  printf("Value1 is '%.*s'\n", value1->len, value1->start);

  printf("Key2 is '%.*s'\n", obj.pairs[1].key->len, obj.pairs[1].key->start);
  JsonStr *value2 = obj.pairs[1].value;
  printf("Value2 is '%.*s'\n", value2->len, value2->start);
  return 0;
}
