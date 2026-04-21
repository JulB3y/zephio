/**
 * @file tui_table.h
 * @brief Table widget with column headers, sorting, and row selection.
 *
 * Displays data in a grid of rows and columns. Column headers can be
 * clicked to cycle sort order (none -> asc -> desc -> none). Supports
 * keyboard navigation (arrows, PageUp/Down, Home/End), row selection
 * via Enter, and mouse interaction (header click to sort, row click
 * to select). Scrolls vertically when rows exceed the widget height
 * and horizontally when columns exceed the widget width.
 */

#ifndef ZEPHIO_TABLE_H
#define ZEPHIO_TABLE_H

#include "tui_widget.h"

typedef enum {
    TUI_SORT_NONE = 0,
    TUI_SORT_ASC,
    TUI_SORT_DESC
} TuiSortOrder;

typedef void (*TuiTableCallback)(TuiWidget *widget, int row, void *user_data);

typedef struct {
    char            *label;
    int              width;
    TuiSortOrder     sort_order;
} TuiTableColumn;

typedef struct {
    TuiWidget          base;

    TuiTableColumn    *columns;
    int                col_count;
    int                col_capacity;

    char            ***rows;
    int                row_count;
    int                row_capacity;

    int               *sort_mapping;

    int                selected;
    int                scroll_y;
    int                scroll_x;

    int                sort_col;

    TuiColor           fg;
    TuiColor           bg;
    TuiColor           fg_header;
    TuiColor           bg_header;
    TuiColor           fg_selected;
    TuiColor           bg_selected;

    TuiTableCallback   on_select;
    void              *user_data;
} TuiTable;

TuiResult tui_table_init_ctx(TuiTable *table, TuiContext *ctx, int x, int y, int width, int height);

TuiResult tui_table_add_column(TuiTable *table, const char *label, int width);

TuiResult tui_table_add_row(TuiTable *table, const char **cells, int cell_count);

void tui_table_remove_row(TuiTable *table, int row);

void tui_table_clear_rows(TuiTable *table);

int tui_table_get_selected(TuiTable *table);

void tui_table_set_colors(TuiTable *table, TuiColor fg, TuiColor bg,
                          TuiColor fg_header, TuiColor bg_header,
                          TuiColor fg_selected, TuiColor bg_selected);

void tui_table_set_on_select(TuiTable *table, TuiTableCallback callback,
                             void *user_data);

void tui_table_sort_by(TuiTable *table, int col, TuiSortOrder order);

#endif
