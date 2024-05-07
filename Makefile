CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -fPIE -Werror -no-pie -m32
LDFLAGS = -lelf

TARGET = spo_debug
SRC = main.c

all: spo_debug

spo_debug: main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) main.o

main.o: $(SRC)
	$(CC) $(CFLAGS) -c -o main.o $(SRC)

DEPS = $(SRC:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

-include $(DEPS)

clean:
	rm -f $(TARGET) main.o $(DEPS)
