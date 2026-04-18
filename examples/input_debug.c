#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_terminal.h"
#include "tui_ansi.h"
#include "tui_input.h"

#include <stdio.h>
#include <string.h>

#define MAX_LOG 18

static char g_log[MAX_LOG][80];
static int  g_log_count = 0;

static void log_event(const char *msg)
{
    if (g_log_count < MAX_LOG) {
        snprintf(g_log[g_log_count], sizeof(g_log[g_log_count]), "%s", msg);
        g_log_count++;
    } else {
        for (int i = 0; i < MAX_LOG - 1; i++) {
            memcpy(g_log[i], g_log[i + 1], sizeof(g_log[i]));
        }
        snprintf(g_log[MAX_LOG - 1], sizeof(g_log[MAX_LOG - 1]), "%s", msg);
    }
}

static void draw(TuiContext *ctx, int rows, int cols)
{
    ansi_clear_screen(ctx);

    ansi_set_bold(ctx);
    ansi_set_fg(ctx, 12);
    ansi_write_at(ctx, 1, 2, "Phase 2 - Input System Debug", 28);
    ansi_reset(ctx);

    ansi_set_fg(ctx, 8);
    char info[64];
    int info_len = snprintf(info, sizeof(info), "Terminal: %dx%d", cols, rows);
    ansi_write_at(ctx, 1, cols - info_len, info, (size_t)info_len);
    ansi_reset(ctx);

    ansi_set_fg(ctx, 6);
    ansi_write_at(ctx, 3, 2, "Press keys to see events. Ctrl+C or 'q' to quit.", 48);
    ansi_reset(ctx);

    ansi_set_fg(ctx, 8);
    ansi_write_at(ctx, 4, 2, "------------------------------------------------", 48);
    ansi_reset(ctx);

    for (int i = 0; i < g_log_count; i++) {
        ansi_move_cursor(ctx, 6 + i, 2);
        ansi_write(ctx, g_log[i], strnlen(g_log[i], 79));
    }

    ansi_move_cursor(ctx, rows - 1, 2);
    ansi_set_fg(ctx, 8);
    ansi_write(ctx, "Ready...", 8);
    ansi_reset(ctx);
}

static int input_callback(const TuiEvent *event, void *user_data)
{
    TuiContext *ctx = (TuiContext *)user_data;
    char buf[80];

    if (event->key == TUI_EVENT_RESIZE) {
        snprintf(buf, sizeof(buf), "[Resize] %dx%d", event->size.cols, event->size.rows);
        log_event(buf);

        TuiSize size = tui_screen_size(ctx);
        draw(ctx, size.rows, size.cols);
        return 0;
    }

    const char *mod = tui_modifier_name(event->modifiers);
    const char *key_name = tui_key_name(event->key);

    if (event->codepoint > 0 && event->key == TUI_KEY_UNKNOWN) {
        if (event->codepoint >= 0x20 && event->codepoint < 0x7F) {
            snprintf(buf, sizeof(buf), "[%s'%c' U+%04X]", mod, (char)event->codepoint, event->codepoint);
        } else {
            snprintf(buf, sizeof(buf), "[%sU+%04X]", mod, event->codepoint);
        }
    } else {
        snprintf(buf, sizeof(buf), "[%s%s (0x%X)]", mod, key_name, event->key);
    }

    log_event(buf);

    if (event->key == TUI_KEY_CTRL_C || event->key == TUI_KEY_ESCAPE) {
        return 1;
    }
    if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
        return 1;
    }

    TuiSize size = tui_screen_size(ctx);
    draw(ctx, size.rows, size.cols);

    return 0;
}

int main(void)
{
    TuiContext ctx;

    TuiResult res = tui_init(&ctx);
    if (res != TUI_OK) {
        fprintf(stderr, "tui_init failed: %d\n", res);
        return 1;
    }

    tui_input_init(&ctx);

    TuiSize size = tui_screen_size(&ctx);
    draw(&ctx, size.rows, size.cols);

    tui_input_loop(&ctx, input_callback, &ctx);

    tui_input_shutdown(&ctx);
    tui_shutdown(&ctx);
    return 0;
}
