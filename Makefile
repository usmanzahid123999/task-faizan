CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -fPIE
LDFLAGS = -lelf

TARGET = spo_debug
SRC = main.c

all: spo_debug

spo_debug: main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) main.o

main.o: $(SRC)
	$(CC) $(CFLAGS) -c -o main.o $(SRC)

clean:
	rm -f $(TARGET) main.o
