CC := gcc
CFLAGS := -g -Wall -Wextra -std=c11 -I../src -D_XOPEN_SOURCE=700

# Source files for queue tests
SRCS := test_queue.c ../src/utils/queue.c
OBJS := $(SRCS:.c=.o)

TARGET := queue_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
