# Getting Started

This guide walks through building Zephio, running your first TUI application, and troubleshooting common issues.

## Prerequisites

- GCC or Clang (C11 support)
- GNU Make
- A terminal emulator with 256-color or truecolor support (xterm, kitty, Alacritty, iTerm2, etc.)

## Building

```sh
git clone <repo-url> && cd zephio
make                # Release build: lib/libzephio.so + examples
make DEBUG=1        # Debug build with AddressSanitizer + UBSan
make examples       # Build examples only
make clean          # Remove build/ and lib/
```

Output:
- `lib/libzephio.so` — shared library
- `lib/libzephio-widgets.so` — shared widget library
- `build/*` — compiled example binaries

The static library `lib/libzephio.a` is also built for compatibility.

### Running the Examples

```sh
./build/hello             # Low-level ANSI demo
./build/boxes             # Screen buffer and box drawing
./build/layout_dashboard  # Nested layout with resize
./build/widgets_demo      # Interactive widgets with focus cycling
./build/mouse_demo        # Mouse support (high-level API)
```

## Project Structure

```
zephio-framework/
├── include/       Public API headers (36 files)
├── src/           Implementation (.c)
├── examples/      Ready-to-run demos
├── tests/         Unit tests
├── docs/          Documentation (Doxygen, architecture, guides)
├── lib/           Build output (libzephio.a)
├── build/         Build output (binaries)
├── Makefile
├── README.md
└── roadmap.md
```

## First Program: Low-Level API

The simplest Zephio program uses the ANSI helpers directly. This is useful for scripts, demos, or when you need fine-grained terminal control.

```c
#include "zephio.h"
#include "zephio_ansi.h"

int main(void) {
    ZephioContext ctx;
    if (zephio_init(&ctx) != ZEPHIO_OK) return 1;

    ZephioSize size;
    zephio_get_size(&ctx, &size);

    ansi_set_bold();
    ansi_set_fg(2);
    ansi_write_at(size.rows / 2, (size.cols - 12) / 2,
                  "Hello, TUI!", 12);
    ansi_reset();

    getchar();
    zephio_shutdown(&ctx);
    return 0;
}
```

**What happens here:**

1. `zephio_init()` enables raw mode, enters the alternate screen buffer, hides the cursor, and registers cleanup handlers.
2. `zephio_get_size()` queries the terminal dimensions.
3. `ansi_write_at()` positions the cursor and writes text.
4. `getchar()` blocks until any key is pressed.
5. `zephio_shutdown()` restores the terminal to its original state.

Compile and run:

```sh
gcc -std=c11 -Iinclude -o hello hello.c -Llib -lzephio -lm
./hello
```

## First Program: Screen Buffer API

For more complex applications, use the double-buffered screen. It eliminates flicker by only writing changed cells to the terminal.

```c
#define _POSIX_C_SOURCE 200809L
#include "zephio.h"
#include "zephio_input.h"
#include "zephio_screen.h"

static void draw_frame(ZephioContext *ctx, int rows, int cols) {
    zephio_screen_clear(ctx);
    zephio_screen_fill(ctx, 0, 0, cols, 1, " ",
        ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_write(ctx, 0, 2, "My First Screen App",
        ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_render(ctx);
}

static int on_input(const ZephioEvent *ev, void *ud) {
    if (ev->key == ZEPHIO_EVENT_RESIZE) {
        zephio_screen_resize(&ctx, ev->size.rows, ev->size.cols);
        draw_frame(&ctx, ev->size.rows, ev->size.cols);
        return 0;
    }
    if (ev->key == ZEPHIO_KEY_ESCAPE || ev->codepoint == 'q') return 1;
    return 0;
}

int main(void) {
    ZephioContext ctx;
    if (zephio_init(&ctx) != ZEPHIO_OK) return 1;
    zephio_input_init(&ctx);
    ZephioSize size = zephio_screen_size(&ctx);
    draw_frame(&ctx, size.rows, size.cols);
    zephio_input_loop(&ctx, on_input, NULL);
    zephio_input_shutdown(&ctx);
    zephio_shutdown(&ctx);
    return 0;
}
```

**Key concepts:**

- `zephio_screen_clear()` fills the back buffer with spaces.
- `zephio_screen_write()` / `zephio_screen_fill()` draw into the back buffer.
- `zephio_screen_render()` diffs front vs. back and writes only changed cells.
- `zephio_input_loop()` blocks and calls your callback for each event.

## First Program: High-Level API (ZephioApp)

For real applications with widgets, use `ZephioApp`. It manages init, the event loop, and shutdown for you.

