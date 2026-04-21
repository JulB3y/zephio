/**
 * @file widgets_demo.c
 * @brief Widget showcase — all widget types with focus management.
 *
 * Demonstrates:
 *   - Widgets: ZephioLabel, ZephioSeparator, ZephioButton, ZephioInputField,
 *     ZephioList, ZephioContainer
 *   - Tab / Shift+Tab focus cycling
 *   - Event dispatch to callbacks (button clicks, list selection,
 *     input field change/submit)
 *   - Resize handling (widget tree rebuilt on resize)
 *   - Press q / Escape to exit
 */

#define _POSIX_C_SOURCE 200809L

#include "zephio.h"
#include "zephio_button.h"
#include "zephio_container.h"
#include "zephio_input.h"
#include "zephio_input_field.h"
#include "zephio_label.h"
#include "zephio_list.h"
#include "zephio_screen.h"
#include "zephio_separator.h"
#include "zephio_widget.h"

#include <stdio.h>
#include <string.h>

typedef struct {
  ZephioWidget root;
  ZephioLabel title;
  ZephioLabel name_label;
  ZephioInputField name_input;
  ZephioSeparator sep1;
  ZephioButton btn_hello;
  ZephioButton btn_clear;
  ZephioLabel action_label;
  ZephioSeparator sep2;
  ZephioList menu_list;
  ZephioLabel selection_label;
  ZephioSeparator sep3;
  ZephioLabel status;
  char status_text[120];
  char action_text[80];
  char selection_text[80];
  int initialized;
} Widgets;

static Widgets g_w;

static void on_hello_click(ZephioWidget *widget, void *user_data) {
  (void)widget;
  (void)user_data;
  const char *name = zephio_input_field_get_text(&g_w.name_input);
  if (name && name[0]) {
    snprintf(g_w.action_text, sizeof(g_w.action_text), "Hello, %s!", name);
  } else {
    snprintf(g_w.action_text, sizeof(g_w.action_text), "Hello, World!");
  }
  zephio_label_set_text(&g_w.action_label, g_w.action_text);
  zephio_label_set_colors(&g_w.action_label, ZEPHIO_COLOR_INDEX(10), ZEPHIO_COLOR_INDEX(0));
}

