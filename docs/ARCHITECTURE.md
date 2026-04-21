# Zephio Architecture

## Overview

Zephio is a lightweight, portable terminal UI framework for C with zero external dependencies. It provides a widget-based model with a double-buffered rendering system, keyboard/mouse input handling, and a flexible layout engine.

## Module Map

```
┌─────────────────────────────────────────────────────────────────────┐
│                         Application Layer                            │
│  tui_app.h — Event loop, lifecycle callbacks, overlay stack        │
└───────────────────────────────┬─────────────────────────────────────┘
                                │
┌───────────────────────────────▼─────────────────────────────────────┐
│                         Widget Layer                                 │
│  tui_widget.h   — Base widget type, tree operations, focus, mouse   │
│  tui_label.h    tui_button.h  tui_input_field.h  tui_list.h        │
│  tui_container.h  tui_box.h  tui_separator.h                        │
│  tui_text_view.h  tui_textarea.h  tui_scroll_container.h           │
│  tui_dialog.h  tui_dropdown.h  tui_menubar.h  tui_context_menu.h  │
│  tui_radio.h  tui_checkbox.h  tui_progress.h                       │
│  tui_tabbar.h  tui_statusbar.h  tui_tree_view.h  tui_table.h      │
└───────────────────────────────┬─────────────────────────────────────┘
                                │
┌───────────────────────────────▼─────────────────────────────────────┐
│                         Layout Engine                                │
│  tui_layout.h — Horizontal/vertical stacking, fixed/fill/weighted   │
└───────────────────────────────┬─────────────────────────────────────┘
                                │
┌───────────────────────────────▼─────────────────────────────────────┐
│                       Rendering Layer                                │
│  tui_screen.h — Double-buffered screen, cell diff, box drawing     │
│  tui_style.h  — Themes, per-state styles, color utilities         │
│  tui_text.h   — UTF-8 text processing, word wrapping               │
│  tui_ansi.h   — ANSI escape sequence macros                         │
└───────────────────────────────┬─────────────────────────────────────┘
                                │
┌───────────────────────────────▼─────────────────────────────────────┐
│                        Input Layer                                   │
│  tui_input.h  — Keyboard parsing, TuiEvent, TuiKey, modifiers      │
│  tui_mouse.h  — Mouse event types and button definitions           │
└───────────────────────────────┬─────────────────────────────────────┘
                                │
┌───────────────────────────────▼─────────────────────────────────────┐
│                      Terminal Layer                                  │
│  tui.h          — Init/shutdown, TuiSize, TuiResult, terminal mode  │
│  tui_terminal.h — Raw mode, alternate screen, signal handling      │
└─────────────────────────────────────────────────────────────────────┘
```

## Core Data Structures

### TuiWidget

The central type. All UI elements embed `TuiWidget` as their first member:

```c
struct TuiWidget {
    int x, y;           // Position relative to parent
    int width, height;  // Dimensions
    int abs_x, abs_y;   // Computed absolute position

    int visible;
    int dirty;          // Needs re-render
    int focusable;
    int focused;
    int disabled;
    int hovered;
    int tab_index;

    TuiWidget  *parent;
    TuiWidget **children;
    int         child_count;

    const TuiTheme      *theme;
    TuiWidgetVTable     *vtable;
    void                *data;
};
```

Polymorphism is via a vtable (`TuiWidgetVTable`) with function pointers for:
- `render` — Draw the widget
- `handle_input` — Process keyboard events
- `handle_mouse` — Process mouse events
- `destroy` — Cleanup
- `on_resize` — Respond to size changes
- `on_focus` / `on_blur` — Focus notifications

### TuiScreen

Double-buffered terminal representation:

```c
typedef struct {
    TuiCell *front;   // Current displayed buffer
    TuiCell *back;    // Work buffer for next frame
    int      rows;
    int      cols;
} TuiScreen;
```

Each `TuiCell` holds a character (up to 4 bytes for UTF-8), foreground color, background color, and attribute flags.

### TuiEvent

Unified input event structure:

```c
typedef struct {
    TuiKey        key;        // Key code or ZEPHIO_EVENT_*
    int           modifiers;   // ZEPHIO_MOD_SHIFT | ZEPHIO_MOD_CTRL | ...
    uint32_t      codepoint;  // UTF-32 for printable keys
    TuiSize       size;       // For resize events
    TuiMouseEvent mouse;      // For mouse events
} TuiEvent;
```

## Rendering Pipeline

```
┌─────────────────────────────────────────────────────────────────────┐
│                         Frame Tick                                  │
│                                                                     │
│  1. tui_screen_clear()                                             │
│     └─ Fill back buffer with spaces, default colors                │
│                                                                     │
│  2. Widget tree traversal (pre-order DFS)                          │
│     └─ For each visible widget:                                   │
│         ├─ Compute abs_x, abs_y from parent chain                  │
│         ├─ Call vtable->render()                                  │
│         └─ Mark widget clean (dirty = 0)                          │
│                                                                     │
│  3. tui_screen_render()                                           │
│     └─ Diff front vs. back:                                        │
│         ├─ Compare cell by cell                                   │
│         ├─ For changed cells:                                      │
│         │   └─ Emit ANSI sequences to terminal                    │
│         └─ Swap front/back pointers                                │
└─────────────────────────────────────────────────────────────────────┘
```

