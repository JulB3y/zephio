/**
 * @file mouse_demo.c
 * @brief Mouse event demo — interactive widgets with mouse and focus.
 *
 * Demonstrates:
 *   - Mouse events via zephio_widget_handle_mouse()
 *   - Double-buffered rendering (flicker-free)
 *   - Widget tree with focus management (Tab / Shift+Tab)
 *   - Event dispatch to callbacks (button clicks, list selection,
 *     input field change/submit)
 *   - Resize handling with focus restoration
 *   - Press q / Escape to exit
 */

#define _POSIX_C_SOURCE 200809L

#include "zephio.h"
#include "zephio_app.h"
#include "zephio_button.h"
#include "zephio_container.h"
#include "zephio_input_field.h"
#include "zephio_label.h"
#include "zephio_list.h"
#include "zephio_screen.h"
#include "zephio_separator.h"
#include "zephio_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  ZephioWidget root;
  ZephioLabel title;
  ZephioLabel mouse_info;
  ZephioButton btn_click;
  ZephioButton btn_reset;
  ZephioList list;
  ZephioInputField input;
  ZephioLabel status;
  ZephioContainer panel_bg;
  ZephioSeparator sep;
  ZephioLabel focus_label;
  char mouse_text[80];
  char status_text[80];
  char focus_text[80];
  int click_count;
} AppWidgets;

