CC ?= cc
CFLAGS ?= -std=c99 -O2
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=

.PHONY: all clean

all: build/lc3

build/lc3: src/lc3.c
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -f build/lc3