The diff-based approach ensures minimal terminal output: only cells that actually changed are written, eliminating flicker.

## Event Flow

```
┌─────────────────────────────────────────────────────────────────────┐
│                     Input Event Handling                            │
│                                                                     │
│  Terminal Input ──► tui_input_poll() ──► TuiEvent                   │
│                                          │                          │
│                    ┌─────────────────────┼─────────────────────┐   │
│                    ▼                     ▼                     ▼   │
│              ZEPHIO_EVENT_RESIZE      ZEPHIO_EVENT_MOUSE        Keyboard  │
│                    │                     │                     │   │
│                    ▼                     ▼                     ▼   │
│           tui_screen_resize()   tui_widget_handle_mouse()  dispatch│
│                                        │                     │     │
│                                        ▼                     ▼     │
│                              focused/hovered widget     vtable->    │
│                                                        handle_input│
└─────────────────────────────────────────────────────────────────────┘
```

### Overlay Stack

Modal widgets (dialogs, menus) use the overlay stack in `TuiApp`:

- `tui_app_push_overlay()` — Renders widget on top, routes input exclusively
- `tui_app_pop_overlay()` — Returns to normal widget tree
- Input events always go to the topmost overlay when present

## Layout System

`TuiLayout` arranges children using constraints:

| Constraint | Behavior |
|------------|----------|
| `ZEPHIO_LAYOUT_FIXED(n)` | Exact size in cells |
| `ZEPHIO_LAYOUT_FILL` | Share remaining space equally (weight 1.0) |
| `ZEPHIO_LAYOUT_FILL_WEIGHT(w)` | Share with custom weight |
| `ZEPHIO_LAYOUT_AUTO` | Use child's natural size |

```
┌─────────────────────────────────────────┐
│ ZEPHIO_LAYOUT_VERTICAL (padding=1)         │
│ ┌─────────────────────────────────────┐ │
│ │ HEADER: ZEPHIO_LAYOUT_FIXED(3)         │ │
│ ├─────────────────────────────────────┤ │
│ │ CONTENT: ZEPHIO_LAYOUT_FILL_WEIGHT(1) │ │
│ ├─────────────────────────────────────┤ │
│ │ FOOTER: ZEPHIO_LAYOUT_FIXED(2)        │ │
│ └─────────────────────────────────────┘ │
└─────────────────────────────────────────┘
```

Call `tui_layout_recalculate()` after adding/removing children or resizing.

## Style and Theme System

### TuiStyle

```c
typedef struct {
    TuiColor fg;
    TuiColor bg;
    TuiAttr  attr;  // ZEPHIO_ATTR_BOLD | ZEPHIO_ATTR_UNDERLINE | ...
} TuiStyle;
```

### TuiTheme

Per-widget visual states:

```c
typedef struct {
    TuiStyle styles[ZEPHIO_STATE_COUNT];  // NORMAL, FOCUSED, DISABLED, ACTIVE, HOVER
} TuiTheme;
```

`tui_widget_get_style()` resolves the appropriate style based on widget state (disabled → focused → hovered → normal).

### Colors

Two color types supported:
- `ZEPHIO_COLOR_INDEX(n)` — 256-color palette (0-255)
- `ZEPHIO_COLOR_RGB(r,g,b)` — 24-bit truecolor

## Widget Hierarchy

All widgets embed `TuiWidget` as `base` or `super`:

```
TuiLayout
    ├── TuiLabel
    ├── TuiButton
    ├── TuiInputField
    ├── TuiList
    ├── TuiContainer
    ├── TuiBox
    ├── TuiSeparator
    ├── TuiTextView
    ├── TuiScrollContainer
    │       └── TuiTextArea
    ├── TuiDialog
    │       └── (content widget)
    ├── TuiDropdown
    ├── TuiMenuBar
    │       └── TuiContextMenu
    ├── TuiRadioGroup
    │       └── TuiRadio
    ├── TuiCheckbox
    ├── TuiProgress
    ├── TuiTabBar
    ├── TuiStatusBar
    ├── TuiTreeView
    └── TuiTable
```

## Build System

```
make              # Build lib/libzephio.a
make DEBUG=1      # Debug build with sanitizers
make examples     # Build examples
make test         # Run tests
make clean        # Remove build artifacts
```

The library is self-contained: `lib/libzephio.a` with no external runtime dependencies beyond libc.

## Signal Safety

`tui_shutdown()` is registered via `atexit()` and signal handlers (SIGINT, SIGTERM, SIGQUIT). The terminal is always restored to a usable state on exit, even if the program crashes.
