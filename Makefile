SRC = main.c json.c
NAME = c-json

all:
	gcc ${SRC} -o ${NAME}

clean:
	rm ${NAME}
