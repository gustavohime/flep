all: example

GCC = gcc
# The following flag is too stringent, I decided not to comply with it
#NOT_COMPLIANT = -Wmissing-prototypes
ANSI_FLAGS = -std=c89 -ansi -Wstrict-prototypes -Wold-style-definition \
  -Wunused -Wall -Wextra -pedantic
CFLAGS = -O3 $(FULL_WARN)
LDFLAGS = -g
LDLIBS = -lm

example: flep.o example.o
	$(GCC) $(LDFLAGS) -o example $^ $(LDLIBS)
flep.o: flep.c
	$(GCC) $(CFLAGS) $(WARN_FLAGS) $(ANSI_FLAGS) -c $<
example.o: example.c
	$(GCC) $(CFLAGS) $(WARN_FLAGS) $(ANSI_FLAGS) -c $<
flep.o: flep.c flep.h

example.o: example.c flep.h

clean:
	rm -f example example.o flep.o
