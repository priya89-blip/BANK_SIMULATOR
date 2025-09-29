CC = gcc
CFLAGS = -Wall -Iinclude
SRCS = src/main.c src/customer.c src/teller.c src/queue.c src/event.c
OBJS = $(SRCS:.c=.o)
TARGET = bin/qSim

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)
