# Migrating from ncurses to Zephio

This guide helps ncurses users transition to Zephio by mapping familiar ncurses concepts and functions to their Zephio equivalents.

## Concept Mapping

| ncurses Concept | Zephio Equivalent |
|---|---|
| `WINDOW` | `TuiWidget` |
| `stdscr` | Root `TuiWidget` + `TuiScreen` |
| Panels / `PANEL` | Overlay stack (`tui_app_push_overlay`) |
| Attributes (`A_BOLD`, etc.) | `TuiAttr` bitmask (`TUI_ATTR_BOLD`, etc.) |
| Color pairs (`init_pair`) | `TuiStyle` / `TuiColor` (direct fg + bg, no pair index) |
| `attron` / `attroff` | Pass `TuiAttr` to write/fill functions |
| Menus / Forms | Built-in widgets: `TuiMenuBar`, `TuiContextMenu`, `TuiInputField`, `TuiList` |
| `getch()` | `tui_input_poll()` / event loop with callbacks |

## Initialization & Cleanup

### ncurses

```c
#include <ncurses.h>

int main(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);
    curs_set(0);
    start_color();

    // ...

    endwin();
    return 0;
}
```

### Zephio

```c
#include "tui.h"

int main(void) {
    if (tui_init() != TUI_OK) return 1;

    // ...

    tui_shutdown();
    return 0;
}
```

`tui_init()` handles all of the following in one call:
- Raw mode (equivalent to `cbreak()` + `noecho()`)
- Alternate screen buffer
- Cursor hiding (`curs_set(0)`)
- Signal handlers and `atexit()` cleanup

## Drawing Text

### ncurses

```c
mvprintw(row, col, "Hello, %s!", name);
mvaddch(row, col, ACS_ULCORNER);
attron(A_BOLD);
mvprintw(row, col, "Bold text");
attroff(A_BOLD);
```

### Zephio (low-level)

```c
tui_screen_write(row, col, "Hello!", fg, bg, TUI_ATTR_NONE);
tui_screen_set_cell(row, col, "\xe2\x95\x94", fg, bg, TUI_ATTR_NONE);  // Unicode corner
tui_screen_write(row, col, "Bold text", fg, bg, TUI_ATTR_BOLD);
```

### Zephio (with TuiStyle)

```c
TuiStyle style = TUI_STYLE(15, 0, TUI_ATTR_BOLD);
tui_style_write(row, col, "Bold text", &style);
```

Key differences:
- No separate `printw` and `addstr` — `tui_screen_write()` handles all text output.
- Colors are passed directly, not via `init_pair()` / `COLOR_PAIR()`.
- Attributes are combined with colors in one call.
- Box characters are UTF-8 strings, not `ACS_*` macros.

## Colors

### ncurses

```c
start_color();
init_pair(1, COLOR_RED, COLOR_BLACK);
attron(COLOR_PAIR(1));
mvprintw(row, col, "Red text");
attroff(COLOR_PAIR(1));
```

### Zephio

```c
tui_screen_write(row, col, "Red text",
    TUI_COLOR_INDEX(1),    // fg: red (index 1)
    TUI_COLOR_INDEX(0),    // bg: black (index 0)
    TUI_ATTR_NONE);
```

Zephio has no color pair limit. Each cell stores its own foreground and background color independently. No `init_pair()` call is needed.

### Truecolor (no ncurses equivalent)

```c
tui_screen_write(row, col, "Custom color",
    TUI_COLOR_RGB(255, 100, 50),    // fg: orange
    TUI_COLOR_RGB(30, 30, 46),      // bg: dark blue
    TUI_ATTR_NONE);
```

## Box Drawing

### ncurses

```c
WINDOW *win = newwin(height, width, row, col);
box(win, 0, 0);
wrefresh(win);
```

### Zephio (screen-level)

```c
tui_screen_box_single(row, col, width, height, fg, bg, TUI_ATTR_NONE);
tui_screen_box_double(row, col, width, height, fg, bg, TUI_ATTR_NONE);
```

### Zephio (TuiBox widget)

```c
TuiBox box;
tui_box_init(&box, x, y, width, height, TUI_BOX_SINGLE);
tui_box_set_title(&box, "My Panel");
tui_box_set_colors(&box, fg, bg);
```

## Input Handling

### ncurses

```c
int ch = getch();
switch (ch) {
    case KEY_UP:    // handle up
    case KEY_DOWN:  // handle down
    case 'q':       // quit
    case 27:        // escape
}
```

### Zephio (polling)

```c
TuiEvent event;
tui_input_poll(&event);

if (event.key == TUI_KEY_UP)          { /* up */ }
if (event.key == TUI_KEY_DOWN)        { /* down */ }
if (event.codepoint == 'q')           { /* quit */ }
if (event.key == TUI_KEY_ESCAPE)      { /* escape */ }
```

### Zephio (event loop)

```c
int on_input(const TuiEvent *ev, void *ud) {
    if (ev->key == TUI_KEY_ESCAPE) return 1;
    if (ev->key == TUI_KEY_UP)     { /* handle up */ }
    return 0;
}

tui_input_loop(on_input, NULL);
```

Key differences:
- Zephio uses an event struct (`TuiEvent`) instead of a single integer.
- Modifiers are separate: `event->modifiers & TUI_MOD_CTRL`.
- Unicode input comes as `event->codepoint` (UTF-32).
- Resize events are handled via `ev->key == TUI_EVENT_RESIZE`.

## Refresh / Rendering

### ncurses

```c
wrefresh(stdscr);
// or
wnoutrefresh(stdscr);
doupdate();
```

### Zephio

