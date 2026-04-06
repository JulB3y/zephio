#include "tui_style.h"

TuiTheme tui_theme_default(void)
{
    TuiTheme theme;
    theme.styles[TUI_STATE_NORMAL]   = TUI_STYLE(TUI_COLOR_WHITE, TUI_COLOR_BLACK, TUI_ATTR_NONE);
    theme.styles[TUI_STATE_FOCUSED]  = TUI_STYLE(TUI_COLOR_BLACK, TUI_COLOR_BRIGHT_CYAN, TUI_ATTR_BOLD);
    theme.styles[TUI_STATE_DISABLED] = TUI_STYLE(TUI_COLOR_BRIGHT_BLACK, TUI_COLOR_BLACK, TUI_ATTR_DIM);
    theme.styles[TUI_STATE_ACTIVE]   = TUI_STYLE(TUI_COLOR_WHITE, TUI_COLOR_BRIGHT_BLUE, TUI_ATTR_BOLD);
    theme.styles[TUI_STATE_HOVER]    = TUI_STYLE(TUI_COLOR_BLACK, TUI_COLOR_BRIGHT_WHITE, TUI_ATTR_NONE);
    return theme;
}

TuiTheme tui_theme_create(const TuiStyle *normal, const TuiStyle *focused,
                          const TuiStyle *disabled, const TuiStyle *active)
{
    TuiTheme theme;
    theme.styles[TUI_STATE_NORMAL]   = normal   ? *normal   : TUI_STYLE_NONE;
    theme.styles[TUI_STATE_FOCUSED]  = focused  ? *focused  : TUI_STYLE_NONE;
    theme.styles[TUI_STATE_DISABLED] = disabled ? *disabled : TUI_STYLE_NONE;
    theme.styles[TUI_STATE_ACTIVE]   = active   ? *active   : TUI_STYLE_NONE;
    theme.styles[TUI_STATE_HOVER]    = TUI_STYLE(TUI_COLOR_BLACK, TUI_COLOR_BRIGHT_WHITE, TUI_ATTR_NONE);
    return theme;
}

void tui_style_set_cell(int row, int col, const char *ch, const TuiStyle *style)
{
    if (!style) return;
    tui_screen_set_cell(row, col, ch, style->fg, style->bg, style->attr);
}

void tui_style_write(int row, int col, const char *text, const TuiStyle *style)
{
    if (!style) return;
    tui_screen_write(row, col, text, style->fg, style->bg, style->attr);
}

void tui_style_fill(int row, int col, int width, int height,
                    const char *ch, const TuiStyle *style)
{
    if (!style) return;
    tui_screen_fill(row, col, width, height, ch, style->fg, style->bg, style->attr);
}
