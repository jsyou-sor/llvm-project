CC = /home/odroid/build/bin/clang
CFLAGS += -O2
CFLAGS += -fsanitize=safe-stack

all:
	${CC} ${CFLAGS} test.c -o test

clean:
	rm test	