```c
tui_screen_clear();      // Clear back buffer
// ... draw into back buffer ...
tui_screen_render();     // Diff and flush to terminal
```

Zephio uses double buffering by default. `tui_screen_render()` compares front and back buffers and only sends changed cells to the terminal. No need for `wnoutrefresh` / `doupdate` separation.

## Mouse

### ncurses

```c
mousemask(ALL_MOUSE_EVENTS, NULL);
MEVENT mevent;
int ch = getch();
if (ch == KEY_MOUSE) {
    getmouse(&mevent);
    // mevent.x, mevent.y, mevent.bstate
}
```

### Zephio

```c
// Mouse events arrive via the input loop
int on_input(const TuiEvent *ev, void *ud) {
    if (ev->key == TUI_EVENT_MOUSE) {
        int row = ev->mouse.row;
        int col = ev->mouse.col;
        if (ev->mouse.action == TUI_MOUSE_PRESS &&
            ev->mouse.button == TUI_MOUSE_BTN_LEFT) {
            // Left click at (row, col)
        }
    }
    return 0;
}
```

Mouse is enabled automatically by `tui_app_new()` or manually via `tui_mouse_enable()`.

## Windows → Widgets

### ncurses

```c
WINDOW *win = newwin(10, 40, 5, 10);
mvwprintw(win, 1, 1, "Content");
wrefresh(win);
delwin(win);
```

### Zephio

```c
TuiWidget widget;
tui_widget_init(&widget, 10, 5, 40, 10, NULL, NULL);
// In render:
tui_screen_write(widget.abs_y + 1, widget.abs_x + 1, "Content",
    fg, bg, TUI_ATTR_NONE);
```

For interactive widgets, use the built-in types:

```c
TuiLabel label;
tui_label_init(&label, 10, 5, 40, 1, "Content");
tui_label_set_colors(&label, fg, bg);
tui_widget_add_child(&root, &label.base);
```

## Complete Cheatsheet

| ncurses | Zephio | Notes |
|---|---|---|
| `initscr()` | `tui_init()` | Also sets up signal handlers |
| `endwin()` | `tui_shutdown()` | Restores terminal state |
| `cbreak()` | *(included in `tui_init()`)* | |
| `noecho()` | *(included in `tui_init()`)* | |
| `keypad(stdscr, TRUE)` | *(always enabled)* | |
| `curs_set(0)` | *(included in `tui_init()`)* | |
| `start_color()` | *(always enabled)* | No init needed |
| `init_pair(n, fg, bg)` | *(not needed)* | Pass colors directly |
| `COLOR_PAIR(n)` | `TUI_COLOR_INDEX(n)` | Direct color, no pair index |
| `getch()` | `tui_input_poll(&ev)` | Returns event struct |
| `mvprintw(r,c,fmt,...)` | `tui_screen_write(r,c,text,fg,bg,attr)` | No printf-style formatting |
| `mvaddch(r,c,ch)` | `tui_screen_set_cell(r,c,ch,fg,bg,attr)` | UTF-8 char string |
| `attron(A_BOLD)` | `TUI_ATTR_BOLD` in write call | Combined with colors |
| `attroff(A_BOLD)` | *(not needed)* | Per-call attributes |
| `newwin(h,w,r,c)` | `tui_widget_init(w,c,r,w,h,...)` | Widget-based |
| `delwin(win)` | `tui_widget_destroy(w)` | Recursive tree cleanup |
| `box(win,0,0)` | `tui_screen_box_single(...)` | Or `TuiBox` widget |
| `wrefresh(win)` | `tui_screen_render()` | Double-buffered diff |
| `clear()` | `tui_screen_clear()` | Clears back buffer |
| `getmaxyx(stdscr,r,c)` | `tui_screen_size()` | Returns `TuiSize` |
| `mousemask(...)` | `tui_mouse_enable()` | SGR mode by default |
| `getmouse(&me)` | `ev->mouse` in event | Part of `TuiEvent` |
| `KEY_UP` | `TUI_KEY_UP` | Same naming convention |
| `KEY_DOWN` | `TUI_KEY_DOWN` | |
| `KEY_F(n)` | `TUI_KEY_F1`..`TUI_KEY_F12` | Explicit constants |
| `KEY_RESIZE` | `TUI_EVENT_RESIZE` | New size in `ev->size` |
| `ACS_ULCORNER` | `"\xe2\x95\x94"` | UTF-8 Unicode directly |
| `new_panel(win)` | `tui_app_push_overlay(...)` | Z-ordered overlay stack |

## Migration Checklist

1. Replace `initscr()`/`endwin()` with `tui_init()`/`tui_shutdown()`.
2. Replace `getch()` with `tui_input_poll()` or `tui_input_loop()`.
3. Replace `printw()`/`mvprintw()` with `tui_screen_write()`.
4. Replace `init_pair()`/`COLOR_PAIR()` with direct `TuiColor` values.
5. Replace `WINDOW*` with `TuiWidget` and the widget tree.
6. Replace `wrefresh()`/`doupdate()` with `tui_screen_clear()` + draw + `tui_screen_render()`.
7. Replace mouse handling with `TUI_EVENT_MOUSE` in the event loop.
8. Use built-in widgets (Button, List, InputField, etc.) instead of manual ncurses forms/menus.

## Next Steps

- [Getting Started](GETTING_STARTED.md) — Build your first Zephio app
- [Widget Development](WIDGET_DEVELOPMENT.md) — Create custom widgets
- [Layout System](LAYOUT.md) — Arrange widgets with constraints
- [Architecture Overview](ARCHITECTURE.md) — Rendering pipeline and event flow
