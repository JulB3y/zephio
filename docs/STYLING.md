# Styling & Theming Guide

Zephio provides a style system with 256-color and truecolor RGB support, per-state widget themes, and convenience drawing functions. This guide covers colors, text attributes, style objects, themes, and terminal compatibility.

## Colors

### 256-Color Palette

Use `ZEPHIO_COLOR_INDEX(n)` for the standard 256-color terminal palette (values 0-255):

```c
TuiColor fg = ZEPHIO_COLOR_INDEX(15);  // Bright white
TuiColor bg = ZEPHIO_COLOR_INDEX(4);   // Blue
```

Named palette colors are available as enum values:

```c
ZEPHIO_COLOR_BLACK         // 0
ZEPHIO_COLOR_RED           // 1
ZEPHIO_COLOR_GREEN         // 2
ZEPHIO_COLOR_YELLOW        // 3
ZEPHIO_COLOR_BLUE          // 4
ZEPHIO_COLOR_MAGENTA       // 5
ZEPHIO_COLOR_CYAN          // 6
ZEPHIO_COLOR_WHITE         // 7
ZEPHIO_COLOR_BRIGHT_BLACK  // 8
ZEPHIO_COLOR_BRIGHT_RED    // 9
// ... through ZEPHIO_COLOR_BRIGHT_WHITE (15)
ZEPHIO_COLOR_GRAY_DARK     // 232
ZEPHIO_COLOR_GRAY_MID      // 240
ZEPHIO_COLOR_GRAY_LIGHT    // 248
ZEPHIO_COLOR_BG_DARK       // 234
ZEPHIO_COLOR_BG_MID        // 236
ZEPHIO_COLOR_BG_LIGHT      // 238
```

### Truecolor (24-bit RGB)

Use `ZEPHIO_COLOR_RGB(r, g, b)` for precise color control:

```c
TuiColor fg = ZEPHIO_COLOR_RGB(255, 100, 50);   // Orange
TuiColor bg = ZEPHIO_COLOR_RGB(30, 30, 46);     // Catppuccin background
```

Truecolor is automatically detected via the `COLORTERM` environment variable. If the terminal does not support truecolor, the framework falls back to the closest 256-color match.

### No Color

Use `ZEPHIO_COLOR_NONE` for transparent/no-color:

```c
TuiColor none = ZEPHIO_COLOR_NONE;
```

### Color Comparison

```c
if (tui_color_eq(color_a, color_b)) {
    // Colors are identical
}
```

## Text Attributes

Attributes are a bitmask — combine with `|`:

```c
TuiAttr attr = ZEPHIO_ATTR_BOLD | ZEPHIO_ATTR_UNDERLINE;
```

| Constant | Value | Effect |
|---|---|---|
| `ZEPHIO_ATTR_NONE` | 0x0000 | No attributes |
| `ZEPHIO_ATTR_BOLD` | 0x0001 | Bold/bright |
| `ZEPHIO_ATTR_DIM` | 0x0002 | Dim/faint |
| `ZEPHIO_ATTR_ITALIC` | 0x0004 | Italic |
| `ZEPHIO_ATTR_UNDERLINE` | 0x0008 | Underline |
| `ZEPHIO_ATTR_BLINK` | 0x0010 | Blinking |
| `ZEPHIO_ATTR_REVERSE` | 0x0020 | Reverse video (swap fg/bg) |
| `ZEPHIO_ATTR_STRIKETHROUGH` | 0x0040 | Strikethrough |

## TuiStyle

`TuiStyle` bundles foreground, background, and attributes into a single object:

```c
typedef struct {
    TuiColor fg;
    TuiColor bg;
    TuiAttr  attr;
} TuiStyle;
```

### Creating Styles

```c
// Using the macro
TuiStyle style = TUI_STYLE(15, 4, ZEPHIO_ATTR_BOLD);
// fg=white, bg=blue, bold

// RGB style
TuiStyle rgb_style = TUI_STYLE_RGB(255, 100, 50, 30, 30, 46, ZEPHIO_ATTR_NONE);

// Using the function
TuiStyle s = tui_style_make(ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(0), ZEPHIO_ATTR_BOLD);

// No style
TuiStyle none = TUI_STYLE_NONE;
```

### Style Convenience Functions

These functions apply a `TuiStyle` and draw to the back buffer in one call:

```c
// Write text with a style
tui_style_write(row, col, "Hello", &style);

// Fill a region with a character and style
tui_style_fill(row, col, width, height, " ", &style);

// Set a single cell with a style
tui_style_set_cell(row, col, "X", &style);
```

## Themes

A `TuiTheme` holds styles for each widget state:

```c
typedef struct TuiTheme {
    TuiStyle styles[ZEPHIO_STATE_COUNT];
} TuiTheme;
```

### Widget States

| State | Index | When Active |
|---|---|---|
| `ZEPHIO_STATE_NORMAL` | 0 | Default state |
| `ZEPHIO_STATE_FOCUSED` | 1 | Widget has keyboard focus |
| `ZEPHIO_STATE_DISABLED` | 2 | Widget is disabled (`widget->disabled = 1`) |
| `ZEPHIO_STATE_ACTIVE` | 3 | Widget is being pressed/activated |
| `ZEPHIO_STATE_HOVER` | 4 | Mouse cursor is over the widget |

### Default Theme

```c
TuiTheme theme = tui_theme_default();
```

Returns a dark color scheme with cyan/blue accents.

### Creating Custom Themes

