# Layout System Guide

Zephio's layout engine arranges widgets in horizontal or vertical stacks with flexible sizing constraints. This guide covers the constraint system, weight distribution, nesting, and a full dashboard example.

## Overview

`TuiLayout` is a widget (it embeds `TuiWidget`) that arranges its children according to `TuiLayoutConstraints`. Children are added with `tui_layout_add()`.

```
┌─────────────────────────────────────┐
│ TuiLayout (VERTICAL, padding=1)      │
│ ┌─────────────────────────────────┐ │
│ │ Child 1: FIXED(1) — Titlebar    │ │
│ ├─────────────────────────────────┤ │
│ │ Child 2: FILL — Content area    │ │
│ ├─────────────────────────────────┤ │
│ │ Child 3: FIXED(1) — Statusbar   │ │
│ └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

## Layout Direction

```c
typedef enum {
    TUI_LAYOUT_HORIZONTAL,  // Children arranged left to right
    TUI_LAYOUT_VERTICAL     // Children arranged top to bottom
} TuiLayoutDirection;
```

Set at init time and changeable via `tui_layout_set_direction()`.

## Sizing Constraints

Each child is paired with a `TuiLayoutConstraints` value:

| Constraint | Macro | Behavior |
|---|---|---|
| **Fixed** | `TUI_LAYOUT_FIXED(n)` | Exact size in cells |
| **Fill** | `TUI_LAYOUT_FILL` | Share remaining space equally (weight 1.0) |
| **Weighted Fill** | `TUI_LAYOUT_FILL_WEIGHT(w)` | Share remaining space with weight `w` |
| **Auto** | `TUI_LAYOUT_AUTO` | Use the child's current `width`/`height` |

### How Fill Works

After allocating space for all `FIXED` and `AUTO` children, the remaining space is divided among `FILL` children proportionally to their weights.

Example (80 columns, horizontal layout):
- Child A: `TUI_LAYOUT_FIXED(20)` → 20 columns
- Child B: `TUI_LAYOUT_FILL` → 30 columns (weight 1.0)
- Child C: `TUI_LAYOUT_FILL` → 30 columns (weight 1.0)

Example with weights (80 columns, horizontal layout):
- Child A: `TUI_LAYOUT_FIXED(20)` → 20 columns
- Child B: `TUI_LAYOUT_FILL_WEIGHT(2)` → 40 columns (weight 2.0)
- Child C: `TUI_LAYOUT_FILL` → 20 columns (weight 1.0)

## Basic API

### Initialization

```c
TuiLayout layout;
tui_layout_init(&layout, TUI_LAYOUT_VERTICAL, 0, 0, width, height);
```

### Adding Children

```c
TuiWidget header, content, footer;

tui_widget_init(&header, 0, 0, 1, 1, NULL, NULL);
tui_widget_init(&content, 0, 0, 1, 1, NULL, NULL);
tui_widget_init(&footer, 0, 0, 1, 1, NULL, NULL);

tui_layout_add(&layout, &header, TUI_LAYOUT_FIXED(1));
tui_layout_add(&layout, &content, TUI_LAYOUT_FILL);
tui_layout_add(&layout, &footer, TUI_LAYOUT_FIXED(1));
```

### Recalculating

Call `tui_layout_recalculate()` after adding/removing children or changing the layout size:

```c
tui_layout_recalculate(&layout);
```

Note: `tui_widget_resize()` on the layout triggers recalculation automatically.

### Padding and Margins

```c
tui_layout_set_padding(&layout, 1);                     // 1 cell between children
tui_layout_set_margin(&layout, 1, 0, 1, 0);            // top, right, bottom, left
```

Padding is internal spacing between children. Margins are external spacing around the layout boundary.

### Cleanup

```c
tui_layout_remove_all(&layout);   // Remove all children (does not destroy)
```

Use `tui_layout_destroy()` as the vtable's `destroy` function if the layout was heap-allocated.

## Nesting Layouts

Layouts can be nested: a `TuiLayout` is itself a `TuiWidget`, so you can add it as a child of another layout.

### Example: Header + Body (Sidebar + Content) + Footer

```c
TuiLayout root;       // Vertical: header, body, footer
TuiLayout body;       // Horizontal: sidebar, content
TuiWidget header, sidebar, content, footer;