static void on_clear_click(ZephioWidget *widget, void *user_data) {
  (void)widget;
  (void)user_data;
  zephio_input_field_set_text(&g_w.name_input, "");
  zephio_label_set_text(&g_w.action_label, "Cleared.");
  zephio_label_set_colors(&g_w.action_label, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
}

static void on_name_change(ZephioWidget *widget, const char *text,
                           void *user_data) {
  (void)widget;
  (void)user_data;
  snprintf(g_w.action_text, sizeof(g_w.action_text), "Editing: \"%s\"", text);
  zephio_label_set_text(&g_w.action_label, g_w.action_text);
  zephio_label_set_colors(&g_w.action_label, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
}

static void on_name_submit(ZephioWidget *widget, const char *text,
                           void *user_data) {
  (void)widget;
  (void)user_data;
  snprintf(g_w.action_text, sizeof(g_w.action_text), "Submitted: \"%s\"", text);
  zephio_label_set_text(&g_w.action_label, g_w.action_text);
  zephio_label_set_colors(&g_w.action_label, ZEPHIO_COLOR_INDEX(11), ZEPHIO_COLOR_INDEX(0));
}

static void on_list_select(ZephioWidget *widget, int index, const char *item,
                           void *user_data) {
  (void)widget;
  (void)user_data;
  (void)index;
  snprintf(g_w.selection_text, sizeof(g_w.selection_text), "Selected: %s",
           item);
  zephio_label_set_text(&g_w.selection_label, g_w.selection_text);
  zephio_label_set_colors(&g_w.selection_label, ZEPHIO_COLOR_INDEX(13), ZEPHIO_COLOR_INDEX(0));
}

static void build_widgets(int rows, int cols, ZephioContext *ctx) {
  int uw = cols > 60 ? 56 : cols - 4;
  int ux = (cols - uw) / 2;
  if (ux < 1)
    ux = 1;
  int y = 2;

  zephio_widget_init_ctx(&g_w.root, 0, 0, cols, rows, NULL, ctx, NULL);

  zephio_label_init_ctx(&g_w.title, ctx, ux, y, uw, 1,
                 "Widget Demo — Labels, Buttons, Input, List");
  zephio_label_set_colors(&g_w.title, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
  zephio_label_set_attr(&g_w.title, ZEPHIO_ATTR_BOLD);
  zephio_widget_add_child(&g_w.root, &g_w.title.base);
  y += 2;

  zephio_label_init_ctx(&g_w.name_label, ctx, ux, y, 16, 1, "Your Name:");
  zephio_label_set_colors(&g_w.name_label, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(0));
  zephio_widget_add_child(&g_w.root, &g_w.name_label.base);

  zephio_input_field_init_ctx(&g_w.name_input, ctx, ux + 12, y, uw - 14, 128);
  zephio_input_field_set_colors(&g_w.name_input, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(235), ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12));
  zephio_input_field_set_on_change(&g_w.name_input, on_name_change, NULL);
  zephio_input_field_set_on_submit(&g_w.name_input, on_name_submit, NULL);
  g_w.name_input.base.focusable = 1;
  zephio_widget_add_child(&g_w.root, &g_w.name_input.base);
  y += 2;

  if (y < rows - 8) {
    zephio_separator_init_h_ctx(&g_w.sep1, ctx, ux, y, uw);
    zephio_separator_set_colors(&g_w.sep1, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&g_w.root, &g_w.sep1.base);
    y += 1;
  }

  if (y < rows - 7) {
    int half = uw / 2 - 1;
    zephio_button_init_ctx(&g_w.btn_hello, ctx, ux, y, half > 6 ? half : 6, 1, "Say Hello");
    zephio_button_set_colors(&g_w.btn_hello, ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(2), ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(10));
    zephio_button_set_on_click(&g_w.btn_hello, on_hello_click, NULL);
    g_w.btn_hello.base.focusable = 1;
    zephio_widget_add_child(&g_w.root, &g_w.btn_hello.base);

    int b2x = ux + half + 2;
    if (b2x + 6 < ux + uw) {
      zephio_button_init_ctx(&g_w.btn_clear, ctx, b2x, y, uw - half - 2, 1, "Clear");
      zephio_button_set_colors(&g_w.btn_clear, ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(1), ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(9));
      zephio_button_set_on_click(&g_w.btn_clear, on_clear_click, NULL);
      g_w.btn_clear.base.focusable = 1;
      zephio_widget_add_child(&g_w.root, &g_w.btn_clear.base);
    }
    y += 2;
  }

  if (y < rows - 5) {
    zephio_label_init_ctx(&g_w.action_label, ctx, ux, y, uw, 1, "Actions appear here...");
    zephio_label_set_colors(&g_w.action_label, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&g_w.root, &g_w.action_label.base);
    y += 1;
  }

  if (y < rows - 5) {
    zephio_separator_init_h_ctx(&g_w.sep2, ctx, ux, y, uw);
    zephio_separator_set_colors(&g_w.sep2, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&g_w.root, &g_w.sep2.base);
    y += 1;
  }

  int remaining = rows - 1 - y - 2;
  if (remaining >= 3) {
    int list_h = remaining > 8 ? 6 : (remaining - 1);
    if (list_h < 2)
      list_h = 2;

    zephio_list_init_ctx(&g_w.menu_list, ctx, ux, y, uw, list_h);
    zephio_list_set_colors(&g_w.menu_list, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(234), ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12));
    zephio_list_set_on_select(&g_w.menu_list, on_list_select, NULL);
    g_w.menu_list.base.focusable = 1;
    zephio_list_add_item(&g_w.menu_list, "Open File...");
    zephio_list_add_item(&g_w.menu_list, "Save");
    zephio_list_add_item(&g_w.menu_list, "Save As...");
    zephio_list_add_item(&g_w.menu_list, "Settings");
    zephio_list_add_item(&g_w.menu_list, "About");
    zephio_widget_add_child(&g_w.root, &g_w.menu_list.base);
    y += list_h;

    zephio_label_init_ctx(&g_w.selection_label, ctx, ux, y, uw, 1,
                   "Select a menu item above");
    zephio_label_set_colors(&g_w.selection_label, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&g_w.root, &g_w.selection_label.base);
    y += 1;
  }

  zephio_label_init_ctx(&g_w.status, ctx, 1, rows - 1, cols - 2, 1,
                 " Tab: next focus | Shift+Tab: prev | q/Esc: quit");
  zephio_label_set_colors(&g_w.status, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));
  zephio_widget_add_child(&g_w.root, &g_w.status.base);

  g_w.initialized = 1;
}

