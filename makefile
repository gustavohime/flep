all: example

GCC = gcc
#NOT_COMPLIANT = -std=c89 -Wmissing-prototypes -Wunused
FULL_WARN = -Wall -Wextra -std=gnu99 -pedantic \
  -Wstrict-prototypes -Wold-style-definition -Wno-unused
CFLAGS = -O3 $(FULL_WARN)
LDFLAGS = -g
LDLIBS = -lm

example: flep.o example.o
	$(GCC) $(LDFLAGS) -o example $^ $(LDLIBS)
%.o: %.c
	$(GCC) $(CFLAGS) -c $<
flep.o: flep.c flep.h
example.o: example.c flep.h

clean:
	rm -f example example.o flep.o
