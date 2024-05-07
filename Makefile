CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -fPIE -Werror -no-pie -m32
LDFLAGS = -lelf

TARGET = spo_debug
SRC = main.c program.c  # Include both main.c and program.c here

all: $(TARGET)

$(TARGET): main.o program.o
    $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

main.o: main.c
    $(CC) $(CFLAGS) -c -o $@ $<

program.o: program.c
    $(CC) $(CFLAGS) -c -o $@ $<

DEPS = $(SRC:.c=.d)

%.d: %.c
    @set -e; rm -f $@; \
    $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$

-include $(DEPS)

clean:
    rm -f $(TARGET) main.o program.o $(DEPS)


