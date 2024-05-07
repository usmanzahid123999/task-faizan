CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -fPIE -Werror -Wl,-no-pie
LDFLAGS = -lelf

TARGET = spo_debug
SRC = main.c

all: $(TARGET)

$(TARGET): main.o $(DEPS)
    $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

main.o: $(SRC) $(DEPS)
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


