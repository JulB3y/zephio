# Zephio

A lightweight, portable terminal UI framework in C with **zero external dependencies**

## Features

- Raw terminal mode with alternate screen buffer
- Double-buffered, diff-based rendering (flicker-free)
- Full keyboard input parsing (special keys, modifiers, escape sequences)
- Mouse event support (click, drag, scroll, hover)
- Widget tree with parent-child hierarchy
- Focus management (Tab / Shift+Tab cycling)
- Built-in widgets: Label, Button, InputField, List, Container, Box, Separator, CheckBox, Radio, Progress, Dropdown, Dialog, MenuBar, ContextMenu, TabBar, StatusBar, Table, TreeView, TextArea, TextView, ScrollContainer
- Layout engine (vertical/horizontal stacks, weighted sizing)
- Style system with themes (256-color + truecolor RGB)
- Animation system (easing, slide/fade effects)
- Mouse support (click, drag, scroll, hover)
- Clipboard support (OSC 52)
- Automatic resize handling
- Signal-safe cleanup on exit/crash
- AddressSanitizer and UBSan support via `DEBUG=1`

## Images
![Widgets](./examples/imgs/interactive-widgets.png)

## Build

Requires `gcc` and `make`.

```sh
make              # Release build (lib/libzephio.so + examples)
make DEBUG=1      # Debug build with -g -O0 -fsanitize=address,undefined
make examples     # Build examples only
make clean        # Remove build/ and lib/
```

Output:
- `lib/libzephio.so` — shared library
- `lib/libzephio-widgets.so` — shared widget library
- `build/*` — compiled example binaries

The static library `lib/libzephio.a` is also built for compatibility.

## Quick Start

### Minimal example (low-level API)

```c
#include "zephio.h"
#include "zephio_ansi.h"

int main(void) {
    ZephioContext ctx;
    zephio_init(&ctx);

    ZephioSize size;
    zephio_get_size(&ctx, &size);

    ansi_set_fg(2);
    ansi_write_at(size.rows / 2, (size.cols - 12) / 2,
                  "Hello, TUI!", 12);
    ansi_reset();

    getchar();      /* wait for any key */
    zephio_shutdown(&ctx);
    return 0;
}
```

### App-based example (high-level API)

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
        .on_init   = on_init,
        .on_render = on_render,
        .on_input  = on_input,
        .user_data = &app,
        .tick_rate_ms = 50
    };
    ZephioApp *a = zephio_app_new(&cfg);
    zephio_app_run(a);
    zephio_app_free(a);
    return 0;
}
```

## Project Structure

```
├── include/          Public headers (API, 36 files)
│   ├── zephio.h              Core init/shutdown
│   ├── zephio_app.h          High-level app runtime (event loop, lifecycle)
│   ├── zephio_widget.h       Base widget type and tree operations
│   ├── zephio_screen.h       Double-buffered screen
│   ├── zephio_input.h        Keyboard input and event loop
│   ├── zephio_mouse.h        Mouse event types
│   ├── zephio_ansi.h         ANSI escape sequence helpers
│   ├── zephio_style.h        Themes and styles
│   ├── zephio_layout.h       Layout engine
│   ├── zephio_animation.h    Animation system
│   ├── zephio_label.h        zephio_button.h   zephio_input_field.h  zephio_list.h
│   ├── zephio_container.h    zephio_box.h      zephio_separator.h    zephio_text.h
│   ├── zephio_checkbox.h     zephio_radio.h    zephio_progress.h
│   ├── zephio_dropdown.h     zephio_dialog.h   zephio_clipboard.h
│   ├── zephio_menubar.h      zephio_context_menu.h
│   ├── zephio_tabbar.h       zephio_statusbar.h
│   ├── zephio_table.h        zephio_tree_view.h
│   ├── zephio_textarea.h     zephio_text_view.h  zephio_scroll_container.h
│   └── zephio_terminal.h
├── src/              Implementation
├── examples/         Ready-to-run demos (15 examples)
├── tests/            Unit tests
├── docs/             Doxygen API reference, architecture docs, tutorials
├── Makefile
└── roadmap.md
```

## Widget Overview

| Widget | Header | Description |
|---|---|---|
| **Label** | `zephio_label.h` | Static text display with configurable colors and attributes |
| **Button** | `zephio_button.h` | Clickable button with `on_click` callback |
| **InputField** | `zephio_input_field.h` | Text input with `on_change` / `on_submit` callbacks |
| **List** | `zephio_list.h` | Scrollable selectable list with `on_select` callback |
| **Container** | `zephio_container.h` | Background panel with configurable background color |
| **Box** | `zephio_box.h` | Bordered box with optional title |
| **Separator** | `zephio_separator.h` | Horizontal or vertical divider line |
| **CheckBox** | `zephio_checkbox.h` | Toggle checkbox with label |
| **Radio** | `zephio_radio.h` | Single-selection radio button group |
| **Progress** | `zephio_progress.h` | Horizontal progress bar (0-100%) |
| **Dropdown** | `zephio_dropdown.h` | Expandable selection dropdown |
| **Dialog** | `zephio_dialog.h` | Modal dialog with overlay |
| **MenuBar** | `zephio_menubar.h` | Horizontal menu bar with submenus |
| **ContextMenu** | `zephio_context_menu.h` | Right-click context menu |
| **TabBar** | `zephio_tabbar.h` | Tab-based page switching |
| **StatusBar** | `zephio_statusbar.h` | Segmented status bar |
| **Table** | `zephio_table.h` | Column-based data display with headers |
| **TreeView** | `zephio_tree_view.h` | Hierarchical tree with expand/collapse |
| **TextArea** | `zephio_textarea.h` | Multi-line editable text |
| **TextView** | `zephio_text_view.h` | Multi-line read-only text with word wrap |
| **ScrollContainer** | `zephio_scroll_container.h` | Scrollable widget container |
| **Clipboard** | `zephio_clipboard.h` | OSC 52 clipboard copy/paste |

All widgets embed `TuiWidget` as their first member and support the widget tree operations from `tui_widget.h` (add/remove children, render, focus, hit-testing, mouse dispatch).

## Documentation

- [Getting Started](docs/GETTING_STARTED.md) — Build, first project, troubleshooting
- [Architecture](docs/ARCHITECTURE.md) — Module map, rendering pipeline, event flow
- [Widget Development](docs/WIDGET_DEVELOPMENT.md) — Creating custom widgets
- [Layout System](docs/LAYOUT.md) — Fixed/Fill/Auto, weights, nested layouts
- [Styling & Theming](docs/STYLING.md) — Colors, themes, truecolor
- [ncurses Migration](docs/MIGRATION_NCURSES.md) — Cheatsheet: ncurses → Zephio mapping

## Linking Against Zephio

To link against the shared libraries:

```sh
gcc -std=c11 $(pkg-config --cflags --libs zephio) myapp.c -o myapp
```

Or manually:

```sh
gcc -std=c11 -I/path/to/zephio/include -o myapp myapp.c \
    -L/path/to/zephio/lib -lzephio -lzephio-widgets -lm
```

## License

MIT License - see [LICENSE](LICENSE) for details.