static void destroy_widgets(void) {
  if (!g_w.initialized)
    return;
  for (int i = g_w.root.child_count - 1; i >= 0; i--) {
    zephio_widget_destroy(g_w.root.children[i]);
  }
  zephio_widget_remove_all_children(&g_w.root);
  g_w.initialized = 0;
}

static void draw_frame(ZephioContext *ctx, int rows, int cols) {
  zephio_screen_clear(ctx);
  zephio_screen_fill(ctx, 0, 0, cols, 1, " ", ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
  zephio_screen_write(ctx, 0, 2,
                   "Widget Demo  |  All interactive widgets  |  q/Esc to quit",
                   ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
  zephio_screen_fill(ctx, rows - 1, 0, cols, 1, " ", ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);
  zephio_widget_render(&g_w.root);
  zephio_screen_render(ctx);
}

static int input_callback(const ZephioEvent *event, void *user_data) {
  ZephioContext *ctx = (ZephioContext *)user_data;

  if (event->key == ZEPHIO_EVENT_RESIZE) {
    zephio_screen_resize(ctx, event->size.rows, event->size.cols);
    destroy_widgets();
    build_widgets(event->size.rows, event->size.cols, ctx);
    zephio_widget_focus_next(&g_w.root);
    draw_frame(ctx, event->size.rows, event->size.cols);
    return 0;
  }

  if (event->key == ZEPHIO_KEY_ESCAPE || event->key == ZEPHIO_KEY_CTRL_C) {
    return 1;
  }

  if (event->codepoint == 'q' && event->modifiers == ZEPHIO_MOD_NONE) {
    ZephioWidget *focused = zephio_widget_get_focused(&g_w.root);
    if (!focused || focused == &g_w.root) {
      return 1;
    }
  }

  if (event->key == ZEPHIO_KEY_TAB) {
    if (event->modifiers & ZEPHIO_MOD_SHIFT) {
      zephio_widget_focus_prev(&g_w.root);
    } else {
      zephio_widget_focus_next(&g_w.root);
    }
    zephio_widget_mark_dirty_recursive(&g_w.root);
    ZephioSize sz = zephio_screen_size(ctx);
    draw_frame(ctx, sz.rows, sz.cols);
    return 0;
  }

  ZephioWidget *focused = zephio_widget_get_focused(&g_w.root);
  if (focused) {
    zephio_widget_handle_input(focused, event);
  }

  {
    ZephioSize sz = zephio_screen_size(ctx);
    draw_frame(ctx, sz.rows, sz.cols);
  }

  return 0;
}

int main(void) {
  ZephioContext ctx = {0};

  ZephioResult res = zephio_init(&ctx);
  if (res != ZEPHIO_OK) {
    fprintf(stderr, "zephio_init failed: %d\n", res);
    return 1;
  }

  zephio_input_init(&ctx);

  ZephioSize size = zephio_screen_size(&ctx);
  build_widgets(size.rows, size.cols, &ctx);
  zephio_widget_focus_next(&g_w.root);
  draw_frame(&ctx, size.rows, size.cols);

  zephio_input_loop(&ctx, input_callback, &ctx);

  destroy_widgets();
  zephio_input_shutdown(&ctx);
  zephio_shutdown(&ctx);
  return 0;
}
