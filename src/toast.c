#define _POSIX_C_SOURCE 200809L

#include "zephio_toast.h"
#include "zephio_context.h"
#include "zephio_style.h"

#include <string.h>

static void severity_colors(ZephioToastSeverity severity, ZephioColor *fg,
                            ZephioColor *bg, ZephioColor *border_fg,
                            const char **icon)
{
    switch (severity) {
    case ZEPHIO_TOAST_SUCCESS:
        *fg        = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BLACK);
        *bg        = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_GREEN);
        *border_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_GREEN);
        *icon      = "\xe2\x9c\x93";
        break;
    case ZEPHIO_TOAST_WARNING:
        *fg        = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BLACK);
        *bg        = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_YELLOW);
        *border_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_YELLOW);
        *icon      = "\xe2\x9a\xa0";
        break;
    case ZEPHIO_TOAST_ERROR:
        *fg        = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_WHITE);
        *bg        = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_RED);
        *border_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_RED);
        *icon      = "\xe2\x9c\x97";
        break;
    default:
        *fg        = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_WHITE);
        *bg        = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BLUE);
        *border_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_CYAN);
        *icon      = "\xe2\x84\xb9";
        break;
    }
}

static int toast_message_width(const char *msg)
{
    int len = 0;
    for (const char *p = msg; *p; p++) {
        if ((*p & 0xC0) != 0x80) len++;
    }
    return len;
}

static int compute_toast_width(const char *message)
{
    int msg_w = toast_message_width(message);
    int w = msg_w + 6;
    if (w < ZEPHIO_TOAST_MIN_WIDTH) w = ZEPHIO_TOAST_MIN_WIDTH;
    if (w > ZEPHIO_TOAST_MAX_WIDTH) w = ZEPHIO_TOAST_MAX_WIDTH;
    return w;
}

static void compact_toasts(ZephioToastManager *mgr)
{
    int write = 0;
    for (int read = 0; read < ZEPHIO_TOAST_MAX_COUNT; read++) {
        if (mgr->toasts[read].state != ZEPHIO_TOAST_DISMISSED) {
            if (write != read) {
                mgr->toasts[write] = mgr->toasts[read];
            }
            write++;
        }
    }
    for (int i = write; i < ZEPHIO_TOAST_MAX_COUNT; i++) {
        memset(&mgr->toasts[i], 0, sizeof(ZephioToast));
        mgr->toasts[i].state = ZEPHIO_TOAST_DISMISSED;
    }
    mgr->count = write;
}

ZephioResult zephio_toast_manager_init(ZephioToastManager *mgr)
{
    if (!mgr) return TUI_ERR_MEMORY;
    memset(mgr, 0, sizeof(*mgr));
    mgr->count   = 0;
    mgr->next_id = 1;
    for (int i = 0; i < ZEPHIO_TOAST_MAX_COUNT; i++) {
        mgr->toasts[i].state = ZEPHIO_TOAST_DISMISSED;
    }
    return ZEPHIO_OK;
}

void zephio_toast_manager_free(ZephioToastManager *mgr)
{
    if (!mgr) return;
    mgr->count = 0;
}

int zephio_toast_show(ZephioToastManager *mgr, ZephioToastSeverity severity,
                   const char *message, double duration_ms)
{
    return zephio_toast_show_cb(mgr, severity, message, duration_ms, NULL, NULL);
}

