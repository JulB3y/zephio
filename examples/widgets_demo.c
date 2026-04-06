/**
 * @file widgets_demo.c
 * @brief Widget showcase — all widget types with focus management.
 *
 * Demonstrates:
 *   - Widgets: TuiLabel, TuiSeparator, TuiButton, TuiInputField,
 *     TuiList, TuiContainer
 *   - Tab / Shift+Tab focus cycling
 *   - Event dispatch to callbacks (button clicks, list selection,
 *     input field change/submit)
 *   - Resize handling (widget tree rebuilt on resize)
 *   - Press q / Escape to exit
 */

#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_button.h"
#include "tui_container.h"
#include "tui_input.h"
#include "tui_input_field.h"
#include "tui_label.h"
#include "tui_list.h"
#include "tui_screen.h"
#include "tui_separator.h"
#include "tui_widget.h"

#include <stdio.h>
#include <string.h>

typedef struct {
  TuiWidget root;
  TuiLabel title;
  TuiLabel name_label;
  TuiInputField name_input;
  TuiSeparator sep1;
  TuiButton btn_hello;
  TuiButton btn_clear;
  TuiLabel action_label;
  TuiSeparator sep2;
  TuiList menu_list;
  TuiLabel selection_label;
  TuiSeparator sep3;
  TuiLabel status;
  char status_text[120];
  char action_text[80];
  char selection_text[80];
  int initialized;
} Widgets;

static Widgets g_w;

static void on_hello_click(TuiWidget *widget, void *user_data) {
  (void)widget;
  (void)user_data;
  const char *name = tui_input_field_get_text(&g_w.name_input);
  if (name && name[0]) {
    snprintf(g_w.action_text, sizeof(g_w.action_text), "Hello, %s!", name);
  } else {
    snprintf(g_w.action_text, sizeof(g_w.action_text), "Hello, World!");
  }
  tui_label_set_text(&g_w.action_label, g_w.action_text);
  tui_label_set_colors(&g_w.action_label, 10, 0);
}

static void on_clear_click(TuiWidget *widget, void *user_data) {
  (void)widget;
  (void)user_data;
  tui_input_field_set_text(&g_w.name_input, "");
  tui_label_set_text(&g_w.action_label, "Cleared.");
  tui_label_set_colors(&g_w.action_label, 8, 0);
}

static void on_name_change(TuiWidget *widget, const char *text,
                           void *user_data) {
  (void)widget;
  (void)user_data;
  snprintf(g_w.action_text, sizeof(g_w.action_text), "Editing: \"%s\"", text);
  tui_label_set_text(&g_w.action_label, g_w.action_text);
  tui_label_set_colors(&g_w.action_label, 14, 0);
}

static void on_name_submit(TuiWidget *widget, const char *text,
                           void *user_data) {
  (void)widget;
  (void)user_data;
  snprintf(g_w.action_text, sizeof(g_w.action_text), "Submitted: \"%s\"", text);
  tui_label_set_text(&g_w.action_label, g_w.action_text);
  tui_label_set_colors(&g_w.action_label, 11, 0);
}

static void on_list_select(TuiWidget *widget, int index, const char *item,
                           void *user_data) {
  (void)widget;
  (void)user_data;
  (void)index;
  snprintf(g_w.selection_text, sizeof(g_w.selection_text), "Selected: %s",
           item);
  tui_label_set_text(&g_w.selection_label, g_w.selection_text);
  tui_label_set_colors(&g_w.selection_label, 13, 0);
}

