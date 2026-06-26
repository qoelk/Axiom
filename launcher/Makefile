CC = clang
CFLAGS = -Wall -Wextra -O2
TARGET = ../../bin/launcher
SRCS = src/main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean

