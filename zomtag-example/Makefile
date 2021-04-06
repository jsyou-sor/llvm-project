DIRECTORY ?= /home/jsyou/zomtag/example
LINARO_GCC ?= ${DIRECTORY}/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu
SYSROOT ?= ${DIRECTORY}/sysroot-glibc-linaro-2.25-2018.05-aarch64-linux-gnu
LLVM ?= /home/jsyou/zomtag/build

LLVM_LINKER := ${LLVM}/bin/llvm-link
LLVM_DIS    := ${LLVM}/bin/llvm-dis
LLVM_OPT    := ${LLVM}/bin/opt
LLVM_CONFIG := ${LLVM}/bin/llvm-config
CLANG       := ${LLVM}/bin/clang
CLANGPP     := ${LLVM}/bin/clang++
LLC         := ${LLVM}/bin/llc
GCC         := ${LINARO_GCC}/bin/aarch64-linux-gnu-gcc

#CFLAGS += -Xclang -load -Xclang /home/jsyou/zomtag/build/lib/LLVMGepMDPass.*
CFLAGS += -O0
CFLAGS += --target=aarch64-linux-gnu
CFLAGS += -march=armv8.2a
CFLAGS += -mfloat-abi=hard
CFLAGS += -I${SYSROOT}/usr/include
CFLAGS += -B$(LINARO_GCC)
CFLAGS += -Wall
CFLAGS += -Wextra
CLFAGS += -fPIC
CFLAGS += -fvisibility=hidden
CFLAGS += --sysroot=${SYSROOT}
CFLAGS += -fno-stack-protector
CFLAGS += -D_FORTIFY_SOURCE=0
CFLAGS += -static
#CFLAGS += -mllvm -hello
CFLAGS += -Wno-override-module

LLC_FLAGS += -mtriple=aarch64-linux-gnu
OPT_FLAGS += -load ../build/lib/LLVMHello.so -hello

SOURCES ?= $(wildcard *.c)
BIN ?= $(shell basename $(shell pwd)).out

$(BIN): $(SOURCES)
	${CLANG} ${CFLAGS} $^ -o $@

${SOURCES:.c=.ll}: %.ll : %.c
	${CLANG} -S -emit-llvm -c $<

${BIN:=.linked.ll}: ${SOURCES:.c=.ll}
	${LLVM_LINKER} -S $^ -o $@

${BIN:=.linked.opt.ll}: %.linked.opt.ll : %.linked.ll
	${LLVM_OPT} ${OPT_FLAGS} -S < $< > $@

$(BIN:=.s): %.s : %.linked.opt.ll
	$(LLC) $(LLC_FLAGS) $< -o $@

test:
	#${CLANG} -emit-llvm -o test.bc -c test.c
	#${LLVM_OPT} -load ../../build/lib/LLVMGepMDPass.so -gep-md-pass test.bc -o test.opt.bc
	#${CLANG} ${CFLAGS} -o test test.opt.bc
	${CLANG} ${CFLAGS} -o test test.c

.PHONY: dis
dis: $(BIN:=.linked.ll) $(BIN=.linked.opt.ll) $(BIN:=.s)

.PHONY: clean
clean::
	rm -rf $(CLEAN_FILES) *.bc *.o *.s $(BIN) $(BIN) *.linked_bc *.ll *.dump a.out example hello test
