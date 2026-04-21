#ifndef ZEPHIO_CONTEXT_H
#define ZEPHIO_CONTEXT_H

#include "zephio_terminal.h"
#include "zephio_screen.h"

struct ZephioContext {
    Terminal  terminal;
    ZephioScreen screen;
};

extern _Thread_local ZephioContext *zephio_current_ctx;

#endif
