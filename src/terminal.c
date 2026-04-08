#define _POSIX_C_SOURCE 200809L

#include "tui_terminal.h"
#include "tui_ansi.h"
#include "tui_screen.h"
#include "tui_mouse.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

Terminal g_terminal = {
    .fd = -1,
    .rows = 0,
    .cols = 0,
    .initialized = 0,
    .truecolor = 0
};

static struct termios g_orig_termios;
static int g_termios_saved = 0;

static void terminal_atexit(void)
{
    tui_shutdown();
}

static void terminal_sig_handler(int sig)
{
    (void)sig;
    _exit(0);
}

TuiResult terminal_raw_mode_enable(Terminal *t)
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

    return TUI_OK;
}

TuiResult terminal_raw_mode_disable(Terminal *t)
{
    if (!g_termios_saved) {
        return TUI_OK;
    }
    if (tcsetattr(t->fd, TCSAFLUSH, &g_orig_termios) == -1) {
        return TUI_ERR_TCSETATTR;
    }
    g_termios_saved = 0;
    return TUI_OK;
}

TuiResult terminal_enter_alt_screen(Terminal *t)
{
    terminal_write_seq(t, ANSI_ALT_SCREEN_ON, sizeof(ANSI_ALT_SCREEN_ON) - 1);
    return TUI_OK;
}

TuiResult terminal_exit_alt_screen(Terminal *t)
{
    terminal_write_seq(t, ANSI_ALT_SCREEN_OFF, sizeof(ANSI_ALT_SCREEN_OFF) - 1);
    return TUI_OK;
}

TuiResult terminal_cursor_hide(Terminal *t)
{
    terminal_write_seq(t, ANSI_CURSOR_HIDE, sizeof(ANSI_CURSOR_HIDE) - 1);
    return TUI_OK;
}

TuiResult terminal_cursor_show(Terminal *t)
{
    terminal_write_seq(t, ANSI_CURSOR_SHOW, sizeof(ANSI_CURSOR_SHOW) - 1);
    return TUI_OK;
}

TuiResult terminal_clear_screen(Terminal *t)
{
    terminal_write_seq(t, ANSI_CLEAR_SCREEN, sizeof(ANSI_CLEAR_SCREEN) - 1);
    terminal_write_seq(t, ANSI_CURSOR_HOME, sizeof(ANSI_CURSOR_HOME) - 1);
    return TUI_OK;
}

TuiResult terminal_get_size(Terminal *t)
{
    struct winsize ws;

    if (ioctl(t->fd, TIOCGWINSZ, &ws) == -1) {
        return TUI_ERR_IOCTL;
    }
    t->rows = ws.ws_row;
    t->cols = ws.ws_col;
    return TUI_OK;
}

void terminal_write_seq(Terminal *t, const char *seq, size_t len)
{
    if (t->fd >= 0 && seq != NULL && len > 0) {
        (void)write(t->fd, seq, len);
    }
}

void terminal_ensure_cleanup(void)
{
    if (g_terminal.initialized) {
        tui_mouse_disable();
        tui_screen_free();
        terminal_cursor_show(&g_terminal);
        terminal_clear_screen(&g_terminal);
        terminal_exit_alt_screen(&g_terminal);
        terminal_raw_mode_disable(&g_terminal);
        g_terminal.initialized = 0;
    }
}

TuiResult tui_init(void)
{
    TuiResult res;

    g_terminal.fd = STDIN_FILENO;

    const char *colorterm = getenv("COLORTERM");
    g_terminal.truecolor = (colorterm != NULL &&
        (strcmp(colorterm, "truecolor") == 0 ||
         strcmp(colorterm, "24bit") == 0));

    res = terminal_raw_mode_enable(&g_terminal);
    if (res != TUI_OK) {
        return res;
    }

    res = terminal_get_size(&g_terminal);
    if (res != TUI_OK) {
        terminal_raw_mode_disable(&g_terminal);
        return res;
    }

    terminal_enter_alt_screen(&g_terminal);
    terminal_clear_screen(&g_terminal);
    terminal_cursor_hide(&g_terminal);

    tui_screen_init(g_terminal.rows, g_terminal.cols);

    g_terminal.initialized = 1;

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

    return TUI_OK;
}

void tui_shutdown(void)
{
    terminal_ensure_cleanup();
}

TuiResult tui_get_size(TuiSize *size)
{
    TuiResult res;

    res = terminal_get_size(&g_terminal);
    if (res != TUI_OK) {
        return res;
    }
    size->rows = g_terminal.rows;
    size->cols = g_terminal.cols;
    return TUI_OK;
}