```c
TuiStyle normal   = TUI_STYLE(15, 234, ZEPHIO_ATTR_NONE);
TuiStyle focused  = TUI_STYLE(0,  12,  ZEPHIO_ATTR_BOLD);
TuiStyle disabled = TUI_STYLE(8,  232, ZEPHIO_ATTR_DIM);
TuiStyle active   = TUI_STYLE(0,  10,  ZEPHIO_ATTR_BOLD);

TuiTheme theme = tui_theme_create(&normal, &focused, &disabled, &active);
```

The hover state is derived from the focused style automatically.

### Applying Themes

```c
// Set theme on a widget (propagates to all children)
tui_widget_set_theme(&root, &theme);

// Get the effective style for the current widget state
TuiStyle effective = tui_widget_get_style(widget);
```

`tui_widget_get_style()` resolves the correct style based on the widget's current state:
1. If `widget->disabled` → `ZEPHIO_STATE_DISABLED`
2. If `widget->focused` → `ZEPHIO_STATE_FOCUSED`
3. If `widget->hovered` → `ZEPHIO_STATE_HOVER`
4. Otherwise → `ZEPHIO_STATE_NORMAL`

### Per-Widget Theme Override

Each widget can have its own theme:

```c
tui_widget_set_theme(&my_button, &red_theme);
```

If a widget has no theme set, it inherits from its parent. The root widget uses the default theme.

## Using Styles in Widgets

### Low-Level Screen Functions

```c
// Direct color + attribute
tui_screen_write(row, col, "text", fg, bg, attr);
tui_screen_fill(row, col, w, h, " ", fg, bg, attr);
tui_screen_set_cell(row, col, "X", fg, bg, attr);

// Via TuiStyle
tui_style_write(row, col, "text", &style);
tui_style_fill(row, col, w, h, " ", &style);
```

### Widget-Specific Color APIs

Most built-in widgets have their own color setters:

```c
// Label
tui_label_set_colors(&label, fg, bg);
tui_label_set_attr(&label, ZEPHIO_ATTR_BOLD);

// Button (normal + focused states)
tui_button_set_colors(&btn, fg, bg, fg_focused, bg_focused);

// InputField
tui_input_field_set_colors(&input, fg, bg, fg_cursor, bg_cursor);

// List
tui_list_set_colors(&list, fg, bg, fg_selected, bg_selected);

// Box
tui_box_set_colors(&box, fg, bg);
tui_box_set_attr(&box, ZEPHIO_ATTR_BOLD);
```

## Truecolor vs. 256-Color Fallback

Zephio supports both color modes simultaneously. The framework auto-detects truecolor support at startup.

### Detection

The `g_terminal.truecolor` flag is set during `tui_init()`:

```c
extern Terminal g_terminal;
if (g_terminal.truecolor) {
    // Terminal supports 24-bit color
}
```

Detection checks the `COLORTERM` environment variable for `truecolor` or `24bit`.

### Behavior

- `ZEPHIO_COLOR_INDEX(n)` always uses 256-color sequences (`ESC[38;5;Nm`).
- `ZEPHIO_COLOR_RGB(r,g,b)` uses truecolor sequences (`ESC[38;2;R;G;Bm`) when available, or falls back to the closest 256-color index.
- You can mix indexed and RGB colors freely — each cell stores its own color type.

### Best Practices

1. Use `ZEPHIO_COLOR_INDEX()` for broad compatibility.
2. Use `ZEPHIO_COLOR_RGB()` for precise branding or gradient effects.
3. Test with `TERM=xterm` (no truecolor) to verify fallback rendering.
4. The 6x6x6 color cube (indices 16-231) provides a reasonable approximation for most RGB values.

## Practical Examples

### Status Bar with Segments

```c
TuiColor bar_fg = ZEPHIO_COLOR_INDEX(15);
TuiColor bar_bg = ZEPHIO_COLOR_INDEX(4);

tui_screen_fill(row, 0, cols, 1, " ", bar_fg, bar_bg, ZEPHIO_ATTR_BOLD);
tui_screen_write(row, 1, "NORMAL", bar_fg, bar_bg, ZEPHIO_ATTR_BOLD);

tui_screen_write(row, 20, "line 42, col 17",
    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_NONE);

char pos[32];
snprintf(pos, sizeof(pos), "%d:%d", cols, rows);
tui_screen_write(row, cols - strlen(pos) - 1, pos,
    bar_fg, bar_bg, ZEPHIO_ATTR_BOLD);
```

### Colored Box Panel

```c
TuiStyle panel = TUI_STYLE(0, 236, ZEPHIO_ATTR_NONE);
tui_style_fill(row, col, width, height, " ", &panel);
tui_screen_box_single(row, col, width, height,
    ZEPHIO_COLOR_INDEX(12), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);
tui_style_write(row + 1, col + 1, "Panel Title",
    &(TuiStyle){ .fg = ZEPHIO_COLOR_INDEX(14), .bg = ZEPHIO_COLOR_INDEX(236),
                 .attr = ZEPHIO_ATTR_BOLD });
```

### RGB Gradient Bar

```c
for (int i = 0; i < width; i++) {
    int r = (int)(255.0 * i / width);
    int g = (int)(100.0 * (1.0 - (double)i / width));
    int b = 150;
    tui_screen_set_cell(row, col + i, " ",
        ZEPHIO_COLOR_RGB(r, g, b), ZEPHIO_COLOR_RGB(r, g, b), ZEPHIO_ATTR_NONE);
}
```

## Next Steps

- [Layout System](LAYOUT.md) — Arrange styled widgets
- [Widget Development](WIDGET_DEVELOPMENT.md) — Apply themes in custom widgets
- [Architecture Overview](ARCHITECTURE.md) — Style system internals
