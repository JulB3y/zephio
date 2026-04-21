#include "zephio_style.h"
#include "zephio_context.h"

ZephioStyle zephio_style_make(ZephioColor fg, ZephioColor bg, ZephioAttr attr)
{
    ZephioStyle s;
    s.fg   = fg;
    s.bg   = bg;
    s.attr = attr;
    return s;
}

ZephioTheme zephio_theme_default(void)
{
    ZephioTheme theme;
    theme.styles[ZEPHIO_STATE_NORMAL]   = ZEPHIO_STYLE(ZEPHIO_COLOR_WHITE, ZEPHIO_COLOR_BLACK, ZEPHIO_ATTR_NONE);
    theme.styles[ZEPHIO_STATE_FOCUSED]  = ZEPHIO_STYLE(ZEPHIO_COLOR_BLACK, ZEPHIO_COLOR_BRIGHT_CYAN, ZEPHIO_ATTR_BOLD);
    theme.styles[ZEPHIO_STATE_DISABLED] = ZEPHIO_STYLE(ZEPHIO_COLOR_BRIGHT_BLACK, ZEPHIO_COLOR_BLACK, ZEPHIO_ATTR_DIM);
    theme.styles[ZEPHIO_STATE_ACTIVE]   = ZEPHIO_STYLE(ZEPHIO_COLOR_WHITE, ZEPHIO_COLOR_BRIGHT_BLUE, ZEPHIO_ATTR_BOLD);
    theme.styles[ZEPHIO_STATE_HOVER]    = ZEPHIO_STYLE(ZEPHIO_COLOR_BLACK, ZEPHIO_COLOR_BRIGHT_WHITE, ZEPHIO_ATTR_NONE);
    return theme;
}

ZephioTheme zephio_theme_create(const ZephioStyle *normal, const ZephioStyle *focused,
                          const ZephioStyle *disabled, const ZephioStyle *active)
{
    ZephioTheme theme;
    theme.styles[ZEPHIO_STATE_NORMAL]   = normal   ? *normal   : ZEPHIO_STYLE_NONE;
    theme.styles[ZEPHIO_STATE_FOCUSED]  = focused  ? *focused  : ZEPHIO_STYLE_NONE;
    theme.styles[ZEPHIO_STATE_DISABLED] = disabled ? *disabled : ZEPHIO_STYLE_NONE;
    theme.styles[ZEPHIO_STATE_ACTIVE]   = active   ? *active   : ZEPHIO_STYLE_NONE;
    theme.styles[ZEPHIO_STATE_HOVER]    = ZEPHIO_STYLE(ZEPHIO_COLOR_BLACK, ZEPHIO_COLOR_BRIGHT_WHITE, ZEPHIO_ATTR_NONE);
    return theme;
}

void zephio_style_set_cell(ZephioContext *ctx, int row, int col, const char *ch, const ZephioStyle *style)
{
    if (!style) return;
    zephio_screen_set_cell(ctx, row, col, ch, style->fg, style->bg, style->attr);
}

void zephio_style_write(ZephioContext *ctx, int row, int col, const char *text, const ZephioStyle *style)
{
    if (!style) return;
    zephio_screen_write(ctx, row, col, text, style->fg, style->bg, style->attr);
}

void zephio_style_fill(ZephioContext *ctx, int row, int col, int width, int height,
                    const char *ch, const ZephioStyle *style)
{
    if (!style) return;
    zephio_screen_fill(ctx, row, col, width, height, ch, style->fg, style->bg, style->attr);
}
