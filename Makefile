SRC = main.c json.c
NAME = c-json

all:
	gcc ${SRC} -Wall -Wpedantic -fsanitize=address -o ${NAME}

release:
	gcc ${SRC} -O2 -o ${NAME}

clean:
	rm ${NAME}


