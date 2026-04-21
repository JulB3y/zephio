# Widget Development Guide

This guide explains how to create custom widgets for Zephio. We build a `CounterWidget` as a complete example — a simple counter with increment/decrement buttons.

## Widget Architecture

All Zephio widgets use **struct composition**: your widget struct embeds `TuiWidget` as its first member. Polymorphism is achieved via a vtable of function pointers.

### The Pattern

```c
typedef struct {
    TuiWidget base;       // Always first — enables casting to TuiWidget*
    // ... your custom fields
} MyWidget;
```

The `TuiWidget` base provides:
- Position and size (`x`, `y`, `width`, `height`, `abs_x`, `abs_y`)
- State flags (`visible`, `focused`, `focusable`, `disabled`, `hovered`, `dirty`)
- Tree structure (`parent`, `children`, `child_count`)
- Theme and vtable pointers

## Vtable (TuiWidgetVTable)

The vtable defines the widget's behavior:

```c
typedef struct {
    void (*render)(TuiWidget *widget);
    int  (*handle_input)(TuiWidget *widget, const TuiEvent *event);
    int  (*handle_mouse)(TuiWidget *widget, const TuiMouseEvent *mouse);
    void (*destroy)(TuiWidget *widget);
    void (*on_resize)(TuiWidget *widget, int width, int height);
    void (*on_focus)(TuiWidget *widget);
    void (*on_blur)(TuiWidget *widget);
} TuiWidgetVTable;
```

Any function pointer may be `NULL` — the dispatch functions check before calling. At minimum, implement `render` and `destroy`.

## Widget Lifecycle

1. **Init** — Allocate (stack or heap) and call `tui_widget_init()`.
2. **Configure** — Set colors, callbacks, initial state.
3. **Add to tree** — `tui_widget_add_child(parent, &widget->base)`.
4. **Render** — The framework calls `vtable->render()` during tree traversal.
5. **Input** — The framework dispatches events via `vtable->handle_input()`.
6. **Destroy** — `tui_widget_destroy()` calls `vtable->destroy()` recursively.

## Example: CounterWidget

A counter with `+` and `-` buttons, displayed as text.

### Header (`counter_widget.h`)

```c
#ifndef COUNTER_WIDGET_H
#define COUNTER_WIDGET_H

#include "zephio_widget.h"

typedef struct {
    TuiWidget base;
    int       count;
    int       min;
    int       max;
    TuiColor  fg;
    TuiColor  bg;
    TuiColor  fg_focused;
    TuiColor  bg_focused;
} CounterWidget;

TuiResult counter_widget_init(CounterWidget *cw, int x, int y,
                               int width, int min, int max);
void counter_widget_set_value(CounterWidget *cw, int value);
int  counter_widget_get_value(CounterWidget *cw);

#endif
```

### Implementation (`counter_widget.c`)

