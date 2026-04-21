/**
 * @file zephio_table.h
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

#include "zephio_widget.h"

typedef enum {
    ZEPHIO_SORT_NONE = 0,
    ZEPHIO_SORT_ASC,
    ZEPHIO_SORT_DESC
} ZephioSortOrder;

typedef void (*ZephioTableCallback)(ZephioWidget *widget, int row, void *user_data);

typedef struct {
    char            *label;
    int              width;
    ZephioSortOrder     sort_order;
} ZephioTableColumn;

typedef struct {
    ZephioWidget          base;

    ZephioTableColumn    *columns;
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

    ZephioColor           fg;
    ZephioColor           bg;
    ZephioColor           fg_header;
    ZephioColor           bg_header;
    ZephioColor           fg_selected;
    ZephioColor           bg_selected;

    ZephioTableCallback   on_select;
    void              *user_data;
} ZephioTable;

ZephioResult zephio_table_init_ctx(ZephioTable *table, ZephioContext *ctx, int x, int y, int width, int height);

ZephioResult zephio_table_add_column(ZephioTable *table, const char *label, int width);

ZephioResult zephio_table_add_row(ZephioTable *table, const char **cells, int cell_count);

void zephio_table_remove_row(ZephioTable *table, int row);

void zephio_table_clear_rows(ZephioTable *table);

int zephio_table_get_selected(ZephioTable *table);

void zephio_table_set_colors(ZephioTable *table, ZephioColor fg, ZephioColor bg,
                          ZephioColor fg_header, ZephioColor bg_header,
                          ZephioColor fg_selected, ZephioColor bg_selected);

void zephio_table_set_on_select(ZephioTable *table, ZephioTableCallback callback,
                             void *user_data);

void zephio_table_sort_by(ZephioTable *table, int col, ZephioSortOrder order);

#endif
