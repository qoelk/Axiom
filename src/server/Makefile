GOC = go
TARGET = ../../bin/server

all: $(TARGET)

$(TARGET):
	$(GOC) build -o $(TARGET) cmd/main.go

clean:
	rm -f $(TARGET)

.PHONY: all clean
