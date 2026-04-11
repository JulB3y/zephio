# Getting Started

This guide walks through building Zephio, running your first TUI application, and troubleshooting common issues.

## Prerequisites

- GCC or Clang (C11 support)
- GNU Make
- A terminal emulator with 256-color or truecolor support (xterm, kitty, Alacritty, iTerm2, etc.)

## Building

```sh
git clone <repo-url> && cd zephio
make                # Release build: lib/libtui.a + examples
make DEBUG=1        # Debug build with AddressSanitizer + UBSan
make examples       # Build examples only
make clean          # Remove build/ and lib/
```

Output:
- `lib/libtui.a` — static library
- `build/*` — compiled example binaries

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
├── lib/           Build output (libtui.a)
├── build/         Build output (binaries)
├── Makefile
├── README.md
└── roadmap.md
```

## First Program: Low-Level API

The simplest Zephio program uses the ANSI helpers directly. This is useful for scripts, demos, or when you need fine-grained terminal control.

```c
#include "tui.h"
#include "tui_ansi.h"

int main(void) {
    if (tui_init() != TUI_OK) return 1;

    TuiSize size;
    tui_get_size(&size);

    ansi_set_bold();
    ansi_set_fg(2);
    ansi_write_at(size.rows / 2, (size.cols - 12) / 2,
                  "Hello, TUI!", 12);
    ansi_reset();

    getchar();
    tui_shutdown();
    return 0;
}
```

**What happens here:**

1. `tui_init()` enables raw mode, enters the alternate screen buffer, hides the cursor, and registers cleanup handlers.
2. `tui_get_size()` queries the terminal dimensions.
3. `ansi_write_at()` positions the cursor and writes text.
4. `getchar()` blocks until any key is pressed.
5. `tui_shutdown()` restores the terminal to its original state.

Compile and run:

```sh
gcc -std=c11 -Iinclude -o hello hello.c -Llib -ltui -lm
./hello
```

## First Program: Screen Buffer API

For more complex applications, use the double-buffered screen. It eliminates flicker by only writing changed cells to the terminal.

```c
#define _POSIX_C_SOURCE 200809L
#include "tui.h"
#include "tui_input.h"
#include "tui_screen.h"

