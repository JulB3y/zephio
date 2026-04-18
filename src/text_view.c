#define _POSIX_C_SOURCE 200809L

#include "tui_text_view.h"
#include "tui_context.h"
#include "tui_text.h"
#include "tui_screen.h"

#include <stdlib.h>
#include <string.h>

#define LINES_INITIAL_CAPACITY 64
#define RENDER_BUF_SIZE 4096

static void text_view_ensure_capacity(TuiTextView *tv, int needed)
{
    if (needed <= tv->line_capacity) return;

    int new_cap = tv->line_capacity > 0 ? tv->line_capacity : LINES_INITIAL_CAPACITY;
    while (new_cap < needed) new_cap *= 2;

    int *new_starts  = realloc(tv->line_starts,  (size_t)new_cap * sizeof(int));
    int *new_lengths = realloc(tv->line_lengths, (size_t)new_cap * sizeof(int));
    if (!new_starts || !new_lengths) {
        free(new_starts);
        free(new_lengths);
        return;
    }

    tv->line_starts  = new_starts;
    tv->line_lengths = new_lengths;
    tv->line_capacity = new_cap;
}

static void text_view_build_lines(TuiTextView *tv, int wrap_width)
{
    tv->line_count     = 0;
    tv->max_line_width = 0;

    if (!tv->text || tv->text_len == 0) return;

    int max_breaks = tv->text_len + 1;
    int *breaks = malloc((size_t)max_breaks * sizeof(int));
    if (!breaks) return;

    int break_count;

    if (tv->word_wrap && wrap_width > 0) {
        break_count = tui_text_word_wrap(tv->text, (size_t)tv->text_len,
                                          wrap_width, breaks, max_breaks);
    } else {
        break_count = 0;
        for (int i = 0; i < tv->text_len; i++) {
            if (tv->text[i] == '\n') {
                if (break_count < max_breaks)
                    breaks[break_count++] = i;
            }
        }
    }

    int total_lines = break_count + 1;
    if (tv->text_len > 0 && tv->text[tv->text_len - 1] == '\n')
        total_lines++;

    text_view_ensure_capacity(tv, total_lines);

    int line_idx = 0;
    int pos = 0;

    for (int b = 0; b < break_count; b++) {
        int end = breaks[b];
        int len = end - pos;

        tv->line_starts[line_idx]  = pos;
        tv->line_lengths[line_idx] = len;

        if (len > 0) {
            int w = tui_text_width(tv->text + pos, (size_t)len);
            if (w > tv->max_line_width) tv->max_line_width = w;
        }

        line_idx++;

        if (tv->text[end] == '\n' || tv->text[end] == ' ') {
            pos = end + 1;
        } else {
            pos = end;
        }
    }

    if (pos < tv->text_len) {
        int len = tv->text_len - pos;
        tv->line_starts[line_idx]  = pos;
        tv->line_lengths[line_idx] = len;
        if (len > 0) {
            int w = tui_text_width(tv->text + pos, (size_t)len);
            if (w > tv->max_line_width) tv->max_line_width = w;
        }
        line_idx++;
    }

    if (tv->text_len > 0 && tv->text[tv->text_len - 1] == '\n') {
        tv->line_starts[line_idx]  = tv->text_len;
        tv->line_lengths[line_idx] = 0;
        line_idx++;
    }

    tv->line_count = line_idx;

    free(breaks);
}

static void text_view_update_content(TuiTextView *tv)
{
    TuiWidget *w = &tv->base.base;
    int wrap_width = w->width;

    if (tv->word_wrap && wrap_width > 0) {
        for (int iter = 0; iter < 3; iter++) {
            text_view_build_lines(tv, wrap_width);
            int needs_vscroll = tv->line_count > w->height;
            int adjusted = w->width - (needs_vscroll ? 1 : 0);
            if (adjusted <= 0) adjusted = 1;
            if (adjusted == wrap_width) break;
            wrap_width = adjusted;
        }
    } else {
        text_view_build_lines(tv, 0);
    }

    int content_w = tv->max_line_width;
    if (content_w < w->width) content_w = w->width;
    tui_scroll_container_set_content_size(&tv->base, content_w, tv->line_count);
}

