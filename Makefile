CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -fPIE
LDFLAGS = -lelf

TARGET = spo_debug
SRC = main.c

all: build

build: $(SRC)
    $(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(SRC)

run:
    ./$(TARGET) program_name

clean:
    rm -f $(TARGET)
