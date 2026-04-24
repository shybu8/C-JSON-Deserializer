# C-JSON Serializer-Deserializer
---
Этот проект представляет собой библиотеку для сериализации/десериализации Json для языка C.

## Возможности
- Парсинг всех базовых типов JSON:
  - object
  - array
  - string
  - integer (long long, 64 bit)
  - double
  - boolean
  - null
- Поддержка escape-последовательностей (`\n`, `\t`, `\uXXXX`, surrogate pairs)
- Поиск значений по ключу
- Сериализация (compact / pretty-print)

## Особенности
- Без копирования исходных строк (zero-copy для простых строк)
- Строки с escape-sequences декодируются в новую память
- Возможность обработать ошибку (при получении false переданный указатель стоит на проблемном месте)
- Сериализатор не экранирует строки автоматически. Если `JsonStr` создаётся или изменяется вручную, перед сериализацией нужно самостоятельно подготовить строку в JSON-safe виде: вызвать `json_str_needs_encoding()` и при необходимости `json_str_encode_into_buf()`.

## Пример использования
```c
#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  // Чтение всего файла в буфер
  FILE *f = fopen("example.json", "r");
  fseek(f, 0, SEEK_END);
  size_t f_size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *f_alloc = malloc(f_size);
  char *f_content = f_alloc;
  fread(f_content, sizeof(char), f_size, f);

  // Десериализация (с обработкой возможной ошибки)
  JsonVal val;
  if (!json_parse_val(&val, (const char **)&f_content)) {
    printf("Произошла ошибка десериализации на символе: %c\n", *f_content);
    json_free_val(&val); // При возниковении ошибки объекты гарантированно корректны для выполнения очистки
  }

  // Сериализация (со стилем с табами)
  size_t buf_len = 1;
  char *buf = malloc(buf_len);
  *buf = '\0'; // Сериалайзер дописывает данные в существующую null-терменированную строку
  size_t str_len = strlen(buf);
  JsonStyle style = JSON_STYLE_PRETTY_PRINT_TABS;
  json_serialize_val(&val, &buf, &str_len, &buf_len, &style);

  // Освобождение десериализованных данных
  json_free_val(&val);
}
```

## Тестирование производительности
Было выполнено тестирование производительнсоти с помощью десериализации и сериализации [Json-файла размером 1GB](https://github.com/antonmedv/json-examples/blob/master/data_1gb.json).
Программа была скомпилированна с флагом оптимизации `-O2`.

### Время десериализации и сериализации
![serde-timings](./docs/images/timings.png)

### Вывод `perf report`
![perf-report](./docs/images/perf-report.png)

### Вывод `perf stat`
![perf-stat](./docs/images/perf-stat.png)
