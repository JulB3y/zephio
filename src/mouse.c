#include "tui_mouse.h"
#include "tui_terminal.h"
#include "tui_ansi.h"
#include "tui_context.h"

TuiResult tui_mouse_enable(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_ENABLE,
                       sizeof(ANSI_MOUSE_ENABLE) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_DRAG_ENABLE,
                       sizeof(ANSI_MOUSE_DRAG_ENABLE) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_SGR_ENABLE,
                       sizeof(ANSI_MOUSE_SGR_ENABLE) - 1);
    return TUI_OK;
}

void tui_mouse_disable(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_SGR_DISABLE,
                       sizeof(ANSI_MOUSE_SGR_DISABLE) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_DRAG_DISABLE,
                       sizeof(ANSI_MOUSE_DRAG_DISABLE) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_DISABLE,
                       sizeof(ANSI_MOUSE_DISABLE) - 1);
}

const char *tui_mouse_action_name(TuiMouseAction action)
{
    switch (action) {
        case TUI_MOUSE_PRESS:      return "Press";
        case TUI_MOUSE_RELEASE:    return "Release";
        case TUI_MOUSE_MOTION:     return "Motion";
        case TUI_MOUSE_WHEEL_UP:   return "WheelUp";
        case TUI_MOUSE_WHEEL_DOWN: return "WheelDown";
        default:                   return "Unknown";
    }
}

const char *tui_mouse_button_name(TuiMouseButton button)
{
    switch (button) {
        case TUI_MOUSE_BTN_NONE:   return "None";
        case TUI_MOUSE_BTN_LEFT:   return "Left";
        case TUI_MOUSE_BTN_MIDDLE: return "Middle";
        case TUI_MOUSE_BTN_RIGHT:  return "Right";
        default:                   return "Unknown";
    }
}
