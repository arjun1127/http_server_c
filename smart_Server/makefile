CC = gcc
CFLAGS = -Wall -pthread
TARGET = smart_server

SRCS = \
    main.c \
    server.c \
    rate_limit.c \
    device_sim.c \
    utils.c

OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET)
