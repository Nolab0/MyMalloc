CC = gcc
CPPFLAGS = -D_DEFAULT_SOURCE -g
CFLAGS = -Wall -Wextra -Werror -std=c99 -fPIC -fno-builtin -g
LDFLAGS = -shared -g
VPATH = src
INCLUDES = -I./src/ -I./tests
TARGET_LIB = libmalloc.so
OBJS = malloc.o utils.o

all: library

library: $(TARGET_LIB)
$(TARGET_LIB): CFLAGS += -pedantic -fvisibility=hidden
$(TARGET_LIB): LDFLAGS += -Wl,--no-undefined
$(TARGET_LIB): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

debug: CFLAGS += -g
debug: clean $(TARGET_LIB)

check: ./src/*.c ./tests/tests1.c
	$(CC) $(CFLAGS) -g ${INCLUDES} src/*.c tests/tests1.c -o check
	LD_LIBRARY_PATH=. ./check


clean:
	$(RM) $(TARGET_LIB) $(OBJS)
	$(RM) check

.PHONY: all $(TARGET_LIB) clean