```c
#include "counter_widget.h"
#include "zephio_screen.h"
#include "zephio_style.h"
#include <stdio.h>
#include <stdlib.h>

static void counter_render(TuiWidget *widget) {
    CounterWidget *cw = (CounterWidget *)widget;

    TuiColor fg = cw->base.focused ? cw->fg_focused : cw->fg;
    TuiColor bg = cw->base.focused ? cw->bg_focused : cw->bg;

    tui_screen_fill(widget->abs_y, widget->abs_x,
                    widget->width, widget->height,
                    " ", fg, bg, ZEPHIO_ATTR_NONE);

    char buf[32];
    int len = snprintf(buf, sizeof(buf), "[-] %d [+]", cw->count);

    int text_x = widget->abs_x + (widget->width - len) / 2;
    int text_y = widget->abs_y + widget->height / 2;
    if (text_x >= 0 && text_y >= 0) {
        tui_screen_write(text_y, text_x, buf, fg, bg, ZEPHIO_ATTR_BOLD);
    }
}

static int counter_handle_input(TuiWidget *widget, const TuiEvent *event) {
    CounterWidget *cw = (CounterWidget *)widget;

    if (event->codepoint == '+' || event->key == ZEPHIO_KEY_UP) {
        if (cw->count < cw->max) {
            cw->count++;
            tui_widget_set_dirty(widget);
        }
        return 1;
    }
    if (event->codepoint == '-' || event->key == ZEPHIO_KEY_DOWN) {
        if (cw->count > cw->min) {
            cw->count--;
            tui_widget_set_dirty(widget);
        }
        return 1;
    }
    return 0;
}

static void counter_destroy(TuiWidget *widget) {
    // No heap allocations inside CounterWidget beyond what TuiWidget manages.
    // If we had allocated strings or arrays, we'd free them here.
    (void)widget;
}

static TuiWidgetVTable counter_vtable = {
    .render       = counter_render,
    .handle_input = counter_handle_input,
    .handle_mouse = NULL,
    .destroy      = counter_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult counter_widget_init(CounterWidget *cw, int x, int y,
                               int width, int min, int max) {
    TuiResult res = tui_widget_init(&cw->base, x, y, width, 1,
                                     &counter_vtable, NULL);
    if (res != ZEPHIO_OK) return res;

    cw->count       = 0;
    cw->min         = min;
    cw->max         = max;
    cw->fg          = ZEPHIO_COLOR_INDEX(15);
    cw->bg          = ZEPHIO_COLOR_INDEX(234);
    cw->fg_focused  = ZEPHIO_COLOR_INDEX(0);
    cw->bg_focused  = ZEPHIO_COLOR_INDEX(12);
    cw->base.focusable = 1;

    return ZEPHIO_OK;
}

void counter_widget_set_value(CounterWidget *cw, int value) {
    if (value < cw->min) value = cw->min;
    if (value > cw->max) value = cw->max;
    cw->count = value;
    tui_widget_set_dirty(&cw->base);
}

int counter_widget_get_value(CounterWidget *cw) {
    return cw->count;
}
```

### Using the CounterWidget

```c
#include "tui.h"
#include "zephio_input.h"
#include "zephio_screen.h"
#include "counter_widget.h"

static TuiWidget root;
static CounterWidget counter;

static void draw_frame(int rows, int cols) {
    tui_screen_clear();
    tui_screen_write(0, 2, "Counter Demo  |  +/- to change  |  q to quit",
        ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    tui_widget_render(&root);
    tui_screen_render();
}

static int on_input(const TuiEvent *ev, void *ud) {
    if (ev->key == ZEPHIO_EVENT_RESIZE) {
        tui_screen_resize(ev->size.rows, ev->size.cols);
        draw_frame(ev->size.rows, ev->size.cols);
        return 0;
    }
    if (ev->key == ZEPHIO_KEY_ESCAPE || ev->codepoint == 'q') return 1;

    if (ev->key == ZEPHIO_KEY_TAB) {
        tui_widget_focus_next(&root);
        draw_frame(0, 0);
        return 0;
    }

    TuiWidget *focused = tui_widget_get_focused(&root);
    if (focused) tui_widget_handle_input(focused, ev);

    TuiSize sz = tui_screen_size();
    draw_frame(sz.rows, sz.cols);
    return 0;
}

int main(void) {
    tui_init();
    tui_input_init();
    TuiSize sz = tui_screen_size();

    tui_widget_init(&root, 0, 0, sz.cols, sz.rows, NULL, NULL);
    counter_widget_init(&counter, (sz.cols - 14) / 2, sz.rows / 2,
                        14, -10, 10);
    tui_widget_add_child(&root, &counter.base);
    tui_widget_focus_next(&root);

    draw_frame(sz.rows, sz.cols);
    tui_input_loop(on_input, NULL);

    tui_widget_destroy(&counter.base);
    tui_widget_remove_all_children(&root);
    tui_input_shutdown();
    tui_shutdown();
    return 0;
}
```

