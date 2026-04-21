#define _POSIX_C_SOURCE 200809L

#include "zephio_terminal.h"
#include "zephio_ansi.h"
#include "zephio_screen.h"
#include "zephio_mouse.h"
#include "zephio_context.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

static ZephioContext *g_ctx = NULL;
static struct termios g_orig_termios;
static int g_termios_saved = 0;

static void terminal_atexit(void)
{
    if (g_ctx) {
        zephio_shutdown(g_ctx);
    }
}

static void terminal_sig_handler(int sig)
{
    (void)sig;
    _exit(0);
}

static ZephioResult terminal_raw_mode_enable(Terminal *t)
{
    struct termios raw;

    if (tcgetattr(t->fd, &g_orig_termios) == -1) {
        return TUI_ERR_TCGETATTR;
    }
    g_termios_saved = 1;

    raw = g_orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(t->fd, TCSAFLUSH, &raw) == -1) {
        return TUI_ERR_TCSETATTR;
    }

    return ZEPHIO_OK;
}

static ZephioResult terminal_raw_mode_disable(Terminal *t)
{
    if (!g_termios_saved) {
        return ZEPHIO_OK;
    }
    if (tcsetattr(t->fd, TCSAFLUSH, &g_orig_termios) == -1) {
        return TUI_ERR_TCSETATTR;
    }
    g_termios_saved = 0;
    return ZEPHIO_OK;
}

static ZephioResult terminal_enter_alt_screen(Terminal *t)
{
    terminal_write_seq(t, ANSI_ALT_SCREEN_ON, sizeof(ANSI_ALT_SCREEN_ON) - 1);
    return ZEPHIO_OK;
}

static ZephioResult terminal_exit_alt_screen(Terminal *t)
{
    terminal_write_seq(t, ANSI_ALT_SCREEN_OFF, sizeof(ANSI_ALT_SCREEN_OFF) - 1);
    return ZEPHIO_OK;
}

static ZephioResult terminal_cursor_hide(Terminal *t)
{
    terminal_write_seq(t, ANSI_CURSOR_HIDE, sizeof(ANSI_CURSOR_HIDE) - 1);
    return ZEPHIO_OK;
}

static ZephioResult terminal_cursor_show(Terminal *t)
{
    terminal_write_seq(t, ANSI_CURSOR_SHOW, sizeof(ANSI_CURSOR_SHOW) - 1);
    return ZEPHIO_OK;
}

static ZephioResult terminal_clear_screen(Terminal *t)
{
    terminal_write_seq(t, ANSI_CLEAR_SCREEN, sizeof(ANSI_CLEAR_SCREEN) - 1);
    terminal_write_seq(t, ANSI_CURSOR_HOME, sizeof(ANSI_CURSOR_HOME) - 1);
    return ZEPHIO_OK;
}

static ZephioResult terminal_get_size(Terminal *t)
{
    struct winsize ws;

    if (ioctl(t->fd, TIOCGWINSZ, &ws) == -1) {
        return TUI_ERR_IOCTL;
    }
    t->rows = ws.ws_row;
    t->cols = ws.ws_col;
    return ZEPHIO_OK;
}

void terminal_write_seq(Terminal *t, const char *seq, size_t len)
{
    if (t->fd >= 0 && seq != NULL && len > 0) {
        (void)write(t->fd, seq, len);
    }
}

static void terminal_ensure_cleanup(ZephioContext *ctx)
{
    Terminal *terminal = &ctx->terminal;
    if (terminal->initialized) {
        zephio_mouse_disable(ctx);
        zephio_screen_free(ctx);
        terminal_cursor_show(terminal);
        terminal_clear_screen(terminal);
        terminal_exit_alt_screen(terminal);
        terminal_raw_mode_disable(terminal);
        terminal->initialized = 0;
    }
}

ZephioResult zephio_init(ZephioContext *ctx)
{
    ZephioResult res;
    Terminal *terminal = &ctx->terminal;

    g_ctx = ctx;

    terminal->fd = STDIN_FILENO;

    const char *colorterm = getenv("COLORTERM");
    terminal->truecolor = (colorterm != NULL &&
        (strcmp(colorterm, "truecolor") == 0 ||
         strcmp(colorterm, "24bit") == 0));

    res = terminal_raw_mode_enable(terminal);
    if (res != ZEPHIO_OK) {
        return res;
    }

    res = terminal_get_size(terminal);
    if (res != ZEPHIO_OK) {
        terminal_raw_mode_disable(terminal);
        return res;
    }

    terminal_enter_alt_screen(terminal);
    terminal_clear_screen(terminal);
    terminal_cursor_hide(terminal);

    zephio_screen_init(ctx, terminal->rows, terminal->cols);

    terminal->initialized = 1;

    atexit(terminal_atexit);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = terminal_sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

    struct sigaction sa_winch;
    memset(&sa_winch, 0, sizeof(sa_winch));
    sa_winch.sa_handler = SIG_IGN;
    sigemptyset(&sa_winch.sa_mask);
    sigaction(SIGWINCH, &sa_winch, NULL);

    return ZEPHIO_OK;
}

void zephio_shutdown(ZephioContext *ctx)
{
    terminal_ensure_cleanup(ctx);
    g_ctx = NULL;
}

ZephioResult zephio_get_size(ZephioContext *ctx, ZephioSize *size)
{
    ZephioResult res;
    Terminal *terminal = &ctx->terminal;

    res = terminal_get_size(terminal);
    if (res != ZEPHIO_OK) {
        return res;
    }
    size->rows = terminal->rows;
    size->cols = terminal->cols;
    return ZEPHIO_OK;
}
