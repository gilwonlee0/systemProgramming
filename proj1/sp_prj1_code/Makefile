CC = gcc
CFLAGS = -Wall -Wextra -g -O2
OBJS = debug.o hex_dump.o list.o hash.o bitmap.o main.o

TARGET = testlib

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) $< -c

.PHONY: all clean
clean:
	rm -f $(TARGET) *.o

-include $(OBJS:.o=.d)