## Guidelines

### Rendering

- Use `widget->abs_x` and `widget->abs_y` (absolute positions), not `x`/`y` (relative). The framework computes absolute positions before calling `render()`.
- Write to the back buffer via `tui_screen_write()`, `tui_screen_fill()`, or `tui_screen_set_cell()`. Use the styled helpers from `tui_style.h` (`tui_style_write()`, `tui_style_fill()`) when working with `TuiStyle` objects.
- Always check bounds: `widget->width` and `widget->height` may be smaller than expected on small terminals.

### Input Handling

- Return `1` from `handle_input` if the event was consumed, `0` otherwise. This lets parent widgets handle unprocessed events.
- Check `widget->disabled` at the top of your handler — disabled widgets should ignore input.
- Use `tui_widget_set_dirty(widget)` after state changes to trigger a re-render.

### Memory Management

- `tui_widget_init()` does not allocate the widget struct itself — it only initializes the children array. You manage the widget's storage (stack or heap).
- `tui_widget_destroy()` calls `vtable->destroy()` for each widget in the tree and frees the children arrays. It does **not** free the widget structs.
- Free any dynamically allocated fields (strings, buffers) in your `destroy` function.

### Focus

- Set `widget->focusable = 1` to include the widget in Tab/Shift+Tab cycling.
- Implement `on_focus` / `on_blur` to react to focus changes (e.g., change colors).
- `tui_widget_get_style()` automatically resolves the correct style for the current state (normal, focused, hovered, disabled).

### Mouse

- Implement `handle_mouse` to respond to clicks and hover.
- Use `tui_widget_contains(widget, mouse->row, mouse->col)` to check if a mouse event is inside the widget.
- The framework's `tui_widget_handle_mouse()` does hit-testing and dispatch automatically for widget trees.

## Built-in Widget Reference

| Widget | Header | Key Features |
|--------|--------|-------------|
| Label | `tui_label.h` | Static text, configurable colors/attributes |
| Button | `tui_button.h` | Click callback, focused/unfocused states |
| InputField | `tui_input_field.h` | Text input, `on_change`/`on_submit` callbacks |
| List | `tui_list.h` | Scrollable list, `on_select` callback |
| Container | `tui_container.h` | Background panel |
| Box | `tui_box.h` | Bordered panel with optional title |
| Separator | `tui_separator.h` | Horizontal/vertical divider |
| CheckBox | `tui_checkbox.h` | Toggle state |
| Radio | `tui_radio.h` | Single-selection group |
| Progress | `tui_progress.h` | Progress bar (0-100%) |
| Dropdown | `tui_dropdown.h` | Expandable selection list |
| Dialog | `tui_dialog.h` | Modal dialog with overlay |
| MenuBar | `tui_menubar.h` | Horizontal menu bar |
| ContextMenu | `tui_context_menu.h` | Right-click context menu |
| TabBar | `tui_tabbar.h` | Tab-based page switching |
| StatusBar | `tui_statusbar.h` | Segmented status bar |
| Table | `tui_table.h` | Column-based data display |
| TreeView | `tui_tree_view.h` | Hierarchical expand/collapse |
| TextArea | `tui_textarea.h` | Multi-line editable text |
| TextView | `tui_text_view.h` | Multi-line read-only text |
| ScrollContainer | `tui_scroll_container.h` | Scrollable widget container |
| Clipboard | `tui_clipboard.h` | OSC 52 clipboard support |

Study the existing widget implementations in `src/` for more complex patterns (e.g., `TuiList` for scroll + selection, `TuiButton` for mouse + keyboard interaction).

## Next Steps

- [Layout System](LAYOUT.md) — Arrange widgets with the layout engine
- [Styling & Theming](STYLING.md) — Apply colors, themes, and state-based styles
- [Architecture Overview](ARCHITECTURE.md) — Rendering pipeline and event flow