static void build_widgets(int rows, int cols) {
  int uw = cols > 60 ? 56 : cols - 4;
  int ux = (cols - uw) / 2;
  if (ux < 1)
    ux = 1;
  int y = 2;

  tui_widget_init(&g_w.root, 0, 0, cols, rows, NULL, NULL);

  tui_label_init(&g_w.title, ux, y, uw, 1,
                 "Widget Demo — Labels, Buttons, Input, List");
  tui_label_set_colors(&g_w.title, 14, 0);
  tui_label_set_attr(&g_w.title, TUI_ATTR_BOLD);
  tui_widget_add_child(&g_w.root, &g_w.title.base);
  y += 2;

  tui_label_init(&g_w.name_label, ux, y, 16, 1, "Your Name:");
  tui_label_set_colors(&g_w.name_label, 15, 0);
  tui_widget_add_child(&g_w.root, &g_w.name_label.base);

  tui_input_field_init(&g_w.name_input, ux + 12, y, uw - 14, 128);
  tui_input_field_set_colors(&g_w.name_input, 15, 235, 0, 12);
  tui_input_field_set_on_change(&g_w.name_input, on_name_change, NULL);
  tui_input_field_set_on_submit(&g_w.name_input, on_name_submit, NULL);
  g_w.name_input.base.focusable = 1;
  tui_widget_add_child(&g_w.root, &g_w.name_input.base);
  y += 2;

  if (y < rows - 8) {
    tui_separator_init_h(&g_w.sep1, ux, y, uw);
    tui_separator_set_colors(&g_w.sep1, 8, 0);
    tui_widget_add_child(&g_w.root, &g_w.sep1.base);
    y += 1;
  }

  if (y < rows - 7) {
    int half = uw / 2 - 1;
    tui_button_init(&g_w.btn_hello, ux, y, half > 6 ? half : 6, 1, "Say Hello");
    tui_button_set_colors(&g_w.btn_hello, 0, 2, 15, 10);
    tui_button_set_on_click(&g_w.btn_hello, on_hello_click, NULL);
    g_w.btn_hello.base.focusable = 1;
    tui_widget_add_child(&g_w.root, &g_w.btn_hello.base);

    int b2x = ux + half + 2;
    if (b2x + 6 < ux + uw) {
      tui_button_init(&g_w.btn_clear, b2x, y, uw - half - 2, 1, "Clear");
      tui_button_set_colors(&g_w.btn_clear, 0, 1, 15, 9);
      tui_button_set_on_click(&g_w.btn_clear, on_clear_click, NULL);
      g_w.btn_clear.base.focusable = 1;
      tui_widget_add_child(&g_w.root, &g_w.btn_clear.base);
    }
    y += 2;
  }

  if (y < rows - 5) {
    tui_label_init(&g_w.action_label, ux, y, uw, 1, "Actions appear here...");
    tui_label_set_colors(&g_w.action_label, 8, 0);
    tui_widget_add_child(&g_w.root, &g_w.action_label.base);
    y += 1;
  }

  if (y < rows - 5) {
    tui_separator_init_h(&g_w.sep2, ux, y, uw);
    tui_separator_set_colors(&g_w.sep2, 8, 0);
    tui_widget_add_child(&g_w.root, &g_w.sep2.base);
    y += 1;
  }

  int remaining = rows - 1 - y - 2;
  if (remaining >= 3) {
    int list_h = remaining > 8 ? 6 : (remaining - 1);
    if (list_h < 2)
      list_h = 2;

    tui_list_init(&g_w.menu_list, ux, y, uw, list_h);
    tui_list_set_colors(&g_w.menu_list, 15, 234, 0, 12);
    tui_list_set_on_select(&g_w.menu_list, on_list_select, NULL);
    g_w.menu_list.base.focusable = 1;
    tui_list_add_item(&g_w.menu_list, "Open File...");
    tui_list_add_item(&g_w.menu_list, "Save");
    tui_list_add_item(&g_w.menu_list, "Save As...");
    tui_list_add_item(&g_w.menu_list, "Settings");
    tui_list_add_item(&g_w.menu_list, "About");
    tui_widget_add_child(&g_w.root, &g_w.menu_list.base);
    y += list_h;

    tui_label_init(&g_w.selection_label, ux, y, uw, 1,
                   "Select a menu item above");
    tui_label_set_colors(&g_w.selection_label, 8, 0);
    tui_widget_add_child(&g_w.root, &g_w.selection_label.base);
    y += 1;
  }

  tui_label_init(&g_w.status, 1, rows - 1, cols - 2, 1,
                 " Tab: next focus | Shift+Tab: prev | q/Esc: quit");
  tui_label_set_colors(&g_w.status, 15, 236);
  tui_widget_add_child(&g_w.root, &g_w.status.base);

  g_w.initialized = 1;
}

static void destroy_widgets(void) {
  if (!g_w.initialized)
    return;
  for (int i = g_w.root.child_count - 1; i >= 0; i--) {
    tui_widget_destroy(g_w.root.children[i]);
  }
  tui_widget_remove_all_children(&g_w.root);
  g_w.initialized = 0;
}

static void draw_frame(int rows, int cols) {
  tui_screen_clear();
  tui_screen_fill(0, 0, cols, 1, " ", 15, 4, TUI_ATTR_BOLD);
  tui_screen_write(0, 2,
                   "Widget Demo  |  All interactive widgets  |  q/Esc to quit",
                   15, 4, TUI_ATTR_BOLD);
  tui_screen_fill(rows - 1, 0, cols, 1, " ", 15, 236, TUI_ATTR_NONE);
  tui_widget_render(&g_w.root);
  tui_screen_render();
}

static int input_callback(const TuiEvent *event, void *user_data) {
  (void)user_data;

  if (event->key == TUI_EVENT_RESIZE) {
    tui_screen_resize(event->size.rows, event->size.cols);
    destroy_widgets();
    build_widgets(event->size.rows, event->size.cols);
    tui_widget_focus_next(&g_w.root);
    draw_frame(event->size.rows, event->size.cols);
    return 0;
  }

  if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) {
    return 1;
  }

  if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
    TuiWidget *focused = tui_widget_get_focused(&g_w.root);
    if (!focused || focused == &g_w.root) {
      return 1;
    }
  }

  if (event->key == TUI_KEY_TAB) {
    if (event->modifiers & TUI_MOD_SHIFT) {
      tui_widget_focus_prev(&g_w.root);
    } else {
      tui_widget_focus_next(&g_w.root);
    }
    tui_widget_mark_dirty_recursive(&g_w.root);
    draw_frame(event->size.rows, event->size.cols);
    return 0;
  }

  TuiWidget *focused = tui_widget_get_focused(&g_w.root);
  if (focused) {
    tui_widget_handle_input(focused, event);
  }

  {
    TuiSize sz = tui_screen_size();
    draw_frame(sz.rows, sz.cols);
  }

  return 0;
}

int main(void) {
  TuiResult res = tui_init();
  if (res != TUI_OK) {
    fprintf(stderr, "tui_init failed: %d\n", res);
    return 1;
  }

  tui_input_init();

  TuiSize size = tui_screen_size();
  build_widgets(size.rows, size.cols);
  tui_widget_focus_next(&g_w.root);
  draw_frame(size.rows, size.cols);

  tui_input_loop(input_callback, NULL);

  destroy_widgets();
  tui_input_shutdown();
  tui_shutdown();
  return 0;
}
