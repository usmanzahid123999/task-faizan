CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -fPIE -Werror -no-pie
LDFLAGS = $(shell pkg-config --libs libelf)

TARGET = spo_debug
SRC = main.c

all: $(TARGET)

$(TARGET): main.o
    $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

main.o: $(SRC)
    $(CC) $(CFLAGS) -c -o $@ $<

DEPS = $(SRC:.c=.d)

%.d: %.c
    @set -e; rm -f $@; \
    $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$

-include $(DEPS)

clean:
    rm -f $(TARGET) main.o $(DEPS)



