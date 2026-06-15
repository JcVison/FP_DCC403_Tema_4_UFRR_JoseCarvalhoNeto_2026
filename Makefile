CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = edge-mem-profiler
SRC = src/edge_mem_profiler.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
