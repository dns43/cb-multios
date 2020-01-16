ROOT=../../conf

BIN_DIR_LLVM=../../llvm-7.0.0.build/`uname -m`/llvm-7.0/bin/bin

include ${ROOT}/common.inc

CFLAGS=-O0 -g3
CXXFLAGS=-O0 -g3
SANITIZER_FLAGS=-fsanitize=dataflow -fno-stack-protector
SANITIZER_ADDL_FLAGS=

SRCS=$(wildcard *.c)

EXES=$(SRCS:.c=.exe)
BITCODES=$(SRCS:.c=.dfsan.bc)
LLS=$(SRCS:.c=.plain.ll)
CC=$(BIN_DIR_LLVM)/clang

all: $(LLS) $(BITCODES) $(EXES)

%.exe: %.c
	$(CC) $(CXXFLAGS) $< -o $@ $(SANITIZER_FLAGS) $(SANITIZER_ADDL_FLAGS)

%.plain.ll:	%.c
	$(CC) $(CFLAGS) -emit-llvm -S $< -o $@

%.plain.ll:	%.cpp
	$(CXX) $(CXXFLAGS) -emit-llvm -S $< -o $@

%.dfsan.bc:	%.c
	$(CC) $(CFLAGS) -c -emit-llvm $< -o $@ $(SANITIZER_FLAGS) $(SANITIZER_ADDL_FLAGS) 
	$(DIS) $@

%.dfsan.bc:	%.cpp
	$(CXX) $(CXXFLAGS) -c -emit-llvm $< -o $@ $(SANITIZER_FLAGS) $(SANITIZER_ADDL_FLAGS)
	$(DIS) $@


