LLVM ?= /home/odroid/build
CLANG := ${LLVM}/bin/clang
CFLAGS += -O2
CFLGAS += -Wall -Wextra -fPIC -fvisibility=hidden -fno-stack-protector
CFLAGS += -D_FORTIFY_SOURCE=0 -static -g
CFLAGS += -fsanitize=safe-stack -fsanitize=zomtag

SOURCES ?= $(wildcard *.c)

all:
	${CLANG} ${CFLAGS} -I./ ${SOURCES} -o main
.PHONY: clean
clean::
	rm main
