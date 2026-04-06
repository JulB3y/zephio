#include "tui_text.h"

#include <string.h>

int tui_utf8_valid(const char *str, size_t len)
{
    if (!str) return 0;

    size_t i = 0;
    while (i < len) {
        unsigned char c = (unsigned char)str[i];

        int expected;
        uint32_t min_cp;

        if (c <= 0x7F) {
            i++;
            continue;
        } else if ((c & 0xE0) == 0xC0) {
            expected = 2;
            min_cp = 0x80;
        } else if ((c & 0xF0) == 0xE0) {
            expected = 3;
            min_cp = 0x800;
        } else if ((c & 0xF8) == 0xF0) {
            expected = 4;
            min_cp = 0x10000;
        } else {
            return 0;
        }

        if (i + (size_t)expected > len) return 0;

        uint32_t cp = (uint32_t)(c & (0xFF >> (expected + 1)));
        for (int j = 1; j < expected; j++) {
            unsigned char b = (unsigned char)str[i + j];
            if ((b & 0xC0) != 0x80) return 0;
            cp = (cp << 6) | (b & 0x3F);
        }

        if (cp < min_cp) return 0;
        if (cp >= 0xD800 && cp <= 0xDFFF) return 0;
        if (cp > 0x10FFFF) return 0;

        i += (size_t)expected;
    }

    return 1;
}

