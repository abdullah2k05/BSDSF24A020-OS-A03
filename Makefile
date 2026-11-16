CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude
LDFLAGS = -lreadline
TARGET  = bin/myshell
SRCS    = $(wildcard src/*.c)
OBJS    = $(patsubst src/%.c,obj/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) | bin
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

bin obj:
	mkdir -p $@

clean:
	rm -rf obj bin

.PHONY: all clean
