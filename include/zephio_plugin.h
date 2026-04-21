#ifndef ZEPHIO_PLUGIN_H
#define ZEPHIO_PLUGIN_H

#include "zephio_widget.h"
#include "zephio_export.h"

typedef struct {
    const char *name;
    const char *version;
    size_t widget_struct_size;
    ZephioWidgetVTable *vtable;
    ZephioResult (*init)(ZephioWidget *widget, ZephioContext *ctx, int x, int y, int w, int h);
} ZephioWidgetPlugin;

typedef int (*zephio_plugin_register_fn)(ZephioWidgetPlugin *out);

ZEPHIO_API ZephioResult zephio_plugin_load(ZephioContext *ctx, const char *path, ZephioWidgetPlugin *out);
ZEPHIO_API void         zephio_plugin_unload(ZephioWidgetPlugin *plugin);

#endif