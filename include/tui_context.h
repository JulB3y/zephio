#ifndef ZEPHIO_CONTEXT_H
#define ZEPHIO_CONTEXT_H

#include "tui_terminal.h"
#include "tui_screen.h"

struct TuiContext {
    Terminal  terminal;
    TuiScreen screen;
};

extern _Thread_local TuiContext *tui_current_ctx;

#endif
