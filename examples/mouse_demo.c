/**
 * @file mouse_demo.c
 * @brief Mouse event demo — interactive widgets with mouse and focus.
 *
 * Demonstrates:
 *   - Mouse events via tui_widget_handle_mouse()
 *   - Double-buffered rendering (flicker-free)
 *   - Widget tree with focus management (Tab / Shift+Tab)
 *   - Event dispatch to callbacks (button clicks, list selection,
 *     input field change/submit)
 *   - Resize handling with focus restoration
 *   - Press q / Escape to exit
 */

#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_app.h"
#include "tui_button.h"
#include "tui_container.h"
#include "tui_input_field.h"
#include "tui_label.h"
#include "tui_list.h"
#include "tui_screen.h"
#include "tui_separator.h"
#include "tui_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  TuiWidget root;
  TuiLabel title;
  TuiLabel mouse_info;
  TuiButton btn_click;
  TuiButton btn_reset;
  TuiList list;
  TuiInputField input;
  TuiLabel status;
  TuiContainer panel_bg;
  TuiSeparator sep;
  TuiLabel focus_label;
  char mouse_text[80];
  char status_text[80];
  char focus_text[80];
  int click_count;
} AppWidgets;

static void draw_background(TuiApp *app, int rows, int cols) {
  tui_screen_clear(app->ctx);
  tui_screen_fill(app->ctx, 0, 0, cols, 1, " ", ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
  tui_screen_write(app->ctx, 
      0, 2, "Mouse Demo  |  Click widgets, Tab to focus  |  q/Esc to quit", ZEPHIO_COLOR_INDEX(15),
      ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
  tui_screen_fill(app->ctx, rows - 1, 0, cols, 1, " ", ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);
}

static void update_status(AppWidgets *w) {
  snprintf(w->status_text, sizeof(w->status_text),
           " Clicks: %d  |  Tab/Shift+Tab: cycle focus  |  Click: activate",
           w->click_count);
  tui_label_set_text(&w->status, w->status_text);
}

static void update_focus_label(AppWidgets *w) {
  TuiWidget *focused = tui_widget_get_focused(&w->root);
  if (focused) {
    snprintf(w->focus_text, sizeof(w->focus_text),
             "Focused: widget at (%d,%d) %dx%d", focused->x, focused->y,
             focused->width, focused->height);
  } else {
    snprintf(w->focus_text, sizeof(w->focus_text), "No widget focused");
  }
  tui_label_set_text(&w->focus_label, w->focus_text);
}

static void on_click(TuiWidget *widget, void *user_data) {
  (void)widget;
  AppWidgets *w = (AppWidgets *)user_data;
  w->click_count++;
  update_status(w);
}

static void on_reset(TuiWidget *widget, void *user_data) {
  (void)widget;
  AppWidgets *w = (AppWidgets *)user_data;
  w->click_count = 0;
  tui_input_field_set_text(&w->input, "");
  update_status(w);
}

static void on_list_select(TuiWidget *widget, int index, const char *item,
                           void *user_data) {
  (void)widget;
  (void)index;
  AppWidgets *w = (AppWidgets *)user_data;
  snprintf(w->status_text, sizeof(w->status_text),
           " Selected: %s  |  Clicks: %d", item, w->click_count);
  tui_label_set_text(&w->status, w->status_text);
}

static void on_input_change(TuiWidget *widget, const char *text,
                            void *user_data) {
  (void)widget;
  AppWidgets *w = (AppWidgets *)user_data;
  snprintf(w->status_text, sizeof(w->status_text),
           " Typing: \"%s\"  |  Clicks: %d", text, w->click_count);
  tui_label_set_text(&w->status, w->status_text);
}

static void on_input_submit(TuiWidget *widget, const char *text,
                            void *user_data) {
  (void)widget;
  AppWidgets *w = (AppWidgets *)user_data;
  snprintf(w->status_text, sizeof(w->status_text),
           " Submitted: \"%s\"  |  Clicks: %d", text, w->click_count);
  tui_label_set_text(&w->status, w->status_text);
}

static void build_widgets(AppWidgets *w, int rows, int cols, TuiContext *ctx) {
  int usable_w = cols > 60 ? 56 : cols - 4;
  int usable_x = (cols - usable_w) / 2;
  if (usable_x < 1)
    usable_x = 1;

  tui_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

  tui_label_init_ctx(&w->title, ctx, usable_x, 2, usable_w, 1,
                 "Mouse Demo — click buttons, select items, type text");
  tui_label_set_colors(&w->title, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
  tui_label_set_attr(&w->title, ZEPHIO_ATTR_BOLD);
  tui_widget_add_child(&w->root, &w->title.base);

  tui_label_init_ctx(&w->mouse_info, ctx, usable_x, 3, usable_w, 1,
                 "Mouse: (waiting...)");
  tui_label_set_colors(&w->mouse_info, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
  tui_widget_add_child(&w->root, &w->mouse_info.base);

  tui_container_init_ctx(&w->panel_bg, ctx, usable_x, 4, usable_w, 5);
  tui_container_set_bg(&w->panel_bg, ZEPHIO_COLOR_INDEX(234));
  tui_widget_add_child(&w->root, &w->panel_bg.base);

  tui_button_init_ctx(&w->btn_click, ctx, usable_x + 2, 5,
                       usable_w > 20 ? 18 : usable_w / 2, 1, "Click Me");
  tui_button_set_colors(&w->btn_click, ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(2), ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(10));
  tui_button_set_on_click(&w->btn_click, on_click, w);
  w->btn_click.base.focusable = 1;
  tui_widget_add_child(&w->root, &w->btn_click.base);

  int btn2_x = usable_x + 2 + (usable_w > 20 ? 20 : usable_w / 2 + 1);
  if (btn2_x + 10 < usable_x + usable_w) {
    tui_button_init_ctx(&w->btn_reset, ctx, btn2_x, 5, 10, 1, "Reset");
    tui_button_set_colors(&w->btn_reset, ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(1), ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(9));
    tui_button_set_on_click(&w->btn_reset, on_reset, w);
    w->btn_reset.base.focusable = 1;
    tui_widget_add_child(&w->root, &w->btn_reset.base);
  }

  tui_label_init_ctx(&w->focus_label, ctx, usable_x + 2, 7, usable_w - 4, 1,
                 "No widget focused");
  tui_label_set_colors(&w->focus_label, ZEPHIO_COLOR_INDEX(11), ZEPHIO_COLOR_INDEX(0));
  tui_widget_add_child(&w->root, &w->focus_label.base);

  int sep_y = 10;
  if (sep_y < rows - 4) {
    tui_separator_init_h_ctx(&w->sep, ctx, usable_x, sep_y, usable_w);
    tui_separator_set_colors(&w->sep, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->sep.base);
  }

  int list_y = sep_y + 2;
  int list_h = rows > 24 ? 6 : (rows - list_y - 6);
  if (list_h < 2)
    list_h = 2;
  if (list_y + list_h < rows - 4) {
    tui_list_init_ctx(&w->list, ctx, usable_x, list_y, usable_w / 2, list_h);
    tui_list_set_colors(&w->list, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(234), ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12));
    tui_list_set_on_select(&w->list, on_list_select, w);
    w->list.base.focusable = 1;
    tui_list_add_item(&w->list, "Option Alpha");
    tui_list_add_item(&w->list, "Option Beta");
    tui_list_add_item(&w->list, "Option Gamma");
    tui_list_add_item(&w->list, "Option Delta");
    tui_widget_add_child(&w->root, &w->list.base);
  }

  int input_x = usable_x + usable_w / 2 + 2;
  int input_w = usable_w / 2 - 2;
  if (input_x + input_w > cols - 2)
    input_w = cols - 2 - input_x;
  if (input_w > 6 && list_y + list_h < rows - 4) {
    tui_input_field_init_ctx(&w->input, ctx, input_x, list_y, input_w, 128);
    tui_input_field_set_colors(&w->input, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(235), ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12));
    tui_input_field_set_on_change(&w->input, on_input_change, w);
    tui_input_field_set_on_submit(&w->input, on_input_submit, w);
    w->input.base.focusable = 1;
    tui_widget_add_child(&w->root, &w->input.base);
  }

  tui_label_init_ctx(&w->status, ctx, 1, rows - 1, cols - 2, 1,
                 " Clicks: 0  |  Tab: cycle focus");
  tui_label_set_colors(&w->status, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));
  tui_widget_add_child(&w->root, &w->status.base);
}

static void destroy_widgets(AppWidgets *w) {
  for (int i = w->root.child_count - 1; i >= 0; i--) {
    tui_widget_destroy(w->root.children[i]);
  }
  tui_widget_remove_all_children(&w->root);
}

static int on_init(TuiApp *app, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;
  memset(w, 0, sizeof(*w));

  TuiSize size = tui_screen_size(app->ctx);
  build_widgets(w, size.rows, size.cols, app->ctx);
  return 0;
}

static int on_resize(TuiApp *app, int rows, int cols, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;

  TuiWidget *prev_focused = tui_widget_get_focused(&w->root);
  int had_focus = prev_focused != NULL;
  int prev_tab = had_focus ? prev_focused->tab_index : -1;

  destroy_widgets(w);
  build_widgets(w, rows, cols, app->ctx);

  if (had_focus) {
    tui_widget_focus_next(&w->root);
    for (int i = 0; i < w->root.child_count; i++) {
      TuiWidget *child = w->root.children[i];
      if (child->focusable && child->tab_index >= prev_tab) {
        tui_widget_focus(child);
        break;
      }
    }
  }

  update_focus_label(w);
  return 0;
}

static int on_render(TuiApp *app, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;
  TuiSize size = tui_screen_size(app->ctx);

  draw_background(app, size.rows, size.cols);
  tui_widget_render(&w->root);
  tui_screen_render(app->ctx);
  return 0;
}

static int on_input(TuiApp *app, const TuiEvent *event, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;

  if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) {
    return 1;
  }

  if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
    if (!tui_widget_get_focused(&w->root) ||
        !(tui_widget_get_focused(&w->root)->data)) {
      return 1;
    }
  }

  if (event->key == TUI_KEY_TAB) {
    if (event->modifiers & TUI_MOD_SHIFT) {
      tui_widget_focus_prev(&w->root);
    } else {
      tui_widget_focus_next(&w->root);
    }
    tui_widget_mark_dirty_recursive(&w->root);
    update_focus_label(w);
    return 0;
  }

  TuiWidget *focused = tui_widget_get_focused(&w->root);
  if (focused) {
    tui_widget_handle_input(focused, event);
  }

  return 0;
}

