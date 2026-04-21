#define _POSIX_C_SOURCE 200809L

#include "zephio_input.h"
#include "zephio_terminal.h"
#include "zephio_ansi.h"
#include "zephio_context.h"

#include <poll.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t g_winch_received = 0;

static void input_sigwinch_handler(int sig)
{
    (void)sig;
    g_winch_received = 1;
}

ZephioResult zephio_input_init(ZephioContext *ctx)
{
    (void)ctx;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = input_sigwinch_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa, NULL);
    g_winch_received = 0;
    return ZEPHIO_OK;
}

void zephio_input_shutdown(ZephioContext *ctx)
{
    (void)ctx;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGWINCH, &sa, NULL);
}

static int read_byte(int fd, int timeout_ms)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, timeout_ms);
    if (ret <= 0) {
        return -1;
    }

    unsigned char c;
    ssize_t n = read(fd, &c, 1);
    if (n <= 0) {
        return -1;
    }
    return c;
}

static void apply_key_by_param(int param, ZephioEvent *event)
{
    switch (param) {
        case 1:  event->key = ZEPHIO_KEY_HOME;     break;
        case 2:  event->key = ZEPHIO_KEY_INSERT;    break;
        case 3:  event->key = ZEPHIO_KEY_DELETE;    break;
        case 4:  event->key = ZEPHIO_KEY_END;       break;
        case 5:  event->key = ZEPHIO_KEY_PAGE_UP;   break;
        case 6:  event->key = ZEPHIO_KEY_PAGE_DOWN; break;
        case 11: event->key = ZEPHIO_KEY_F1;        break;
        case 12: event->key = ZEPHIO_KEY_F2;        break;
        case 13: event->key = ZEPHIO_KEY_F3;        break;
        case 14: event->key = ZEPHIO_KEY_F4;        break;
        case 15: event->key = ZEPHIO_KEY_F5;        break;
        case 17: event->key = ZEPHIO_KEY_F6;        break;
        case 18: event->key = ZEPHIO_KEY_F7;        break;
        case 19: event->key = ZEPHIO_KEY_F8;        break;
        case 20: event->key = ZEPHIO_KEY_F9;        break;
        case 21: event->key = ZEPHIO_KEY_F10;       break;
        case 23: event->key = ZEPHIO_KEY_F11;       break;
        case 24: event->key = ZEPHIO_KEY_F12;       break;
        default: event->key = ZEPHIO_KEY_UNKNOWN;   break;
    }
}

static void apply_key_by_letter(int letter, ZephioEvent *event)
{
    switch (letter) {
        case 'A': event->key = ZEPHIO_KEY_UP;    break;
        case 'B': event->key = ZEPHIO_KEY_DOWN;  break;
        case 'C': event->key = ZEPHIO_KEY_RIGHT; break;
        case 'D': event->key = ZEPHIO_KEY_LEFT;  break;
        case 'H': event->key = ZEPHIO_KEY_HOME;  break;
        case 'F': event->key = ZEPHIO_KEY_END;   break;
        case 'P': event->key = ZEPHIO_KEY_F1;    break;
        case 'Q': event->key = ZEPHIO_KEY_F2;    break;
        case 'R': event->key = ZEPHIO_KEY_F3;    break;
        case 'S': event->key = ZEPHIO_KEY_F4;    break;
        default:  event->key = ZEPHIO_KEY_UNKNOWN; break;
    }
}

static void apply_modifiers(int mod_val, ZephioEvent *event)
{
    if (mod_val > 1) {
        int m = mod_val - 1;
        if (m & 1) event->modifiers |= ZEPHIO_MOD_SHIFT;
        if (m & 2) event->modifiers |= ZEPHIO_MOD_ALT;
        if (m & 4) event->modifiers |= ZEPHIO_MOD_CTRL;
    }
}

