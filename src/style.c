#include "tui_style.h"
#include "tui_context.h"

TuiStyle tui_style_make(TuiColor fg, TuiColor bg, TuiAttr attr)
{
    TuiStyle s;
    s.fg   = fg;
    s.bg   = bg;
    s.attr = attr;
    return s;
}

TuiTheme tui_theme_default(void)
{
    TuiTheme theme;
    theme.styles[TUI_STATE_NORMAL]   = ZEPHIO_STYLE(TUI_COLOR_WHITE, TUI_COLOR_BLACK, ZEPHIO_ATTR_NONE);
    theme.styles[TUI_STATE_FOCUSED]  = ZEPHIO_STYLE(TUI_COLOR_BLACK, TUI_COLOR_BRIGHT_CYAN, ZEPHIO_ATTR_BOLD);
    theme.styles[TUI_STATE_DISABLED] = ZEPHIO_STYLE(TUI_COLOR_BRIGHT_BLACK, TUI_COLOR_BLACK, ZEPHIO_ATTR_DIM);
    theme.styles[TUI_STATE_ACTIVE]   = ZEPHIO_STYLE(TUI_COLOR_WHITE, TUI_COLOR_BRIGHT_BLUE, ZEPHIO_ATTR_BOLD);
    theme.styles[TUI_STATE_HOVER]    = ZEPHIO_STYLE(TUI_COLOR_BLACK, TUI_COLOR_BRIGHT_WHITE, ZEPHIO_ATTR_NONE);
    return theme;
}

TuiTheme tui_theme_create(const TuiStyle *normal, const TuiStyle *focused,
                          const TuiStyle *disabled, const TuiStyle *active)
{
    TuiTheme theme;
    theme.styles[TUI_STATE_NORMAL]   = normal   ? *normal   : ZEPHIO_STYLE_NONE;
    theme.styles[TUI_STATE_FOCUSED]  = focused  ? *focused  : ZEPHIO_STYLE_NONE;
    theme.styles[TUI_STATE_DISABLED] = disabled ? *disabled : ZEPHIO_STYLE_NONE;
    theme.styles[TUI_STATE_ACTIVE]   = active   ? *active   : ZEPHIO_STYLE_NONE;
    theme.styles[TUI_STATE_HOVER]    = ZEPHIO_STYLE(TUI_COLOR_BLACK, TUI_COLOR_BRIGHT_WHITE, ZEPHIO_ATTR_NONE);
    return theme;
}

void tui_style_set_cell(TuiContext *ctx, int row, int col, const char *ch, const TuiStyle *style)
{
    if (!style) return;
    tui_screen_set_cell(ctx, row, col, ch, style->fg, style->bg, style->attr);
}

void tui_style_write(TuiContext *ctx, int row, int col, const char *text, const TuiStyle *style)
{
    if (!style) return;
    tui_screen_write(ctx, row, col, text, style->fg, style->bg, style->attr);
}

void tui_style_fill(TuiContext *ctx, int row, int col, int width, int height,
                    const char *ch, const TuiStyle *style)
{
    if (!style) return;
    tui_screen_fill(ctx, row, col, width, height, ch, style->fg, style->bg, style->attr);
}
