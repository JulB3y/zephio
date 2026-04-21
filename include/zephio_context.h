#ifndef ZEPHIO_CONTEXT_H
#define ZEPHIO_CONTEXT_H

#include "zephio_terminal.h"
#include "zephio_screen.h"
#include "zephio_export.h"

struct ZephioContext {
    Terminal  terminal;
    ZephioScreen screen;
};

ZEPHIO_API extern _Thread_local ZephioContext *zephio_current_ctx;

#endif