int tui_utf8_encode(uint32_t codepoint, char *buf, size_t buf_size)
{
    if (!buf || buf_size == 0) return 0;

    if (codepoint <= 0x7F) {
        buf[0] = (char)codepoint;
        return 1;
    } else if (codepoint <= 0x7FF) {
        if (buf_size < 2) return 0;
        buf[0] = (char)(0xC0 | (codepoint >> 6));
        buf[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    } else if (codepoint <= 0xFFFF) {
        if (buf_size < 3) return 0;
        buf[0] = (char)(0xE0 | (codepoint >> 12));
        buf[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    } else if (codepoint <= 0x10FFFF) {
        if (buf_size < 4) return 0;
        buf[0] = (char)(0xF0 | (codepoint >> 18));
        buf[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        buf[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buf[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    }

    return 0;
}

int tui_utf8_char_len(unsigned char c)
{
    if ((c & 0x80) == 0x00) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

int tui_utf8_next(const char *str, size_t len, uint32_t *codepoint)
{
    if (!str || len == 0) return 0;

    unsigned char c = (unsigned char)str[0];

    if ((c & 0x80) == 0x00) {
        if (codepoint) *codepoint = (uint32_t)c;
        return 1;
    }

    int expected;
    uint32_t cp;

    if ((c & 0xE0) == 0xC0) {
        expected = 2;
        cp = c & 0x1F;
    } else if ((c & 0xF0) == 0xE0) {
        expected = 3;
        cp = c & 0x0F;
    } else if ((c & 0xF8) == 0xF0) {
        expected = 4;
        cp = c & 0x07;
    } else {
        if (codepoint) *codepoint = (uint32_t)c;
        return 1;
    }

    if ((size_t)expected > len) {
        if (codepoint) *codepoint = (uint32_t)c;
        return 1;
    }

    for (int i = 1; i < expected; i++) {
        unsigned char b = (unsigned char)str[i];
        if ((b & 0xC0) != 0x80) {
            if (codepoint) *codepoint = (uint32_t)c;
            return 1;
        }
        cp = (cp << 6) | (b & 0x3F);
    }

    if (codepoint) *codepoint = cp;
    return expected;
}

int tui_utf8_char_width(uint32_t codepoint)
{
    if (codepoint < 0x20) return 0;
    if (codepoint == 0x7F) return 0;
    if (codepoint >= 0x80 && codepoint <= 0x9F) return 0;
    if (codepoint == 0xAD) return 1;
    if (codepoint <= 0xFF) return 1;
    if (codepoint >= 0x0100 && codepoint <= 0x024F) return 1;
    if (codepoint >= 0x2000 && codepoint <= 0x200B) return 0;
    if (codepoint >= 0x2028 && codepoint <= 0x202F) return 0;
    if (codepoint >= 0x205F && codepoint <= 0x206F) return 0;
    if (codepoint >= 0xFE00 && codepoint <= 0xFE0F) return 0;
    if (codepoint >= 0xFEFF && codepoint <= 0xFEFF) return 0;
    if (codepoint >= 0xFFF0 && codepoint <= 0xFFFF) return 0;
    if (codepoint >= 0x1F000) return 2;
    if (codepoint >= 0x2000 && codepoint <= 0x2BFF) return 1;
    if (codepoint >= 0x3000 && codepoint <= 0x303F) return 2;
    if (codepoint >= 0x3040 && codepoint <= 0x9FFF) return 2;
    if (codepoint >= 0xAC00 && codepoint <= 0xD7AF) return 2;
    if (codepoint >= 0xF900 && codepoint <= 0xFAFF) return 2;
    if (codepoint >= 0xFF00 && codepoint <= 0xFF60) return 1;
    if (codepoint >= 0xFF61 && codepoint <= 0xFFEF) return 2;
    return 1;
}

int tui_text_width(const char *text, size_t len)
{
    if (!text) return 0;

    int width = 0;
    size_t i = 0;

    while (i < len && text[i]) {
        uint32_t cp;
        int clen = tui_utf8_next(text + i, len - i, &cp);
        if (clen == 0) break;

        if (cp == '\t') {
            width += TUI_TAB_SIZE - (width % TUI_TAB_SIZE);
        } else if (cp == '\n' || cp == '\r') {
            break;
        } else {
            width += tui_utf8_char_width(cp);
        }

        i += (size_t)clen;
    }

    return width;
}

int tui_text_str_width(const char *text)
{
    if (!text) return 0;
    return tui_text_width(text, strlen(text));
}

int tui_text_clip(const char *text, size_t len, int max_width, size_t *out_len)
{
    if (!text || max_width <= 0) {
        if (out_len) *out_len = 0;
        return 0;
    }

    int width = 0;
    size_t i = 0;

    while (i < len && text[i]) {
        uint32_t cp;
        int clen = tui_utf8_next(text + i, len - i, &cp);
        if (clen == 0) break;

        if (cp == '\n' || cp == '\r') {
            break;
        }

        int cw;
        if (cp == '\t') {
            cw = TUI_TAB_SIZE - (width % TUI_TAB_SIZE);
        } else {
            cw = tui_utf8_char_width(cp);
        }

        if (width + cw > max_width) {
            break;
        }

        width += cw;
        i += (size_t)clen;
    }

    if (out_len) *out_len = i;
    return width;
}

void tui_text_truncate(const char *text, size_t len, int max_width,
                       const char *ellipsis, char *out, size_t out_size)
{
    if (!text || !out || out_size == 0) {
        if (out && out_size > 0) out[0] = '\0';
        return;
    }

    int full_width = tui_text_width(text, len);

    if (full_width <= max_width) {
        size_t copy = len < out_size - 1 ? len : out_size - 1;
        memcpy(out, text, copy);
        out[copy] = '\0';
        return;
    }

    const char *ell = ellipsis ? ellipsis : "~";
    int ell_width = tui_text_str_width(ell);
    size_t ell_len = strlen(ell);

    int target_width = max_width - ell_width;
    if (target_width < 0) target_width = 0;

    size_t clip_len;
    tui_text_clip(text, len, target_width, &clip_len);

    if (clip_len + ell_len >= out_size) {
        clip_len = out_size - ell_len - 1;
    }

    memcpy(out, text, clip_len);
    memcpy(out + clip_len, ell, ell_len);
    out[clip_len + ell_len] = '\0';
}

int tui_text_word_wrap(const char *text, size_t len, int max_width,
                       int *breaks, int max_breaks)
{
    if (!text || max_width <= 0 || !breaks || max_breaks <= 0) return 0;

    int break_count = 0;
    int line_start = 0;
    int line_width = 0;
    int last_space = -1;
    size_t i = 0;

    while (i < len && text[i]) {
        uint32_t cp;
        int clen = tui_utf8_next(text + i, len - i, &cp);
        if (clen == 0) break;

        if (cp == '\n') {
            if (break_count >= max_breaks) return break_count;
            breaks[break_count++] = (int)i;
            line_start = (int)i + 1;
            line_width = 0;
            last_space = -1;
            i += (size_t)clen;
            continue;
        }

        int cw;
        if (cp == '\t') {
            cw = TUI_TAB_SIZE - (line_width % TUI_TAB_SIZE);
        } else {
            cw = tui_utf8_char_width(cp);
        }

        if (cp == ' ') {
            last_space = (int)i;
        }

        if (line_width + cw > max_width) {
            if (break_count >= max_breaks) return break_count;

            if (last_space > line_start) {
                breaks[break_count++] = last_space;
                i = (size_t)last_space + 1;
                line_start = (int)i;
                line_width = 0;
                last_space = -1;
                continue;
            } else {
                if (line_width > 0) {
                    breaks[break_count++] = (int)i;
                    line_start = (int)i;
                    line_width = cw;
                    i += (size_t)clen;
                    last_space = -1;
                    continue;
                }
            }
        }

        line_width += cw;
        i += (size_t)clen;
    }

    return break_count;
}

int tui_text_index_to_col(const char *text, size_t len, size_t index)
{
    if (!text) return 0;
    if (index > len) index = len;

    int col = 0;
    size_t i = 0;

    while (i < index && i < len && text[i]) {
        uint32_t cp;
        int clen = tui_utf8_next(text + i, len - i, &cp);
        if (clen == 0) break;

        if (cp == '\t') {
            col += TUI_TAB_SIZE - (col % TUI_TAB_SIZE);
        } else if (cp == '\n' || cp == '\r') {
            col = 0;
        } else {
            col += tui_utf8_char_width(cp);
        }

        i += (size_t)clen;
    }

    return col;
}

int tui_text_col_to_index(const char *text, size_t len, int col)
{
    if (!text || col <= 0) return 0;

    int cur_col = 0;
    size_t i = 0;

    while (i < len && text[i]) {
        uint32_t cp;
        int clen = tui_utf8_next(text + i, len - i, &cp);
        if (clen == 0) break;

        if (cp == '\n' || cp == '\r') break;

        int cw;
        if (cp == '\t') {
            cw = TUI_TAB_SIZE - (cur_col % TUI_TAB_SIZE);
        } else {
            cw = tui_utf8_char_width(cp);
        }

        if (cur_col + cw > col) break;

        cur_col += cw;
        i += (size_t)clen;
    }

    return (int)i;
}

int tui_text_expand_tab(int col, int tab_size)
{
    if (tab_size <= 0) tab_size = TUI_TAB_SIZE;
    return tab_size - (col % tab_size);
}

int tui_text_expand_tabs(const char *text, size_t len, int tab_size,
                         char *out, size_t out_size)
{
    if (!text || !out || out_size == 0) return -1;

    if (tab_size <= 0) tab_size = TUI_TAB_SIZE;

    int col = 0;
    size_t oi = 0;
    size_t ti = 0;

    while (ti < len && text[ti]) {
        if (text[ti] == '\t') {
            int spaces = tab_size - (col % tab_size);
            for (int s = 0; s < spaces; s++) {
                if (oi + 1 >= out_size) goto done;
                out[oi++] = ' ';
            }
            col += spaces;
            ti++;
        } else if (text[ti] == '\n' || text[ti] == '\r') {
            if (oi + 1 >= out_size) goto done;
            out[oi++] = text[ti];
            col = 0;
            ti++;
        } else {
            uint32_t cp;
            int clen = tui_utf8_next(text + ti, len - ti, &cp);
            if (clen == 0) break;

            if (oi + (size_t)clen >= out_size) goto done;
            memcpy(out + oi, text + ti, (size_t)clen);
            oi += (size_t)clen;
            ti += (size_t)clen;
            col += tui_utf8_char_width(cp);
        }
    }

done:
    if (oi >= out_size) oi = out_size - 1;
    out[oi] = '\0';
    return (int)oi;
}