```c
#include "zephio.h"
#include "zephio_app.h"
#include "zephio_label.h"
#include "zephio_button.h"
#include "zephio_widget.h"

typedef struct {
    ZephioWidget root;
    ZephioLabel  msg;
    ZephioButton btn;
} App;

static int on_init(ZephioApp *app, void *ud) {
    App *a = (App *)ud;
    ZephioSize sz = zephio_screen_size(&app->ctx);
    zephio_widget_init(&a->root, 0, 0, sz.cols, sz.rows, NULL, NULL);
    zephio_label_init(&a->msg, 2, 2, 30, 1, "Press the button below");
    zephio_button_init(&a->btn, 2, 4, 12, 1, "Click Me");
    a->btn.base.focusable = 1;
    zephio_widget_add_child(&a->root, &a->msg.base);
    zephio_widget_add_child(&a->root, &a->btn.base);
    return 0;
}

static int on_render(ZephioApp *app, void *ud) {
    App *a = (App *)ud;
    zephio_screen_clear(&app->ctx);
    zephio_widget_render(&a->root);
    zephio_screen_render(&app->ctx);
    return 0;
}

static int on_input(ZephioApp *app, const ZephioEvent *ev, void *ud) {
    if (ev->key == ZEPHIO_KEY_ESCAPE) return 1;
    ZephioWidget *f = zephio_widget_get_focused(&((App*)ud)->root);
    if (f) zephio_widget_handle_input(f, ev);
    return 0;
}

int main(void) {
    App app = {0};
    ZephioAppConfig cfg = {
        .on_init      = on_init,
        .on_render    = on_render,
        .on_input     = on_input,
        .user_data    = &app,
        .tick_rate_ms = 50
    };
    ZephioApp *a = zephio_app_new(&cfg);
    zephio_app_run(a);
    zephio_app_free(a);
    return 0;
}
```

**Key concepts:**

- `ZephioAppConfig` holds lifecycle callbacks: `on_init`, `on_render`, `on_input`, `on_resize`, `on_shutdown`, `on_mouse`.
- `zephio_app_new()` calls `zephio_init()` and your `on_init` callback.
- `zephio_app_run()` enters the event loop. It handles resize, keyboard, and mouse events automatically.
- Return `1` from `on_input` to stop the loop.
- Widgets are created on the stack and added to a widget tree via `zephio_widget_add_child()`.

## Linking Against Zephio

After building, you can link against the shared libraries using `pkg-config`:

```sh
gcc -std=c11 $(pkg-config --cflags --libs zephio) myapp.c -o myapp
```

Or manually:

```sh
gcc -std=c11 -I/path/to/zephio/include -o myapp myapp.c \
    -L/path/to/zephio/lib -lzephio -lzephio-widgets -lm
```

Or from within the project:

```sh
gcc -std=c11 -Iinclude -o myapp myapp.c -Llib -lzephio -lzephio-widgets -lm
```

## Troubleshooting

### "zephio_init failed: 2" (TUI_ERR_TCGETATTR)

The framework cannot read terminal attributes. This happens when stdin is not a terminal (e.g., piped input). Make sure you run the program directly in a terminal, not via `echo | ./myapp`.

### "zephio_init failed: 3" (TUI_ERR_TCSETATTR)

The framework cannot set raw mode. This can occur in some SSH sessions or container environments. Ensure your terminal emulator is correctly configured.

### Garbled output or missing colors

1. Check your terminal's color support: `echo $TERM` should report something like `xterm-256color` or `xterm-kitty`.
2. Force 256-color mode: `export TERM=xterm-256color`.
3. Truecolor (RGB) requires a terminal with 24-bit color support. The framework auto-detects via the `COLORTERM` environment variable.

### Screen not restored after crash

`zephio_init()` registers `atexit()` and signal handlers (SIGINT, SIGTERM, SIGQUIT) to ensure `zephio_shutdown()` runs. If the terminal is still broken:

```sh
reset
```

### Build errors: unknown type names

Make sure `-Iinclude` is in your compiler flags. All public headers are in `include/`.

### Shared library not found

If you encounter issues with shared libraries not being found after installation, run:

```sh
sudo ldconfig
```

This updates the dynamic linker cache to include the newly installed libraries in `/usr/local/lib/`.

### Mouse not working

Mouse tracking requires a terminal that supports SGR mouse mode (1006). Most modern terminals support this. If mouse events don't arrive, check your terminal settings or try a different emulator.

## Next Steps

- [Architecture Overview](ARCHITECTURE.md) — Module map, rendering pipeline, event flow
- [Widget Development](WIDGET_DEVELOPMENT.md) — Create custom widgets
- [Layout System](LAYOUT.md) — Fixed, Fill, Auto, weighted layouts
- [Styling & Theming](STYLING.md) — Colors, themes, truecolor
- [Migrating from ncurses](MIGRATION_NCURSES.md) — Cheatsheet for ncurses users