int zephio_toast_show_cb(ZephioToastManager *mgr, ZephioToastSeverity severity,
                      const char *message, double duration_ms,
                      ZephioToastDismissFn on_dismiss, void *user_data)
{
    if (!mgr || !message) return -1;

    if (mgr->count >= ZEPHIO_TOAST_MAX_COUNT) {
        compact_toasts(mgr);
    }
    if (mgr->count >= ZEPHIO_TOAST_MAX_COUNT) {
        int oldest = 0;
        for (int i = 1; i < mgr->count; i++) {
            if (mgr->toasts[i].id < mgr->toasts[oldest].id) {
                oldest = i;
            }
        }
        ZephioToast *t = &mgr->toasts[oldest];
        if (t->on_dismiss) t->on_dismiss(t->id, t->user_data);
        t->state = ZEPHIO_TOAST_DISMISSED;
        compact_toasts(mgr);
    }

    int slot = -1;
    for (int i = 0; i < ZEPHIO_TOAST_MAX_COUNT; i++) {
        if (mgr->toasts[i].state == ZEPHIO_TOAST_DISMISSED) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return -1;

    ZephioToast *t     = &mgr->toasts[slot];
    t->id           = mgr->next_id++;
    t->severity     = severity;
    t->duration_ms  = duration_ms > 0 ? duration_ms : ZEPHIO_TOAST_DEFAULT_MS;
    t->elapsed_ms   = 0.0;
    t->anim_ms      = 0.0;
    t->state        = ZEPHIO_TOAST_FADE_IN;
    t->width        = compute_toast_width(message);
    t->on_dismiss   = on_dismiss;
    t->user_data    = user_data;

    strncpy(t->message, message, ZEPHIO_TOAST_MAX_MESSAGE - 1);
    t->message[ZEPHIO_TOAST_MAX_MESSAGE - 1] = '\0';

    mgr->count++;
    return t->id;
}

void zephio_toast_dismiss(ZephioToastManager *mgr, int id)
{
    if (!mgr) return;
    for (int i = 0; i < ZEPHIO_TOAST_MAX_COUNT; i++) {
        if (mgr->toasts[i].id == id &&
            mgr->toasts[i].state != ZEPHIO_TOAST_DISMISSED &&
            mgr->toasts[i].state != ZEPHIO_TOAST_FADE_OUT) {
            mgr->toasts[i].state   = ZEPHIO_TOAST_FADE_OUT;
            mgr->toasts[i].anim_ms = 0.0;
            return;
        }
    }
}

void zephio_toast_dismiss_all(ZephioToastManager *mgr)
{
    if (!mgr) return;
    for (int i = 0; i < ZEPHIO_TOAST_MAX_COUNT; i++) {
        if (mgr->toasts[i].state != ZEPHIO_TOAST_DISMISSED &&
            mgr->toasts[i].state != ZEPHIO_TOAST_FADE_OUT) {
            mgr->toasts[i].state   = ZEPHIO_TOAST_FADE_OUT;
            mgr->toasts[i].anim_ms = 0.0;
        }
    }
}

void zephio_toast_update(ZephioToastManager *mgr, double delta_ms)
{
    if (!mgr) return;

    int need_compact = 0;
    for (int i = 0; i < ZEPHIO_TOAST_MAX_COUNT; i++) {
        ZephioToast *t = &mgr->toasts[i];
        if (t->state == ZEPHIO_TOAST_DISMISSED) continue;

        switch (t->state) {
        case ZEPHIO_TOAST_FADE_IN:
            t->anim_ms += delta_ms;
            if (t->anim_ms >= ZEPHIO_TOAST_FADE_MS) {
                t->state  = ZEPHIO_TOAST_VISIBLE;
                t->anim_ms = 0.0;
            }
            break;

        case ZEPHIO_TOAST_VISIBLE:
            t->elapsed_ms += delta_ms;
            if (t->elapsed_ms >= t->duration_ms) {
                t->state   = ZEPHIO_TOAST_FADE_OUT;
                t->anim_ms = 0.0;
            }
            break;

        case ZEPHIO_TOAST_FADE_OUT:
            t->anim_ms += delta_ms;
            if (t->anim_ms >= ZEPHIO_TOAST_FADE_MS) {
                t->state = ZEPHIO_TOAST_DISMISSED;
                if (t->on_dismiss) {
                    t->on_dismiss(t->id, t->user_data);
                }
                need_compact = 1;
            }
            break;

        default:
            break;
        }
    }

    if (need_compact) {
        compact_toasts(mgr);
    }
}

void zephio_toast_render(ZephioContext *ctx, ZephioToastManager *mgr, int screen_rows, int screen_cols)
{
    if (!mgr || mgr->count == 0) return;

    int y = ZEPHIO_TOAST_MARGIN;
    for (int i = 0; i < ZEPHIO_TOAST_MAX_COUNT; i++) {
        ZephioToast *t = &mgr->toasts[i];
        if (t->state == ZEPHIO_TOAST_DISMISSED) continue;

        t->col = screen_cols - t->width - ZEPHIO_TOAST_MARGIN;
        if (t->col < 0) t->col = 0;
        t->row = y;

        if (t->row + ZEPHIO_TOAST_HEIGHT > screen_rows) break;

        double opacity = 1.0;
        if (t->state == ZEPHIO_TOAST_FADE_IN) {
            opacity = t->anim_ms / ZEPHIO_TOAST_FADE_MS;
            if (opacity > 1.0) opacity = 1.0;
        } else if (t->state == ZEPHIO_TOAST_FADE_OUT) {
            opacity = 1.0 - (t->anim_ms / ZEPHIO_TOAST_FADE_MS);
            if (opacity < 0.0) opacity = 0.0;
        }

        ZephioColor fg, bg, border_fg;
        const char *icon;
        severity_colors(t->severity, &fg, &bg, &border_fg, &icon);

        int dim = 0;
        ZephioAttr base_attr = ZEPHIO_ATTR_NONE;
        if (opacity < 0.33) {
            dim = 1;
            base_attr = ZEPHIO_ATTR_DIM;
        } else if (opacity < 0.66) {
            base_attr = ZEPHIO_ATTR_DIM;
        }

        ZephioColor use_fg = fg;
        ZephioColor use_bg = bg;
        ZephioColor use_border = border_fg;

        if (dim) {
            use_fg     = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_GRAY_MID);
            use_bg     = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK);
            use_border = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_GRAY_DARK);
        }

        zephio_screen_fill(ctx, t->row, t->col, t->width, ZEPHIO_TOAST_HEIGHT,
                        " ", use_fg, use_bg, base_attr);

        zephio_screen_set_cell(ctx, t->row, t->col, "\xe2\x95\x94",
                            use_border, use_bg, base_attr);
        for (int c = 1; c < t->width - 1; c++) {
            zephio_screen_set_cell(ctx, t->row, t->col + c, "\xe2\x95\x90",
                                use_border, use_bg, base_attr);
        }
        zephio_screen_set_cell(ctx, t->row, t->col + t->width - 1, "\xe2\x95\x97",
                            use_border, use_bg, base_attr);

        for (int r = 1; r < ZEPHIO_TOAST_HEIGHT - 1; r++) {
            zephio_screen_set_cell(ctx, t->row + r, t->col, "\xe2\x95\x91",
                                use_border, use_bg, base_attr);
            zephio_screen_set_cell(ctx, t->row + r, t->col + t->width - 1,
                                "\xe2\x95\x91", use_border, use_bg, base_attr);
        }

        zephio_screen_set_cell(ctx, t->row + ZEPHIO_TOAST_HEIGHT - 1, t->col,
                            "\xe2\x95\x9a", use_border, use_bg, base_attr);
        for (int c = 1; c < t->width - 1; c++) {
            zephio_screen_set_cell(ctx, t->row + ZEPHIO_TOAST_HEIGHT - 1,
                                t->col + c, "\xe2\x95\x90",
                                use_border, use_bg, base_attr);
        }
        zephio_screen_set_cell(ctx, t->row + ZEPHIO_TOAST_HEIGHT - 1,
                            t->col + t->width - 1, "\xe2\x95\x9d",
                            use_border, use_bg, base_attr);

        int text_row = t->row + 1;
        zephio_screen_write(ctx, text_row, t->col + 1, icon,
                         use_fg, use_bg, ZEPHIO_ATTR_BOLD | base_attr);

        int msg_col = t->col + 3;
        int max_msg = t->width - 4;
        if (max_msg > 0) {
            int msg_len = (int)strlen(t->message);
            if (msg_len > max_msg) msg_len = max_msg;
            char buf[ZEPHIO_TOAST_MAX_MESSAGE];
            if (msg_len >= (int)sizeof(buf)) msg_len = (int)sizeof(buf) - 1;
            memcpy(buf, t->message, (size_t)msg_len);
            buf[msg_len] = '\0';
            zephio_screen_write(ctx, text_row, msg_col, buf,
                             use_fg, use_bg, base_attr);
        }

        if (opacity < 1.0 && opacity > 0.0) {
            int progress_chars = (int)(t->width * opacity);
            for (int c = 0; c < progress_chars && c < t->width; c++) {
                zephio_screen_set_cell(ctx, t->row + ZEPHIO_TOAST_HEIGHT - 1,
                                    t->col + c, "\xe2\x96\x88",
                                    use_border, use_bg, ZEPHIO_ATTR_DIM);
            }
        }

        y += ZEPHIO_TOAST_HEIGHT + ZEPHIO_TOAST_MARGIN;
    }
}

int zephio_toast_has_active(const ZephioToastManager *mgr)
{
    if (!mgr) return 0;
    for (int i = 0; i < ZEPHIO_TOAST_MAX_COUNT; i++) {
        if (mgr->toasts[i].state != ZEPHIO_TOAST_DISMISSED) return 1;
    }
    return 0;
}
