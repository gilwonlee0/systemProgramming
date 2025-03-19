CC = gcc
CFLAGS = -Wall -Wextra -g -O2

TARGET = testlib

# Default target
all: $(TARGET)

$(TARGET): main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean