CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude -ffunction-sections -fdata-sections
AR      := ar

SRCDIR  := src
INCDIR  := include
BUILDDIR := build
LIBDIR  := lib

SRCS    := $(wildcard $(SRCDIR)/*.c)
OBJS    := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
LIB     := $(LIBDIR)/libzephio.a

EXAMPLES := $(patsubst examples/%.c,$(BUILDDIR)/%,$(wildcard examples/*.c))
TESTS    := $(patsubst tests/%.c,$(BUILDDIR)/test_%,$(wildcard tests/*.c))

ifeq ($(DEBUG),1)
CFLAGS  += -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
LDFLAGS := -fsanitize=address -fsanitize=undefined
else
CFLAGS  += -Os -DNDEBUG -flto
LDFLAGS := -flto -s
endif

LDFLAGS += -Wl,--gc-sections -lm

.PHONY: all clean examples lib test docs

all: lib examples

docs: docs/html/html/index.html

docs/html/html/index.html: docs/Doxyfile include/*.h src/*.c README.md docs/ARCHITECTURE.md
	doxygen docs/Doxyfile

test: $(TESTS)
	@for t in $(TESTS); do echo "=== $$t ===" && ./$$t; done

lib: $(LIB)

$(LIB): $(OBJS)
	@mkdir -p $(LIBDIR)
	$(AR) rcs $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%: examples/%.c $(LIB)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $< -L$(LIBDIR) -lzephio $(LDFLAGS) -o $@

examples: $(EXAMPLES)

$(BUILDDIR)/test_%: tests/%.c $(LIB)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -Itests $< -L$(LIBDIR) -lzephio $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILDDIR) $(LIBDIR)
