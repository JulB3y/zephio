#define _POSIX_C_SOURCE 200809L

#include "zephio_plugin.h"
#include "zephio_context.h"
#include "zephio_widget.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void *handle;
    ZephioWidgetPlugin plugin;
} PluginHandle;

ZEPHIO_API ZephioResult zephio_plugin_load(ZephioContext *ctx, const char *path, ZephioWidgetPlugin *out)
{
    (void)ctx; // Unused parameter
    if (!path || !out) return TUI_ERR_MEMORY;

    void *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        return TUI_ERR_IOCTL;
    }

    // Use union to avoid pointer conversion warning
    union {
        void *ptr;
        zephio_plugin_register_fn fn;
    } u;
    u.ptr = dlsym(handle, "zephio_plugin_register");
    if (!u.fn) {
        dlclose(handle);
        return TUI_ERR_IOCTL;
    }

    ZephioWidgetPlugin temp_plugin;
    if (u.fn(&temp_plugin) != 0) {
        dlclose(handle);
        return TUI_ERR_IOCTL;
    }

    // Store the handle in a wrapper struct
    PluginHandle *plugin_handle = malloc(sizeof(PluginHandle));
    if (!plugin_handle) {
        dlclose(handle);
        return TUI_ERR_MEMORY;
    }
    
    plugin_handle->handle = handle;
    plugin_handle->plugin = temp_plugin;
    
    // Return the plugin but store the handle pointer in the vtable field temporarily
    *out = temp_plugin;
    out->vtable = (ZephioWidgetVTable *)plugin_handle;
    
    return ZEPHIO_OK;
}

ZEPHIO_API void zephio_plugin_unload(ZephioWidgetPlugin *plugin)
{
    if (!plugin) return;
    
    PluginHandle *handle = (PluginHandle *)plugin->vtable;
    if (handle) {
        dlclose(handle->handle);
        free(handle);
    }
}