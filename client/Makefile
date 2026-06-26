CC = clang
CFLAGS = -Wall -Wextra -O2 -pthread -I./lib/include
LDFLAGS = -lcurl -lcjson -lraylib
TARGET = ../../bin/client
SRCS = src/main.c	src/state.c

CJSON_SRC = lib/src/cJSON.c
CJSON_OBJ = lib/src/cJSON.o
CJSON_LIB = lib/build/libcjson.a

all: $(CJSON_LIB) $(TARGET)

$(CJSON_LIB): $(CJSON_OBJ)
	ar rcs $@ $^

$(CJSON_OBJ): $(CJSON_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(SRCS) $(CJSON_LIB)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ -L./lib/build $(CJSON_LIB) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