static void draw_frame(int rows, int cols) {
    tui_screen_clear();
    tui_screen_fill(0, 0, cols, 1, " ",
        TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
    tui_screen_write(0, 2, "My First Screen App",
        TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
    tui_screen_render();
}

static int on_input(const TuiEvent *ev, void *ud) {
    if (ev->key == TUI_EVENT_RESIZE) {
        tui_screen_resize(ev->size.rows, ev->size.cols);
        draw_frame(ev->size.rows, ev->size.cols);
        return 0;
    }
    if (ev->key == TUI_KEY_ESCAPE || ev->codepoint == 'q') return 1;
    return 0;
}

int main(void) {
    if (tui_init() != TUI_OK) return 1;
    tui_input_init();
    TuiSize size = tui_screen_size();
    draw_frame(size.rows, size.cols);
    tui_input_loop(on_input, NULL);
    tui_input_shutdown();
    tui_shutdown();
    return 0;
}
```

**Key concepts:**

- `tui_screen_clear()` fills the back buffer with spaces.
- `tui_screen_write()` / `tui_screen_fill()` draw into the back buffer.
- `tui_screen_render()` diffs front vs. back and writes only changed cells.
- `tui_input_loop()` blocks and calls your callback for each event.

## First Program: High-Level API (TuiApp)

For real applications with widgets, use `TuiApp`. It manages init, the event loop, and shutdown for you.

```c
#include "tui.h"
#include "tui_app.h"
#include "tui_label.h"
#include "tui_button.h"
#include "tui_widget.h"

typedef struct {
    TuiWidget root;
    TuiLabel  msg;
    TuiButton btn;
} App;

static int on_init(TuiApp *app, void *ud) {
    App *a = (App *)ud;
    TuiSize sz = tui_screen_size();
    tui_widget_init(&a->root, 0, 0, sz.cols, sz.rows, NULL, NULL);
    tui_label_init(&a->msg, 2, 2, 30, 1, "Press the button below");
    tui_button_init(&a->btn, 2, 4, 12, 1, "Click Me");
    a->btn.base.focusable = 1;
    tui_widget_add_child(&a->root, &a->msg.base);
    tui_widget_add_child(&a->root, &a->btn.base);
    return 0;
}

static int on_render(TuiApp *app, void *ud) {
    App *a = (App *)ud;
    tui_screen_clear();
    tui_widget_render(&a->root);
    tui_screen_render();
    return 0;
}

static int on_input(TuiApp *app, const TuiEvent *ev, void *ud) {
    if (ev->key == TUI_KEY_ESCAPE) return 1;
    TuiWidget *f = tui_widget_get_focused(&((App *)ud)->root);
    if (f) tui_widget_handle_input(f, ev);
    return 0;
}

int main(void) {
    App app = {0};
    TuiAppConfig cfg = {
        .on_init      = on_init,
        .on_render    = on_render,
        .on_input     = on_input,
        .user_data    = &app,
        .tick_rate_ms = 50
    };
    TuiApp *a = tui_app_new(&cfg);
    tui_app_run(a);
    tui_app_free(a);
    return 0;
}
```

**Key concepts:**

- `TuiAppConfig` holds lifecycle callbacks: `on_init`, `on_render`, `on_input`, `on_resize`, `on_shutdown`, `on_mouse`.
- `tui_app_new()` calls `tui_init()` and your `on_init` callback.
- `tui_app_run()` enters the event loop. It handles resize, keyboard, and mouse events automatically.
- Return `1` from `on_input` to stop the loop.
- Widgets are created on the stack and added to a widget tree via `tui_widget_add_child()`.

## Linking Against Zephio

After building:

```sh
gcc -std=c11 -I/path/to/zephio/include -o myapp myapp.c \
    -L/path/to/zephio/lib -ltui -lm
```

Or from within the project:

```sh
gcc -std=c11 -Iinclude -o myapp myapp.c -Llib -ltui -lm
```

## Troubleshooting

### "tui_init failed: 2" (TUI_ERR_TCGETATTR)

The framework cannot read terminal attributes. This happens when stdin is not a terminal (e.g., piped input). Make sure you run the program directly in a terminal, not via `echo | ./myapp`.

### "tui_init failed: 3" (TUI_ERR_TCSETATTR)

The framework cannot set raw mode. This can occur in some SSH sessions or container environments. Ensure your terminal emulator is correctly configured.

### Garbled output or missing colors

1. Check your terminal's color support: `echo $TERM` should report something like `xterm-256color` or `xterm-kitty`.
2. Force 256-color mode: `export TERM=xterm-256color`.
3. Truecolor (RGB) requires a terminal with 24-bit color support. The framework auto-detects via the `COLORTERM` environment variable.

### Screen not restored after crash

`tui_init()` registers `atexit()` and signal handlers (SIGINT, SIGTERM, SIGQUIT) to ensure `tui_shutdown()` runs. If the terminal is still broken:

```sh
reset
```

### Build errors: unknown type names

Make sure `-Iinclude` is in your compiler flags. All public headers are in `include/`.

### Mouse not working

Mouse tracking requires a terminal that supports SGR mouse mode (1006). Most modern terminals support this. If mouse events don't arrive, check your terminal settings or try a different emulator.

## Next Steps

- [Architecture Overview](ARCHITECTURE.md) — Module map, rendering pipeline, event flow
- [Widget Development](WIDGET_DEVELOPMENT.md) — Create custom widgets
- [Layout System](LAYOUT.md) — Fixed, Fill, Auto, weighted layouts
- [Styling & Theming](STYLING.md) — Colors, themes, truecolor
- [Migrating from ncurses](MIGRATION_NCURSES.md) — Cheatsheet for ncurses users