static void parse_csi_from(int fd, int first_byte, ZephioEvent *event)
{
    int params[8] = {0};
    int param_count = 0;
    int current_num = -1;
    int final_byte = 0;
    int b = first_byte;

    while (param_count < 8) {
        if (b >= '0' && b <= '9') {
            if (current_num < 0) current_num = 0;
            current_num = current_num * 10 + (b - '0');
        } else if (b == ';') {
            params[param_count++] = (current_num >= 0) ? current_num : 0;
            current_num = -1;
        } else if (b >= 0x40 && b <= 0x7E) {
            params[param_count++] = (current_num >= 0) ? current_num : 1;
            final_byte = b;
            break;
        } else {
            params[param_count++] = (current_num >= 0) ? current_num : 0;
            current_num = -1;
        }

        b = read_byte(fd, 25);
        if (b < 0) {
            event->key = ZEPHIO_KEY_UNKNOWN;
            return;
        }
    }

    if (final_byte == 0) {
        event->key = ZEPHIO_KEY_UNKNOWN;
        return;
    }

    if (param_count >= 2) {
        apply_modifiers(params[1], event);
    }

    if (final_byte == 'Z') {
        event->key = ZEPHIO_KEY_TAB;
        event->modifiers |= ZEPHIO_MOD_SHIFT;
    } else if (final_byte == '~') {
        apply_key_by_param((param_count > 0) ? params[0] : 1, event);
    } else {
        apply_key_by_letter(final_byte, event);
    }
}

static void parse_sgr_mouse(int fd, ZephioEvent *event)
{
    int params[3] = {0, 0, 0};
    int param_count = 0;
    int current_num = -1;
    int final_byte = 0;

    while (param_count < 3) {
        int b = read_byte(fd, 25);
        if (b < 0) {
            event->key = ZEPHIO_KEY_UNKNOWN;
            return;
        }

        if (b >= '0' && b <= '9') {
            if (current_num < 0) current_num = 0;
            current_num = current_num * 10 + (b - '0');
        } else if (b == ';') {
            if (param_count < 3) {
                params[param_count++] = (current_num >= 0) ? current_num : 0;
            }
            current_num = -1;
        } else if (b == 'M' || b == 'm') {
            params[param_count] = (current_num >= 0) ? current_num : 0;
            final_byte = b;
            break;
        } else {
            event->key = ZEPHIO_KEY_UNKNOWN;
            return;
        }
    }

    if (final_byte == 0) {
        event->key = ZEPHIO_KEY_UNKNOWN;
        return;
    }

    int cb = params[0];
    int cx = params[1];
    int cy = params[2];

    if (cx > 0) cx--;
    if (cy > 0) cy--;

    event->key = ZEPHIO_EVENT_MOUSE;
    event->mouse.col = cx;
    event->mouse.row = cy;
    event->mouse.modifiers = ZEPHIO_MOD_NONE;

    if (cb & 4)  event->mouse.modifiers |= ZEPHIO_MOD_SHIFT;
    if (cb & 8)  event->mouse.modifiers |= ZEPHIO_MOD_ALT;
    if (cb & 16) event->mouse.modifiers |= ZEPHIO_MOD_CTRL;

    int btn = cb & 0x03;
    int motion = cb & 0x20;
    int wheel  = cb & 0x40;

    if (wheel) {
        event->mouse.button = ZEPHIO_MOUSE_BTN_NONE;
        event->mouse.action = (btn == 0) ? ZEPHIO_MOUSE_WHEEL_UP : ZEPHIO_MOUSE_WHEEL_DOWN;
    } else if (motion) {
        event->mouse.button = (ZephioMouseButton)(btn + 1);
        event->mouse.action = ZEPHIO_MOUSE_MOTION;
    } else if (final_byte == 'M') {
        if (btn <= 2) {
            event->mouse.button = (ZephioMouseButton)(btn + 1);
        } else {
            event->mouse.button = ZEPHIO_MOUSE_BTN_NONE;
        }
        event->mouse.action = ZEPHIO_MOUSE_PRESS;
    } else {
        event->mouse.button = ZEPHIO_MOUSE_BTN_NONE;
        event->mouse.action = ZEPHIO_MOUSE_RELEASE;
    }
}

static void parse_escape(int fd, ZephioEvent *event)
{
    int b = read_byte(fd, 25);
    if (b < 0) {
        event->key = ZEPHIO_KEY_ESCAPE;
        return;
    }

    if (b == '[') {
        int c = read_byte(fd, 25);
        if (c < 0) {
            event->key = ZEPHIO_KEY_UNKNOWN;
            return;
        }
        if (c == '<') {
            parse_sgr_mouse(fd, event);
        } else {
            parse_csi_from(fd, c, event);
        }
    } else if (b == 'O') {
        int c1 = read_byte(fd, 25);
        if (c1 < 0) {
            event->key = ZEPHIO_KEY_UNKNOWN;
            return;
        }
        apply_key_by_letter(c1, event);
    } else if (b == 0x1B) {
        event->key = ZEPHIO_KEY_ESCAPE;
        event->modifiers |= ZEPHIO_MOD_ALT;
    } else {
        event->key = ZEPHIO_KEY_UNKNOWN;
        event->modifiers |= ZEPHIO_MOD_ALT;

        if (b >= 'a' && b <= 'z') {
            event->codepoint = (uint32_t)b;
        } else if (b >= 'A' && b <= 'Z') {
            event->codepoint = (uint32_t)b;
        }
    }
}

