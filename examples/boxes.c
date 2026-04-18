#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_ansi.h"
#include "tui_input.h"
#include "tui_screen.h"
#include "tui_terminal.h"

#include <stdio.h>
#include <string.h>

static void draw_frame(TuiContext *ctx, int rows, int cols) {
  tui_screen_clear(ctx);

  tui_screen_fill(ctx, 0, 0, cols, 1, " ", TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
  tui_screen_write(ctx, 0, 2, "Phase 3 - Screen Buffer & Rendering Demo", TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4),
                   TUI_ATTR_BOLD);

  char info[64];
  int info_len = snprintf(info, sizeof(info), "Terminal: %dx%d ", cols, rows);
  tui_screen_write(ctx, 0, cols - info_len, info, TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);

  int half_w = cols / 2;
  int box_h = rows > 16 ? 10 : (rows - 6) / 2;
  if (box_h < 3)
    box_h = 3;

  tui_screen_box_single(ctx, 2, 1, half_w - 1, box_h, TUI_COLOR_INDEX(2), TUI_COLOR_INDEX(0), TUI_ATTR_NONE);
  tui_screen_write(ctx, 3, 3, "Single Border Box", TUI_COLOR_INDEX(2), TUI_COLOR_INDEX(0), TUI_ATTR_BOLD);

  int text_y = 5;
  if (text_y < 2 + box_h - 1) {
    tui_screen_write(ctx, text_y, 3, "Colors: ", TUI_COLOR_INDEX(7), TUI_COLOR_INDEX(0), TUI_ATTR_NONE);
    int x = 3 + 8;
    for (int i = 0; i < 8 && x + 3 < half_w - 2; i++) {
      char label[4] = {' ', '0' + i, ' ', 0};
      tui_screen_write(ctx, text_y, x, label, TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(i), TUI_ATTR_NONE);
      x += 3;
    }
  }
  if (text_y + 1 < 2 + box_h - 1) {
    tui_screen_write(ctx, text_y + 1, 3, "Attrs: ", TUI_COLOR_INDEX(6), TUI_COLOR_INDEX(0), TUI_ATTR_NONE);
    int x = 3 + 7;
    if (x + 5 < half_w - 2) {
      tui_screen_write(ctx, text_y + 1, x, "Bold", TUI_COLOR_INDEX(3), TUI_COLOR_INDEX(0), TUI_ATTR_BOLD);
      x += 5;
    }
    if (x + 4 < half_w - 2) {
      tui_screen_write(ctx, text_y + 1, x, "Dim", TUI_COLOR_INDEX(3), TUI_COLOR_INDEX(0), TUI_ATTR_DIM);
      x += 4;
    }
    if (x + 6 < half_w - 2) {
      tui_screen_write(ctx, text_y + 1, x, "Under", TUI_COLOR_INDEX(5), TUI_COLOR_INDEX(0), TUI_ATTR_UNDERLINE);
      x += 6;
    }
  }

  tui_screen_box_double(ctx, 2, half_w, cols - half_w - 1, box_h, TUI_COLOR_INDEX(4), TUI_COLOR_INDEX(0),
                        TUI_ATTR_NONE);
  tui_screen_write(ctx, 3, half_w + 2, "Double Border Box", TUI_COLOR_INDEX(4), TUI_COLOR_INDEX(0), TUI_ATTR_BOLD);

  if (text_y < 2 + box_h - 1) {
    tui_screen_write(ctx, text_y, half_w + 2, "Fill demo:", TUI_COLOR_INDEX(6), TUI_COLOR_INDEX(0), TUI_ATTR_NONE);
    for (int i = 0; i < 5; i++) {
      int bx = half_w + 2 + i * 5;
      if (bx + 4 < cols - 2) {
        tui_screen_fill(ctx, text_y + 1, bx, 4, 2, " ", TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(1 + i * 4), TUI_ATTR_NONE);
      }
    }
  }

  int color_row = 2 + box_h + 1;
  if (color_row < rows - 3) {
    tui_screen_write(ctx, color_row, 1, "256-Color Gradient:", TUI_COLOR_INDEX(14), TUI_COLOR_INDEX(0), TUI_ATTR_BOLD);
    color_row++;
    int bar_cols = cols - 4;
    for (int i = 0; i < bar_cols && i < 256; i++) {
      char block[4] = {' ', 0, 0, 0};
      tui_screen_set_cell(ctx, color_row, 2 + i, block, TUI_COLOR_INDEX(15), TUI_COLOR_INDEX((uint8_t)i),
                          TUI_ATTR_NONE);
    }
  }

  int fill_row = color_row + 2;
  if (fill_row < rows - 3) {
    int inner_w = cols > 12 ? cols - 4 : 8;
    tui_screen_box_single(ctx, fill_row, 1, inner_w, 3, TUI_COLOR_INDEX(5), TUI_COLOR_INDEX(0), TUI_ATTR_NONE);
    tui_screen_fill(ctx, fill_row + 1, 2, inner_w - 2, 1, "\xe2\x95\x90", TUI_COLOR_INDEX(5), TUI_COLOR_INDEX(236),
                    TUI_ATTR_NONE);
    char msg[128];
    int msg_len = snprintf(msg, sizeof(msg),
                           " Buffered rendering with diff-based output ");
    int msg_col = 2 + (inner_w - 2 - msg_len) / 2;
    if (msg_col < 2)
      msg_col = 2;
    tui_screen_write(ctx, fill_row + 1, msg_col, msg, TUI_COLOR_INDEX(13), TUI_COLOR_INDEX(236), TUI_ATTR_BOLD);
  }

  tui_screen_fill(ctx, rows - 1, 0, cols, 1, " ", TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(236), TUI_ATTR_NONE);
  char status[128];
  (void)snprintf(
      status, sizeof(status),
      " Screen: %dx%d  |  Buffer: %d cells  |  Press 'q' or Escape to quit ",
      cols, rows, rows * cols);
  tui_screen_write(ctx, rows - 1, 2, status, TUI_COLOR_INDEX(12), TUI_COLOR_INDEX(236), TUI_ATTR_NONE);

  tui_screen_render(ctx);
}

static int input_callback(const TuiEvent *event, void *user_data) {
  TuiContext *ctx = (TuiContext *)user_data;

  if (event->key == TUI_EVENT_RESIZE) {
    tui_screen_resize(ctx, event->size.rows, event->size.cols);
    draw_frame(ctx, event->size.rows, event->size.cols);
    return 0;
  }

  if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) {
    return 1;
  }
  if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
    return 1;
  }

  return 0;
}

int main(void) {
  TuiContext ctx;

  TuiResult res = tui_init(&ctx);
  if (res != TUI_OK) {
    fprintf(stderr, "tui_init failed: %d\n", res);
    return 1;
  }

  tui_input_init(&ctx);

  TuiSize size = tui_screen_size(&ctx);
  draw_frame(&ctx, size.rows, size.cols);

  tui_input_loop(&ctx, input_callback, &ctx);

  tui_input_shutdown(&ctx);
  tui_shutdown(&ctx);
  return 0;
}