// Root vertical layout
tui_layout_init(&root, TUI_LAYOUT_VERTICAL, 0, 0, cols, rows);

// Header (fixed height)
tui_widget_init(&header, 0, 0, 1, 1, NULL, NULL);
tui_layout_add(&root, &header, TUI_LAYOUT_FIXED(1));

// Body (horizontal, fills remaining space)
tui_layout_init(&body, TUI_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
tui_layout_set_padding(&body, 1);
tui_layout_add(&root, &body.base, TUI_LAYOUT_FILL);

// Sidebar (fixed width)
tui_widget_init(&sidebar, 0, 0, 20, 1, NULL, NULL);
tui_layout_add(&body, &sidebar, TUI_LAYOUT_FIXED(20));

// Content (fills remaining width)
tui_widget_init(&content, 0, 0, 1, 1, NULL, NULL);
tui_layout_add(&body, &content, TUI_LAYOUT_FILL);

// Footer (fixed height)
tui_widget_init(&footer, 0, 0, 1, 1, NULL, NULL);
tui_layout_add(&root, &footer, TUI_LAYOUT_FIXED(1));

// Apply layout
tui_widget_resize(&root.base, cols, rows);
```

This produces:

```
┌──────────────────────────────────────────┐
│ Header (1 row, FIXED)                     │
├──────────┬───────────────────────────────┤
│ Sidebar  │ Content                        │
│ (20 cols │ (FILL — takes remaining space) │
│  FIXED)  │                                │
├──────────┴───────────────────────────────┤
│ Footer (1 row, FIXED)                     │
└──────────────────────────────────────────┘
```

## Full Example: Dashboard Layout

This is the layout from `examples/layout_dashboard.c` — a nested V > H > V dashboard:

```
┌──────────────────────────────────────────────┐
│ Titlebar (FIXED 1)                            │
├───────────┬──────────────────────────────────┤
│           │ Toolbar (FIXED 1)                 │
│           ├─────────────┬────────────────────┤
│ Nav       │ Editor      │ Preview            │
│ FIXED(18) │ FILL w=2    │ FILL w=1           │
├───────────┴─────────────┴────────────────────┤
│ Statusbar (FIXED 1)                           │
└──────────────────────────────────────────────┘
```

```c
TuiLayout root;
TuiWidget  titlebar;
TuiLayout  main_row;
TuiWidget  nav;
TuiLayout  center_col;
TuiWidget  toolbar;
TuiLayout  content_row;
TuiWidget  editor;
TuiWidget  preview;
TuiWidget  statusbar;

tui_layout_init(&root, TUI_LAYOUT_VERTICAL, 0, 0, cols, rows);
tui_layout_set_padding(&root, 0);

// Titlebar
tui_widget_init(&titlebar, 0, 0, 1, 1, NULL, NULL);
tui_layout_add(&root, &titlebar, TUI_LAYOUT_FIXED(1));

// Main area (horizontal: nav + center)
tui_layout_init(&main_row, TUI_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
tui_layout_set_padding(&main_row, 1);
tui_layout_add(&root, &main_row.base, TUI_LAYOUT_FILL);

// Navigation sidebar
tui_widget_init(&nav, 0, 0, 18, 1, NULL, NULL);
tui_layout_add(&main_row, &nav, TUI_LAYOUT_FIXED(18));

// Center column (vertical: toolbar + content)
tui_layout_init(&center_col, TUI_LAYOUT_VERTICAL, 0, 0, 1, 1);
tui_layout_set_padding(&center_col, 1);
tui_layout_add(&main_row, &center_col.base, TUI_LAYOUT_FILL);

// Toolbar
tui_widget_init(&toolbar, 0, 0, 1, 1, NULL, NULL);
tui_layout_add(&center_col, &toolbar, TUI_LAYOUT_FIXED(1));

// Content row (horizontal: editor + preview)
tui_layout_init(&content_row, TUI_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
tui_layout_set_padding(&content_row, 1);
tui_layout_add(&center_col, &content_row.base, TUI_LAYOUT_FILL);

// Editor gets 2/3, preview gets 1/3
tui_widget_init(&editor, 0, 0, 1, 1, NULL, NULL);
tui_layout_add(&content_row, &editor, TUI_LAYOUT_FILL_WEIGHT(2));

tui_widget_init(&preview, 0, 0, 1, 1, NULL, NULL);
tui_layout_add(&content_row, &preview, TUI_LAYOUT_FILL);

// Statusbar
tui_widget_init(&statusbar, 0, 0, 1, 1, NULL, NULL);
tui_layout_add(&root, &statusbar, TUI_LAYOUT_FIXED(1));

// Finalize
tui_widget_resize(&root.base, cols, rows);
```

## Handling Resize

When the terminal is resized, update the root layout and redraw:

```c
static int on_input(const TuiEvent *ev, void *ud) {
    if (ev->key == TUI_EVENT_RESIZE) {
        tui_screen_resize(ev->size.rows, ev->size.cols);
        tui_widget_resize(&root.base, ev->size.cols, ev->size.rows);
        draw_frame(ev->size.rows, ev->size.cols);
        return 0;
    }
    // ...
}
```

`tui_widget_resize()` on a `TuiLayout` triggers `tui_layout_recalculate()` internally, which redistributes space among children according to their constraints.

## Weight Distribution Examples

### Equal Split (1:1:1)

```c
tui_layout_add(&row, &a, TUI_LAYOUT_FILL);
tui_layout_add(&row, &b, TUI_LAYOUT_FILL);
tui_layout_add(&row, &c, TUI_LAYOUT_FILL);
```

### 2:1 Ratio

```c
tui_layout_add(&row, &a, TUI_LAYOUT_FILL_WEIGHT(2));
tui_layout_add(&row, &b, TUI_LAYOUT_FILL);
```

### Mixed Fixed + Fill

```c
tui_layout_add(&row, &sidebar, TUI_LAYOUT_FIXED(20));
tui_layout_add(&row, &content, TUI_LAYOUT_FILL);
```

### Auto-Sized + Fill

```c
tui_layout_add(&row, &label, TUI_LAYOUT_AUTO);  // Uses label's natural width
tui_layout_add(&row, &input, TUI_LAYOUT_FILL);  // Takes the rest
```

## Cleaning Up

Remove children from innermost layouts first, working outward:

```c
tui_layout_remove_all(&content_row);
tui_layout_remove_all(&center_col);
tui_layout_remove_all(&main_row);
tui_layout_remove_all(&root);
```

## Common Patterns

### Two-Column Form

```c
TuiLayout form;
tui_layout_init(&form, TUI_LAYOUT_VERTICAL, x, y, width, height);
tui_layout_set_padding(&form, 1);

for (int i = 0; i < field_count; i++) {
    TuiLayout *row = &rows[i];
    tui_layout_init(row, TUI_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
    tui_layout_add(row, &labels[i].base, TUI_LAYOUT_FIXED(15));
    tui_layout_add(row, &inputs[i].base, TUI_LAYOUT_FILL);
    tui_layout_add(&form, &row->base, TUI_LAYOUT_FIXED(1));
}
```

### Full-Screen App Skeleton

```c
tui_layout_init(&root, TUI_LAYOUT_VERTICAL, 0, 0, cols, rows);
tui_layout_add(&root, &titlebar,  TUI_LAYOUT_FIXED(1));
tui_layout_add(&root, &body,      TUI_LAYOUT_FILL);
tui_layout_add(&root, &statusbar, TUI_LAYOUT_FIXED(1));
```

## Next Steps

- [Getting Started](GETTING_STARTED.md) — Build and run your first app
- [Widget Development](WIDGET_DEVELOPMENT.md) — Create custom widgets
- [Architecture Overview](ARCHITECTURE.md) — Rendering pipeline internals