static void draw_background(ZephioApp *app, int rows, int cols) {
  zephio_screen_clear(app->ctx);
  zephio_screen_fill(app->ctx, 0, 0, cols, 1, " ", ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
  zephio_screen_write(app->ctx, 
      0, 2, "Mouse Demo  |  Click widgets, Tab to focus  |  q/Esc to quit", ZEPHIO_COLOR_INDEX(15),
      ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
  zephio_screen_fill(app->ctx, rows - 1, 0, cols, 1, " ", ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);
}

static void update_status(AppWidgets *w) {
  snprintf(w->status_text, sizeof(w->status_text),
           " Clicks: %d  |  Tab/Shift+Tab: cycle focus  |  Click: activate",
           w->click_count);
  zephio_label_set_text(&w->status, w->status_text);
}

static void update_focus_label(AppWidgets *w) {
  ZephioWidget *focused = zephio_widget_get_focused(&w->root);
  if (focused) {
    snprintf(w->focus_text, sizeof(w->focus_text),
             "Focused: widget at (%d,%d) %dx%d", focused->x, focused->y,
             focused->width, focused->height);
  } else {
    snprintf(w->focus_text, sizeof(w->focus_text), "No widget focused");
  }
  zephio_label_set_text(&w->focus_label, w->focus_text);
}

static void on_click(ZephioWidget *widget, void *user_data) {
  (void)widget;
  AppWidgets *w = (AppWidgets *)user_data;
  w->click_count++;
  update_status(w);
}

static void on_reset(ZephioWidget *widget, void *user_data) {
  (void)widget;
  AppWidgets *w = (AppWidgets *)user_data;
  w->click_count = 0;
  zephio_input_field_set_text(&w->input, "");
  update_status(w);
}

static void on_list_select(ZephioWidget *widget, int index, const char *item,
                           void *user_data) {
  (void)widget;
  (void)index;
  AppWidgets *w = (AppWidgets *)user_data;
  snprintf(w->status_text, sizeof(w->status_text),
           " Selected: %s  |  Clicks: %d", item, w->click_count);
  zephio_label_set_text(&w->status, w->status_text);
}

static void on_input_change(ZephioWidget *widget, const char *text,
                            void *user_data) {
  (void)widget;
  AppWidgets *w = (AppWidgets *)user_data;
  snprintf(w->status_text, sizeof(w->status_text),
           " Typing: \"%s\"  |  Clicks: %d", text, w->click_count);
  zephio_label_set_text(&w->status, w->status_text);
}

static void on_input_submit(ZephioWidget *widget, const char *text,
                            void *user_data) {
  (void)widget;
  AppWidgets *w = (AppWidgets *)user_data;
  snprintf(w->status_text, sizeof(w->status_text),
           " Submitted: \"%s\"  |  Clicks: %d", text, w->click_count);
  zephio_label_set_text(&w->status, w->status_text);
}

static void build_widgets(AppWidgets *w, int rows, int cols, ZephioContext *ctx) {
  int usable_w = cols > 60 ? 56 : cols - 4;
  int usable_x = (cols - usable_w) / 2;
  if (usable_x < 1)
    usable_x = 1;

  zephio_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

  zephio_label_init_ctx(&w->title, ctx, usable_x, 2, usable_w, 1,
                 "Mouse Demo — click buttons, select items, type text");
  zephio_label_set_colors(&w->title, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
  zephio_label_set_attr(&w->title, ZEPHIO_ATTR_BOLD);
  zephio_widget_add_child(&w->root, &w->title.base);

  zephio_label_init_ctx(&w->mouse_info, ctx, usable_x, 3, usable_w, 1,
                 "Mouse: (waiting...)");
  zephio_label_set_colors(&w->mouse_info, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
  zephio_widget_add_child(&w->root, &w->mouse_info.base);

  zephio_container_init_ctx(&w->panel_bg, ctx, usable_x, 4, usable_w, 5);
  zephio_container_set_bg(&w->panel_bg, ZEPHIO_COLOR_INDEX(234));
  zephio_widget_add_child(&w->root, &w->panel_bg.base);

  zephio_button_init_ctx(&w->btn_click, ctx, usable_x + 2, 5,
                       usable_w > 20 ? 18 : usable_w / 2, 1, "Click Me");
  zephio_button_set_colors(&w->btn_click, ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(2), ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(10));
  zephio_button_set_on_click(&w->btn_click, on_click, w);
  w->btn_click.base.focusable = 1;
  zephio_widget_add_child(&w->root, &w->btn_click.base);

  int btn2_x = usable_x + 2 + (usable_w > 20 ? 20 : usable_w / 2 + 1);
  if (btn2_x + 10 < usable_x + usable_w) {
    zephio_button_init_ctx(&w->btn_reset, ctx, btn2_x, 5, 10, 1, "Reset");
    zephio_button_set_colors(&w->btn_reset, ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(1), ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(9));
    zephio_button_set_on_click(&w->btn_reset, on_reset, w);
    w->btn_reset.base.focusable = 1;
    zephio_widget_add_child(&w->root, &w->btn_reset.base);
  }

  zephio_label_init_ctx(&w->focus_label, ctx, usable_x + 2, 7, usable_w - 4, 1,
                 "No widget focused");
  zephio_label_set_colors(&w->focus_label, ZEPHIO_COLOR_INDEX(11), ZEPHIO_COLOR_INDEX(0));
  zephio_widget_add_child(&w->root, &w->focus_label.base);

  int sep_y = 10;
  if (sep_y < rows - 4) {
    zephio_separator_init_h_ctx(&w->sep, ctx, usable_x, sep_y, usable_w);
    zephio_separator_set_colors(&w->sep, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->sep.base);
  }

  int list_y = sep_y + 2;
  int list_h = rows > 24 ? 6 : (rows - list_y - 6);
  if (list_h < 2)
    list_h = 2;
  if (list_y + list_h < rows - 4) {
    zephio_list_init_ctx(&w->list, ctx, usable_x, list_y, usable_w / 2, list_h);
    zephio_list_set_colors(&w->list, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(234), ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12));
    zephio_list_set_on_select(&w->list, on_list_select, w);
    w->list.base.focusable = 1;
    zephio_list_add_item(&w->list, "Option Alpha");
    zephio_list_add_item(&w->list, "Option Beta");
    zephio_list_add_item(&w->list, "Option Gamma");
    zephio_list_add_item(&w->list, "Option Delta");
    zephio_widget_add_child(&w->root, &w->list.base);
  }

  int input_x = usable_x + usable_w / 2 + 2;
  int input_w = usable_w / 2 - 2;
  if (input_x + input_w > cols - 2)
    input_w = cols - 2 - input_x;
  if (input_w > 6 && list_y + list_h < rows - 4) {
    zephio_input_field_init_ctx(&w->input, ctx, input_x, list_y, input_w, 128);
    zephio_input_field_set_colors(&w->input, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(235), ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12));
    zephio_input_field_set_on_change(&w->input, on_input_change, w);
    zephio_input_field_set_on_submit(&w->input, on_input_submit, w);
    w->input.base.focusable = 1;
    zephio_widget_add_child(&w->root, &w->input.base);
  }

  zephio_label_init_ctx(&w->status, ctx, 1, rows - 1, cols - 2, 1,
                 " Clicks: 0  |  Tab: cycle focus");
  zephio_label_set_colors(&w->status, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));
  zephio_widget_add_child(&w->root, &w->status.base);
}

static void destroy_widgets(AppWidgets *w) {
  for (int i = w->root.child_count - 1; i >= 0; i--) {
    zephio_widget_destroy(w->root.children[i]);
  }
  zephio_widget_remove_all_children(&w->root);
}

static int on_init(ZephioApp *app, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;
  memset(w, 0, sizeof(*w));

  ZephioSize size = zephio_screen_size(app->ctx);
  build_widgets(w, size.rows, size.cols, app->ctx);
  return 0;
}

static int on_resize(ZephioApp *app, int rows, int cols, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;

  ZephioWidget *prev_focused = zephio_widget_get_focused(&w->root);
  int had_focus = prev_focused != NULL;
  int prev_tab = had_focus ? prev_focused->tab_index : -1;

  destroy_widgets(w);
  build_widgets(w, rows, cols, app->ctx);

  if (had_focus) {
    zephio_widget_focus_next(&w->root);
    for (int i = 0; i < w->root.child_count; i++) {
      ZephioWidget *child = w->root.children[i];
      if (child->focusable && child->tab_index >= prev_tab) {
        zephio_widget_focus(child);
        break;
      }
    }
  }

  update_focus_label(w);
  return 0;
}

static int on_render(ZephioApp *app, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;
  ZephioSize size = zephio_screen_size(app->ctx);

  draw_background(app, size.rows, size.cols);
  zephio_widget_render(&w->root);
  zephio_screen_render(app->ctx);
  return 0;
}

static int on_input(ZephioApp *app, const ZephioEvent *event, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;

  if (event->key == ZEPHIO_KEY_ESCAPE || event->key == ZEPHIO_KEY_CTRL_C) {
    return 1;
  }

  if (event->codepoint == 'q' && event->modifiers == ZEPHIO_MOD_NONE) {
    if (!zephio_widget_get_focused(&w->root) ||
        !(zephio_widget_get_focused(&w->root)->data)) {
      return 1;
    }
  }

  if (event->key == ZEPHIO_KEY_TAB) {
    if (event->modifiers & ZEPHIO_MOD_SHIFT) {
      zephio_widget_focus_prev(&w->root);
    } else {
      zephio_widget_focus_next(&w->root);
    }
    zephio_widget_mark_dirty_recursive(&w->root);
    update_focus_label(w);
    return 0;
  }

  ZephioWidget *focused = zephio_widget_get_focused(&w->root);
  if (focused) {
    zephio_widget_handle_input(focused, event);
  }

  return 0;
}

static int on_mouse(ZephioApp *app, const ZephioMouseEvent *mouse, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;

  snprintf(w->mouse_text, sizeof(w->mouse_text),
           "Mouse: %s %s at (%d,%d) mod=0x%X",
           zephio_mouse_button_name(mouse->button),
           zephio_mouse_action_name(mouse->action), mouse->col, mouse->row,
           mouse->modifiers);
  zephio_label_set_text(&w->mouse_info, w->mouse_text);

  if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
    ZephioWidget *target = zephio_widget_find_at(&w->root, mouse->row, mouse->col);
    if (target && target->focusable) {
      zephio_widget_focus(target);
      update_focus_label(w);
    }
  }

  zephio_widget_handle_mouse(&w->root, mouse);
  zephio_widget_mark_dirty_recursive(&w->root);

  return 0;
}

static void on_shutdown(ZephioApp *app, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;
  destroy_widgets(w);
}

int main(void) {
  AppWidgets widgets;

  ZephioContext ctx;

  ZephioAppConfig config = {.on_init = on_init,
                         .on_resize = on_resize,
                         .on_render = on_render,
                         .on_input = on_input,
                         .on_mouse = on_mouse,
                         .on_shutdown = on_shutdown,
                         .user_data = &widgets,
                         .tick_rate_ms = 50};

  ZephioApp *app = zephio_app_new(&ctx, &config);
  if (!app) {
    fprintf(stderr, "Failed to create app\n");
    return 1;
  }

  int ret = zephio_app_run(app);
  zephio_app_free(app);
  return ret;
}
