CC = gcc
CFLAGS = -Wall -O2

all: httpd

httpd: httpd.c rate_limit.c
	$(CC) $(CFLAGS) httpd.c rate_limit.c -o httpd

clean:
	rm -f httpd
