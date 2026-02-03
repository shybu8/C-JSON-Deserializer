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
  char *f_alloc = malloc(f_size);
  char *f_content = f_alloc;
  fread(f_content, sizeof(char), f_size, f);

  JsonVal val;

  float st = (float)clock() / CLOCKS_PER_SEC;
  json_parse_val(&val, (const char **)&f_content);
  float et = (float)clock() / CLOCKS_PER_SEC;
  float dt = et - st;
  printf("Delta time is %f\n", dt);

  printf("Value is %d\n", val.type);

  st = (float)clock() / CLOCKS_PER_SEC;
  json_free_val(&val);
  et = (float)clock() / CLOCKS_PER_SEC;
  dt = et - st;
  printf("Freeing delta time is %f\n", dt);

  free(f_alloc);

  return 0;
}