static int on_mouse(TuiApp *app, const TuiMouseEvent *mouse, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;

  snprintf(w->mouse_text, sizeof(w->mouse_text),
           "Mouse: %s %s at (%d,%d) mod=0x%X",
           tui_mouse_button_name(mouse->button),
           tui_mouse_action_name(mouse->action), mouse->col, mouse->row,
           mouse->modifiers);
  tui_label_set_text(&w->mouse_info, w->mouse_text);

  if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
    TuiWidget *target = tui_widget_find_at(&w->root, mouse->row, mouse->col);
    if (target && target->focusable) {
      tui_widget_focus(target);
      update_focus_label(w);
    }
  }

  tui_widget_handle_mouse(&w->root, mouse);
  tui_widget_mark_dirty_recursive(&w->root);

  return 0;
}

static void on_shutdown(TuiApp *app, void *user_data) {
  (void)app;
  AppWidgets *w = (AppWidgets *)user_data;
  destroy_widgets(w);
}

int main(void) {
  AppWidgets widgets;

  TuiContext ctx;

  TuiAppConfig config = {.on_init = on_init,
                         .on_resize = on_resize,
                         .on_render = on_render,
                         .on_input = on_input,
                         .on_mouse = on_mouse,
                         .on_shutdown = on_shutdown,
                         .user_data = &widgets,
                         .tick_rate_ms = 50};

  TuiApp *app = tui_app_new(&ctx, &config);
  if (!app) {
    fprintf(stderr, "Failed to create app\n");
    return 1;
  }

  int ret = tui_app_run(app);
  tui_app_free(app);
  return ret;
}