static int decode_utf8(int first_byte, int fd, uint32_t *out_codepoint)
{
    int num_bytes;
    uint32_t cp;

    if ((first_byte & 0x80) == 0x00) {
        *out_codepoint = (uint32_t)first_byte;
        return 1;
    } else if ((first_byte & 0xE0) == 0xC0) {
        num_bytes = 2;
        cp = first_byte & 0x1F;
    } else if ((first_byte & 0xF0) == 0xE0) {
        num_bytes = 3;
        cp = first_byte & 0x0F;
    } else if ((first_byte & 0xF8) == 0xF0) {
        num_bytes = 4;
        cp = first_byte & 0x07;
    } else {
        *out_codepoint = (uint32_t)first_byte;
        return 1;
    }

    for (int i = 1; i < num_bytes; i++) {
        int b = read_byte(fd, 5);
        if (b < 0 || (b & 0xC0) != 0x80) {
            *out_codepoint = (uint32_t)first_byte;
            return 1;
        }
        cp = (cp << 6) | (b & 0x3F);
    }

    *out_codepoint = cp;
    return num_bytes;
}

ZephioResult zephio_input_poll(ZephioContext *ctx, ZephioEvent *event)
{
    memset(event, 0, sizeof(*event));

    if (g_winch_received) {
        g_winch_received = 0;
        event->key = ZEPHIO_EVENT_RESIZE;
        zephio_get_size(ctx, &event->size);
        return ZEPHIO_OK;
    }

    int fd = ctx->terminal.fd;
    struct pollfd pfd = { fd, POLLIN, 0 };
    int ret = poll(&pfd, 1, -1);

    if (g_winch_received) {
        g_winch_received = 0;
        event->key = ZEPHIO_EVENT_RESIZE;
        zephio_get_size(ctx, &event->size);
        return ZEPHIO_OK;
    }

    if (ret <= 0) {
        return TUI_ERR_WRITE;
    }

    unsigned char c;
    ssize_t n = read(fd, &c, 1);
    if (n <= 0) {
        return TUI_ERR_WRITE;
    }

    if (c == 0x1B) {
        parse_escape(fd, event);
    } else if (c < 0x20) {
        event->key = (ZephioKey)c;
        event->modifiers |= ZEPHIO_MOD_CTRL;

        if (c == 0x0D) {
            event->key = ZEPHIO_KEY_ENTER;
            event->modifiers = ZEPHIO_MOD_NONE;
        } else if (c == 0x09) {
            event->key = ZEPHIO_KEY_TAB;
            event->modifiers = ZEPHIO_MOD_NONE;
        }
    } else if (c == 0x7F) {
        event->key = ZEPHIO_KEY_BACKSPACE;
    } else if (c >= 0x20 && c < 0x7F) {
        event->codepoint = (uint32_t)c;
        event->key = ZEPHIO_KEY_UNKNOWN;
    } else if (c >= 0x80) {
        int num = decode_utf8(c, fd, &event->codepoint);
        (void)num;
        event->key = ZEPHIO_KEY_UNKNOWN;
    }

    return ZEPHIO_OK;
}

int zephio_input_loop(ZephioContext *ctx, ZephioInputCallback callback, void *user_data)
{
    ZephioEvent event;

    while (1) {
        ZephioResult res = zephio_input_poll(ctx, &event);
        if (res != ZEPHIO_OK) {
            return -1;
        }

        int action = callback(&event, user_data);
        if (action != 0) {
            return action;
        }
    }
}