static void text_view_render(TuiWidget *widget)
{
    TuiTextView *tv = (TuiTextView *)widget;
    TuiScrollContainer *sc = &tv->base;

    int has_vscroll = sc->content_height > widget->height;
    int has_hscroll = sc->content_width > widget->width;
    int cv_width  = widget->width  - (has_vscroll ? 1 : 0);
    int cv_height = widget->height - (has_hscroll ? 1 : 0);
    if (cv_width < 0)  cv_width  = 0;
    if (cv_height < 0) cv_height = 0;

    tui_scroll_container_clamp_scroll(sc, cv_width, cv_height);

    tui_screen_fill(tui_current_ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height,
                    " ", tv->fg, tv->bg, tv->attr);

    char buf[RENDER_BUF_SIZE];

    for (int r = 0; r < cv_height; r++) {
        int line_idx = sc->scroll_y + r;
        if (line_idx >= tv->line_count) break;

        int ls = tv->line_starts[line_idx];
        int ll = tv->line_lengths[line_idx];

        if (ll <= 0) continue;

        const char *line_text = tv->text + ls;

        int skip_bytes = tui_text_col_to_index(line_text, (size_t)ll,
                                                sc->scroll_x);
        int remaining = ll - skip_bytes;
        if (remaining <= 0) continue;

        size_t visible_bytes;
        tui_text_clip(line_text + skip_bytes, (size_t)remaining,
                      cv_width, &visible_bytes);

        if (visible_bytes > 0) {
            size_t copy = visible_bytes < sizeof(buf) - 1
                          ? visible_bytes : sizeof(buf) - 1;
            memcpy(buf, line_text + skip_bytes, copy);
            buf[copy] = '\0';
            tui_screen_write(tui_current_ctx, widget->abs_y + r, widget->abs_x,
                             buf, tv->fg, tv->bg, tv->attr);
        }
    }

    tui_scroll_container_render_scrollbars(widget, sc,
                                            has_vscroll, has_hscroll,
                                            cv_width, cv_height);
}

static void text_view_destroy(TuiWidget *widget)
{
    TuiTextView *tv = (TuiTextView *)widget;
    free(tv->text);
    free(tv->line_starts);
    free(tv->line_lengths);
    tv->text         = NULL;
    tv->line_starts  = NULL;
    tv->line_lengths = NULL;
    tv->text_len     = 0;
    tv->line_count   = 0;
    tv->line_capacity = 0;
}

static void text_view_on_resize(TuiWidget *widget, int width, int height)
{
    (void)width;
    (void)height;
    TuiTextView *tv = (TuiTextView *)widget;
    text_view_update_content(tv);
}

static TuiWidgetVTable text_view_vtable = {
    .render       = text_view_render,
    .handle_input = tui_scroll_container_handle_input,
    .handle_mouse = tui_scroll_container_handle_mouse,
    .destroy      = text_view_destroy,
    .on_resize    = text_view_on_resize,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_text_view_init(TuiTextView *tv, int x, int y,
                              int width, int height)
{
    if (!tv) return TUI_ERR_MEMORY;

    TuiResult res = tui_scroll_container_init(&tv->base, x, y, width, height);
    if (res != TUI_OK) return res;

    tv->base.base.vtable = &text_view_vtable;

    tv->text           = NULL;
    tv->text_len       = 0;
    tv->line_starts    = NULL;
    tv->line_lengths   = NULL;
    tv->line_count     = 0;
    tv->line_capacity  = 0;
    tv->max_line_width = 0;
    tv->fg      = TUI_COLOR_INDEX(15);
    tv->bg      = TUI_COLOR_INDEX(0);
    tv->attr    = TUI_ATTR_NONE;
    tv->word_wrap = 1;

    return TUI_OK;
}

void tui_text_view_set_text(TuiTextView *tv, const char *text)
{
    if (!tv) return;

    free(tv->text);

    if (text) {
        tv->text = strdup(text);
        tv->text_len = (int)strlen(text);
    } else {
        tv->text     = NULL;
        tv->text_len = 0;
    }

    tv->base.scroll_x = 0;
    tv->base.scroll_y = 0;

    text_view_update_content(tv);
}

void tui_text_view_set_colors(TuiTextView *tv, TuiColor fg, TuiColor bg)
{
    if (!tv) return;
    tv->fg = fg;
    tv->bg = bg;
    tv->base.base.dirty = 1;
}

void tui_text_view_set_attr(TuiTextView *tv, TuiAttr attr)
{
    if (!tv) return;
    tv->attr = attr;
    tv->base.base.dirty = 1;
}

void tui_text_view_set_word_wrap(TuiTextView *tv, int enabled)
{
    if (!tv) return;
    tv->word_wrap = enabled;
    tv->base.scroll_x = 0;
    tv->base.scroll_y = 0;
    text_view_update_content(tv);
}

int tui_text_view_get_line_count(TuiTextView *tv)
{
    if (!tv) return 0;
    return tv->line_count;
}

int tui_text_view_get_scroll_y(TuiTextView *tv)
{
    if (!tv) return 0;
    return tv->base.scroll_y;
}
