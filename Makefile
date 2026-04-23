CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude -ffunction-sections -fdata-sections
AR      := ar

SRCDIR  := src
INCDIR  := include
BUILDDIR := build
LIBDIR  := lib

# Core sources (non-widget)
CORE_SRCS := \
    $(SRCDIR)/context.c \
    $(SRCDIR)/widget.c \
    $(SRCDIR)/screen.c \
    $(SRCDIR)/input.c \
    $(SRCDIR)/terminal.c \
    $(SRCDIR)/ansi.c \
    $(SRCDIR)/style.c \
    $(SRCDIR)/text.c \
    $(SRCDIR)/layout.c \
    $(SRCDIR)/mouse.c \
    $(SRCDIR)/clipboard.c \
    $(SRCDIR)/plugin.c

# Widget sources grouped by module
WIDGET_CORE_SRCS := \
    $(SRCDIR)/label.c \
    $(SRCDIR)/button.c \
    $(SRCDIR)/box.c \
    $(SRCDIR)/separator.c \
    $(SRCDIR)/container.c \
    $(SRCDIR)/progress.c \
    $(SRCDIR)/input_field.c \
    $(SRCDIR)/checkbox.c \
    $(SRCDIR)/radio.c \
    $(SRCDIR)/list.c \
    $(SRCDIR)/dropdown.c \
    $(SRCDIR)/dialog.c

WIDGET_ADV_SRCS := \
    $(SRCDIR)/scroll_container.c \
    $(SRCDIR)/split_pane.c \
    $(SRCDIR)/tabbar.c \
    $(SRCDIR)/statusbar.c \
    $(SRCDIR)/menubar.c \
    $(SRCDIR)/context_menu.c \
    $(SRCDIR)/tree_view.c \
    $(SRCDIR)/table.c

WIDGET_TEXT_SRCS := \
    $(SRCDIR)/text_view.c \
    $(SRCDIR)/textarea.c

WIDGET_EXTRA_SRCS := \
    $(SRCDIR)/anim_effects.c

# App depends on animator and toast, so they go in core
CORE_SRCS += \
    $(SRCDIR)/animation.c \
    $(SRCDIR)/animator.c \
    $(SRCDIR)/toast.c \
    $(SRCDIR)/app.c

# All sources for static library
SRCS := $(CORE_SRCS) $(WIDGET_CORE_SRCS) $(WIDGET_ADV_SRCS) $(WIDGET_TEXT_SRCS) $(WIDGET_EXTRA_SRCS)
OBJS    := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
LIB     := $(LIBDIR)/libzephio.a

# Core shared library settings
SONAME      := libzephio.so.1
SOLIB       := $(LIBDIR)/$(SONAME)
SOLINK      := $(LIBDIR)/libzephio.so

# PIC flags for shared objects
SO_CFLAGS   := $(CFLAGS) -fPIC -DZEPHIO_BUILDING_SO
SO_LDFLAGS  := -shared -Wl,-soname,$(SONAME) -Wl,--no-undefined -lm

# Widget module settings
WIDGET_CORE_SONAME := libzephio-widgets-core.so.1
WIDGET_ADV_SONAME  := libzephio-widgets-adv.so.1
WIDGET_TEXT_SONAME := libzephio-widgets-text.so.1
WIDGET_EXTRA_SONAME := libzephio-widgets-extra.so.1

WIDGET_CORE_SOLIB := $(LIBDIR)/$(WIDGET_CORE_SONAME)
WIDGET_ADV_SOLIB  := $(LIBDIR)/$(WIDGET_ADV_SONAME)
WIDGET_TEXT_SOLIB := $(LIBDIR)/$(WIDGET_TEXT_SONAME)
WIDGET_EXTRA_SOLIB := $(LIBDIR)/$(WIDGET_EXTRA_SONAME)

WIDGET_CORE_SOLINK := $(LIBDIR)/libzephio-widgets-core.so
WIDGET_ADV_SOLINK  := $(LIBDIR)/libzephio-widgets-adv.so
WIDGET_TEXT_SOLINK := $(LIBDIR)/libzephio-widgets-text.so
WIDGET_EXTRA_SOLINK := $(LIBDIR)/libzephio-widgets-extra.so

# Combined widget library for examples (simplifies linking)
WIDGETS_SONAME := libzephio-widgets.so.1
WIDGETS_SOLIB := $(LIBDIR)/$(WIDGETS_SONAME)
WIDGETS_SOLINK := $(LIBDIR)/libzephio-widgets.so

