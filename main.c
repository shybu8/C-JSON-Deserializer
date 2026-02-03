#include "json.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(void) {
  FILE *f = fopen("example.json", "r");
  fseek(f, 0, SEEK_END);
  size_t f_size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *f_content = malloc(f_size);
  fread(f_content, sizeof(char), f_size, f);

  JsonObj *obj;

  float st = (float)clock() / CLOCKS_PER_SEC;
  // assert(json_parse_obj(&obj, &f_content));
  json_parse_obj(&obj, (const char **)&f_content);
  float et = (float)clock() / CLOCKS_PER_SEC;
  float dt = et - st;
  printf("Delta time is %f\n", dt);

  st = (float)clock() / CLOCKS_PER_SEC;
  json_free_obj(obj);
  et = (float)clock() / CLOCKS_PER_SEC;
  dt = et - st;
  printf("Freeing delta time is %f\n", dt);
  // printf("Key1 is '%.*s'\n", obj.pairs[0].key.len, obj.pairs[0].key.start);
  // JsonStr *str1 = obj.pairs[0].value.ptr;
  // printf("Value1 is '%.*s'\n", str1->len, str1->start);

  // printf("Key2 is '%.*s'\n", obj.pairs[1].key.len, obj.pairs[1].key.start);
  // JsonStr *str2 = obj.pairs[1].value.ptr;
  // printf("Value2 is '%.*s'\n", str2->len, str2->start);

  // assert(obj.len == 3);
  // JsonObj *obj2 = obj.pairs[2].value.ptr;
  // assert(obj2->len == 2);

  // printf("Folded key is '%.*s'\n", obj2->pairs[0].key.len,
  //        obj2->pairs[0].key.start);
  // JsonStr *str3 = obj2->pairs[0].value.ptr;
  // printf("Folded value is '%.*s'\n", str3->len, str3->start);

  // printf("Array key is '%.*s'\n", obj2->pairs[1].key.len,
  //        obj2->pairs[1].key.start);
  // JsonArr *arr = obj2->pairs[1].value.ptr;
  // for (size_t i = 0; i < arr->len; i++) {
  //   JsonStr *str = arr->values[i].ptr;
  //   printf("Array %ld value is '%.*s'\n", i + 1, str->len, str->start);
  // }

  return 0;
}
