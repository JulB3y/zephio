#define _POSIX_C_SOURCE 200809L

#include "zephio.h"
#include "zephio_terminal.h"
#include "zephio_ansi.h"
#include "zephio_input.h"

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

static void draw(ZephioContext *ctx, int rows, int cols)
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

static int input_callback(const ZephioEvent *event, void *user_data)
{
    ZephioContext *ctx = (ZephioContext *)user_data;
    char buf[80];

    if (event->key == ZEPHIO_EVENT_RESIZE) {
        snprintf(buf, sizeof(buf), "[Resize] %dx%d", event->size.cols, event->size.rows);
        log_event(buf);

        ZephioSize size = zephio_screen_size(ctx);
        draw(ctx, size.rows, size.cols);
        return 0;
    }

    const char *mod = zephio_modifier_name(event->modifiers);
    const char *key_name = zephio_key_name(event->key);

    if (event->codepoint > 0 && event->key == ZEPHIO_KEY_UNKNOWN) {
        if (event->codepoint >= 0x20 && event->codepoint < 0x7F) {
            snprintf(buf, sizeof(buf), "[%s'%c' U+%04X]", mod, (char)event->codepoint, event->codepoint);
        } else {
            snprintf(buf, sizeof(buf), "[%sU+%04X]", mod, event->codepoint);
        }
    } else {
        snprintf(buf, sizeof(buf), "[%s%s (0x%X)]", mod, key_name, event->key);
    }

    log_event(buf);

    if (event->key == ZEPHIO_KEY_CTRL_C || event->key == ZEPHIO_KEY_ESCAPE) {
        return 1;
    }
    if (event->codepoint == 'q' && event->modifiers == ZEPHIO_MOD_NONE) {
        return 1;
    }

    ZephioSize size = zephio_screen_size(ctx);
    draw(ctx, size.rows, size.cols);

    return 0;
}

int main(void)
{
    ZephioContext ctx;

    ZephioResult res = zephio_init(&ctx);
    if (res != ZEPHIO_OK) {
        fprintf(stderr, "zephio_init failed: %d\n", res);
        return 1;
    }

    zephio_input_init(&ctx);

    ZephioSize size = zephio_screen_size(&ctx);
    draw(&ctx, size.rows, size.cols);

    zephio_input_loop(&ctx, input_callback, &ctx);

    zephio_input_shutdown(&ctx);
    zephio_shutdown(&ctx);
    return 0;
}