const char *zephio_key_name(ZephioKey key)
{
    switch (key) {
        case ZEPHIO_KEY_UNKNOWN:    return "Unknown";
        case ZEPHIO_KEY_ENTER:      return "Enter";
        case ZEPHIO_KEY_TAB:        return "Tab";
        case ZEPHIO_KEY_BACKSPACE:  return "Backspace";
        case ZEPHIO_KEY_ESCAPE:     return "Escape";
        case ZEPHIO_KEY_CTRL_A:     return "Ctrl+A";
        case ZEPHIO_KEY_CTRL_B:     return "Ctrl+B";
        case ZEPHIO_KEY_CTRL_C:     return "Ctrl+C";
        case ZEPHIO_KEY_CTRL_D:     return "Ctrl+D";
        case ZEPHIO_KEY_CTRL_E:     return "Ctrl+E";
        case ZEPHIO_KEY_CTRL_F:     return "Ctrl+F";
        case ZEPHIO_KEY_CTRL_G:     return "Ctrl+G";
        case ZEPHIO_KEY_CTRL_H:     return "Ctrl+H";
        case ZEPHIO_KEY_CTRL_J:     return "Ctrl+J";
        case ZEPHIO_KEY_CTRL_K:     return "Ctrl+K";
        case ZEPHIO_KEY_CTRL_L:     return "Ctrl+L";
        case ZEPHIO_KEY_CTRL_N:     return "Ctrl+N";
        case ZEPHIO_KEY_CTRL_O:     return "Ctrl+O";
        case ZEPHIO_KEY_CTRL_P:     return "Ctrl+P";
        case ZEPHIO_KEY_CTRL_Q:     return "Ctrl+Q";
        case ZEPHIO_KEY_CTRL_R:     return "Ctrl+R";
        case ZEPHIO_KEY_CTRL_S:     return "Ctrl+S";
        case ZEPHIO_KEY_CTRL_T:     return "Ctrl+T";
        case ZEPHIO_KEY_CTRL_U:     return "Ctrl+U";
        case ZEPHIO_KEY_CTRL_V:     return "Ctrl+V";
        case ZEPHIO_KEY_CTRL_W:     return "Ctrl+W";
        case ZEPHIO_KEY_CTRL_X:     return "Ctrl+X";
        case ZEPHIO_KEY_CTRL_Y:     return "Ctrl+Y";
        case ZEPHIO_KEY_CTRL_Z:     return "Ctrl+Z";
        case ZEPHIO_KEY_UP:         return "Up";
        case ZEPHIO_KEY_DOWN:       return "Down";
        case ZEPHIO_KEY_RIGHT:      return "Right";
        case ZEPHIO_KEY_LEFT:       return "Left";
        case ZEPHIO_KEY_HOME:       return "Home";
        case ZEPHIO_KEY_END:        return "End";
        case ZEPHIO_KEY_INSERT:     return "Insert";
        case ZEPHIO_KEY_DELETE:     return "Delete";
        case ZEPHIO_KEY_PAGE_UP:    return "PageUp";
        case ZEPHIO_KEY_PAGE_DOWN:  return "PageDown";
        case ZEPHIO_KEY_F1:         return "F1";
        case ZEPHIO_KEY_F2:         return "F2";
        case ZEPHIO_KEY_F3:         return "F3";
        case ZEPHIO_KEY_F4:         return "F4";
        case ZEPHIO_KEY_F5:         return "F5";
        case ZEPHIO_KEY_F6:         return "F6";
        case ZEPHIO_KEY_F7:         return "F7";
        case ZEPHIO_KEY_F8:         return "F8";
        case ZEPHIO_KEY_F9:         return "F9";
        case ZEPHIO_KEY_F10:        return "F10";
        case ZEPHIO_KEY_F11:        return "F11";
        case ZEPHIO_KEY_F12:        return "F12";
        case ZEPHIO_EVENT_RESIZE:   return "Resize";
        case ZEPHIO_EVENT_MOUSE:    return "Mouse";
        default:                 return "?";
    }
}

const char *zephio_modifier_name(int mod)
{
    if (mod == ZEPHIO_MOD_NONE) return "";
    if (mod == (ZEPHIO_MOD_CTRL | ZEPHIO_MOD_SHIFT)) return "Ctrl+Shift+";
    if (mod == (ZEPHIO_MOD_CTRL | ZEPHIO_MOD_ALT))   return "Ctrl+Alt+";
    if (mod == (ZEPHIO_MOD_ALT | ZEPHIO_MOD_SHIFT))   return "Alt+Shift+";
    if (mod == ZEPHIO_MOD_CTRL)  return "Ctrl+";
    if (mod == ZEPHIO_MOD_SHIFT) return "Shift+";
    if (mod == ZEPHIO_MOD_ALT)   return "Alt+";
    return "?+";
}