# Widget module link flags
WIDGET_SO_LDFLAGS := -shared -Wl,-rpath,'$$ORIGIN' -lm

WIDGET_CORE_LINK := $(SOLINK)
WIDGET_ADV_LINK := $(WIDGET_CORE_SOLINK) $(SOLINK)
WIDGET_TEXT_LINK := $(WIDGET_ADV_SOLINK) $(WIDGET_CORE_SOLINK) $(SOLINK)
WIDGET_EXTRA_LINK := $(WIDGET_TEXT_SOLINK) $(WIDGET_ADV_SOLINK) $(WIDGET_CORE_SOLINK) $(SOLINK)

EXAMPLES := $(patsubst examples/%.c,$(BUILDDIR)/%,$(wildcard examples/*.c))
TESTS    := $(patsubst tests/%.c,$(BUILDDIR)/test_%,$(wildcard tests/*.c))

ifeq ($(DEBUG),1)
CFLAGS  += -g -O0 -DDEBUG
LDFLAGS := 
else
CFLAGS  += -Os -DNDEBUG -flto
LDFLAGS := -flto -s
endif

LDFLAGS += -Wl,--gc-sections -lm -ldl

.PHONY: all clean examples lib test docs so install

all: so examples

docs: docs/html/html/index.html

docs/html/html/index.html: docs/Doxyfile include/*.h src/*.c README.md docs/ARCHITECTURE.md
	doxygen docs/Doxyfile

test: $(TESTS)
	@for t in $(TESTS); do echo "=== $$t ===" && ./$$t; done

lib: $(LIB)

$(LIB): $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
	@mkdir -p $(LIBDIR)
	$(AR) rcs $@ $^

so: $(SOLIB) $(SOLINK) $(WIDGET_CORE_SOLIB) $(WIDGET_CORE_SOLINK) $(WIDGET_ADV_SOLIB) $(WIDGET_ADV_SOLINK) $(WIDGET_TEXT_SOLIB) $(WIDGET_TEXT_SOLINK) $(WIDGET_EXTRA_SOLIB) $(WIDGET_EXTRA_SOLINK) $(WIDGETS_SOLIB) $(WIDGETS_SOLINK)

$(SOLIB): $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.pic.o,$(CORE_SRCS))
	@mkdir -p $(LIBDIR)
	$(CC) $(SO_LDFLAGS) $^ -o $@

$(SOLINK): $(SOLIB)
	@ln -sf $(notdir $(SOLIB)) $@

# Widget modules - each links against core .so
$(WIDGET_CORE_SOLIB): $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.pic.o,$(WIDGET_CORE_SRCS)) $(SOLIB)
	@mkdir -p $(LIBDIR)
	$(CC) $(WIDGET_SO_LDFLAGS) -Wl,-soname,$(WIDGET_CORE_SONAME) $^ $(SOLIB) -o $@
	@ln -sf $(notdir $@) $(WIDGET_CORE_SOLINK)

$(WIDGET_ADV_SOLIB): $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.pic.o,$(WIDGET_ADV_SRCS)) $(SOLIB) $(WIDGET_CORE_SOLIB)
	@mkdir -p $(LIBDIR)
	$(CC) $(WIDGET_SO_LDFLAGS) -Wl,-soname,$(WIDGET_ADV_SONAME) $^ $(WIDGET_CORE_SOLIB) -o $@
	@ln -sf $(notdir $@) $(WIDGET_ADV_SOLINK)

$(WIDGET_TEXT_SOLIB): $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.pic.o,$(WIDGET_TEXT_SRCS)) $(SOLIB) $(WIDGET_CORE_SOLIB) $(WIDGET_ADV_SOLIB)
	@mkdir -p $(LIBDIR)
	$(CC) $(WIDGET_SO_LDFLAGS) -Wl,-soname,$(WIDGET_TEXT_SONAME) $^ $(WIDGET_ADV_SOLIB) $(WIDGET_CORE_SOLIB) -o $@
	@ln -sf $(notdir $@) $(WIDGET_TEXT_SOLINK)

$(WIDGET_EXTRA_SOLIB): $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.pic.o,$(WIDGET_EXTRA_SRCS)) $(SOLIB) $(WIDGET_CORE_SOLIB) $(WIDGET_ADV_SOLIB) $(WIDGET_TEXT_SOLIB)
	@mkdir -p $(LIBDIR)
	$(CC) $(WIDGET_SO_LDFLAGS) -Wl,-soname,$(WIDGET_EXTRA_SONAME) $^ $(WIDGET_TEXT_SOLIB) $(WIDGET_ADV_SOLIB) $(WIDGET_CORE_SOLIB) -o $@
	@ln -sf $(notdir $@) $(WIDGET_EXTRA_SOLINK)

# Combined widget library - includes all widget modules (for easy linking)
$(WIDGETS_SOLIB): $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.pic.o,$(WIDGET_CORE_SRCS) $(WIDGET_ADV_SRCS) $(WIDGET_TEXT_SRCS) $(WIDGET_EXTRA_SRCS))
	@mkdir -p $(LIBDIR)
	$(CC) -shared -Wl,-soname,$(WIDGETS_SONAME) $^ -o $@
	@ln -sf $(notdir $@) $(WIDGETS_SOLINK)

# Build PIC objects for shared library
$(BUILDDIR)/%.pic.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(SO_CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%: examples/%.c $(SOLIB) $(SOLINK) $(WIDGETS_SOLIB) $(WIDGETS_SOLINK)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $< -L$(LIBDIR) -Wl,--as-needed $(WIDGETS_SOLIB) $(SOLIB) $(LDFLAGS) -Wl,-rpath,'$$ORIGIN/../lib' -o $@

examples: $(EXAMPLES)

$(BUILDDIR)/test_%: tests/%.c $(SOLIB) $(WIDGETS_SOLIB)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -Itests $< -L$(LIBDIR) -lzephio-widgets -lzephio $(LDFLAGS) -Wl,-rpath,'$$ORIGIN/../lib' -o $@

clean:
	rm -rf $(BUILDDIR) $(LIBDIR)

install:
	install -Dm755 $(SOLIB) $(DESTDIR)/usr/local/lib/$(notdir $(SOLIB))
	ln -sf $(notdir $(SOLIB)) $(DESTDIR)/usr/local/lib/libzephio.so
	install -Dm755 $(WIDGET_CORE_SOLIB) $(DESTDIR)/usr/local/lib/$(notdir $(WIDGET_CORE_SOLIB))
	ln -sf $(notdir $(WIDGET_CORE_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets-core.so
	install -Dm755 $(WIDGET_ADV_SOLIB) $(DESTDIR)/usr/local/lib/$(notdir $(WIDGET_ADV_SOLIB))
	ln -sf $(notdir $(WIDGET_ADV_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets-adv.so
	install -Dm755 $(WIDGET_TEXT_SOLIB) $(DESTDIR)/usr/local/lib/$(notdir $(WIDGET_TEXT_SOLIB))
	ln -sf $(notdir $(WIDGET_TEXT_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets-text.so
	install -Dm755 $(WIDGET_EXTRA_SOLIB) $(DESTDIR)/usr/local/lib/$(notdir $(WIDGET_EXTRA_SOLIB))
	ln -sf $(notdir $(WIDGET_EXTRA_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets-extra.so
	install -Dm755 $(WIDGETS_SOLIB) $(DESTDIR)/usr/local/lib/$(notdir $(WIDGETS_SOLIB))
	ln -sf $(notdir $(WIDGETS_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets.so
	install -d $(DESTDIR)/usr/local/include/zephio
	install -m644 include/*.h $(DESTDIR)/usr/local/include/zephio/
	install -Dm644 zephio.pc $(DESTDIR)/usr/local/lib/pkgconfig/zephio.pc
	@ldconfig || true

uninstall:
	rm -f $(DESTDIR)/usr/local/lib/$(notdir $(SOLIB)) $(DESTDIR)/usr/local/lib/libzephio.so
	rm -f $(DESTDIR)/usr/local/lib/$(notdir $(WIDGET_CORE_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets-core.so
	rm -f $(DESTDIR)/usr/local/lib/$(notdir $(WIDGET_ADV_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets-adv.so
	rm -f $(DESTDIR)/usr/local/lib/$(notdir $(WIDGET_TEXT_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets-text.so
	rm -f $(DESTDIR)/usr/local/lib/$(notdir $(WIDGET_EXTRA_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets-extra.so
	rm -f $(DESTDIR)/usr/local/lib/$(notdir $(WIDGETS_SOLIB)) $(DESTDIR)/usr/local/lib/libzephio-widgets.so
	rm -f $(DESTDIR)/usr/local/include/zephio/*.h
	rm -f $(DESTDIR)/usr/local/lib/pkgconfig/zephio.pc
