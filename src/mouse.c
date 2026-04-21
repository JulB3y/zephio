#include "zephio_mouse.h"
#include "zephio_terminal.h"
#include "zephio_ansi.h"
#include "zephio_context.h"

ZephioResult zephio_mouse_enable(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_ENABLE,
                       sizeof(ANSI_MOUSE_ENABLE) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_DRAG_ENABLE,
                       sizeof(ANSI_MOUSE_DRAG_ENABLE) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_SGR_ENABLE,
                       sizeof(ANSI_MOUSE_SGR_ENABLE) - 1);
    return ZEPHIO_OK;
}

void zephio_mouse_disable(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_SGR_DISABLE,
                       sizeof(ANSI_MOUSE_SGR_DISABLE) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_DRAG_DISABLE,
                       sizeof(ANSI_MOUSE_DRAG_DISABLE) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_MOUSE_DISABLE,
                       sizeof(ANSI_MOUSE_DISABLE) - 1);
}

const char *zephio_mouse_action_name(ZephioMouseAction action)
{
    switch (action) {
        case ZEPHIO_MOUSE_PRESS:      return "Press";
        case ZEPHIO_MOUSE_RELEASE:    return "Release";
        case ZEPHIO_MOUSE_MOTION:     return "Motion";
        case ZEPHIO_MOUSE_WHEEL_UP:   return "WheelUp";
        case ZEPHIO_MOUSE_WHEEL_DOWN: return "WheelDown";
        default:                   return "Unknown";
    }
}

const char *zephio_mouse_button_name(ZephioMouseButton button)
{
    switch (button) {
        case ZEPHIO_MOUSE_BTN_NONE:   return "None";
        case ZEPHIO_MOUSE_BTN_LEFT:   return "Left";
        case ZEPHIO_MOUSE_BTN_MIDDLE: return "Middle";
        case ZEPHIO_MOUSE_BTN_RIGHT:  return "Right";
        default:                   return "Unknown";
    }
}
