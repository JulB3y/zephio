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

int tui_utf8_char_width(uint32_t cp)
{
    if (cp < 0x20) return 0;
    if (cp == 0x7F) return 0;
    if (cp >= 0x80 && cp <= 0x9F) return 0;
    if (cp == 0xAD) return 1;
    if (cp <= 0xFF) return 1;

    if (cp >= 0x0300 && cp <= 0x036F) return 0;
    if (cp >= 0x0483 && cp <= 0x0489) return 0;
    if (cp >= 0x0591 && cp <= 0x05BD) return 0;
    if (cp == 0x05BF) return 0;
    if (cp >= 0x05C1 && cp <= 0x05C2) return 0;
    if (cp >= 0x05C4 && cp <= 0x05C5) return 0;
    if (cp == 0x05C7) return 0;
    if (cp >= 0x0610 && cp <= 0x061A) return 0;
    if (cp >= 0x064B && cp <= 0x065F) return 0;
    if (cp == 0x0670) return 0;
    if (cp >= 0x06D6 && cp <= 0x06DC) return 0;
    if (cp >= 0x06DF && cp <= 0x06E4) return 0;
    if (cp >= 0x06E7 && cp <= 0x06E8) return 0;
    if (cp >= 0x06EA && cp <= 0x06ED) return 0;
    if (cp >= 0x0711 && cp <= 0x0711) return 0;
    if (cp >= 0x0730 && cp <= 0x074A) return 0;
    if (cp >= 0x07A6 && cp <= 0x07B0) return 0;
    if (cp >= 0x07EB && cp <= 0x07F3) return 0;
    if (cp >= 0x07FD && cp <= 0x07FD) return 0;
    if (cp >= 0x0816 && cp <= 0x0819) return 0;
    if (cp >= 0x081B && cp <= 0x0823) return 0;
    if (cp >= 0x0825 && cp <= 0x0827) return 0;
    if (cp >= 0x0829 && cp <= 0x082D) return 0;
    if (cp >= 0x0859 && cp <= 0x085B) return 0;
    if (cp >= 0x0898 && cp <= 0x089F) return 0;
    if (cp >= 0x08CA && cp <= 0x08E1) return 0;
    if (cp >= 0x08E3 && cp <= 0x0902) return 0;
    if (cp == 0x093A) return 0;
    if (cp == 0x093C) return 0;
    if (cp >= 0x0941 && cp <= 0x0948) return 0;
    if (cp >= 0x094D && cp <= 0x094D) return 0;
    if (cp >= 0x0951 && cp <= 0x0957) return 0;
    if (cp >= 0x0962 && cp <= 0x0963) return 0;
    if (cp == 0x0981) return 0;
    if (cp == 0x09BC) return 0;
    if (cp == 0x09BE) return 0;
    if (cp >= 0x09C1 && cp <= 0x09C4) return 0;
    if (cp == 0x09CD) return 0;
    if (cp == 0x09D7) return 0;
    if (cp >= 0x09E2 && cp <= 0x09E3) return 0;
    if (cp >= 0x09FE && cp <= 0x09FE) return 0;
    if (cp >= 0x0A01 && cp <= 0x0A02) return 0;
    if (cp == 0x0A3C) return 0;
    if (cp >= 0x0A41 && cp <= 0x0A42) return 0;
    if (cp >= 0x0A47 && cp <= 0x0A48) return 0;
    if (cp >= 0x0A4B && cp <= 0x0A4D) return 0;
    if (cp == 0x0A51) return 0;
    if (cp >= 0x0A70 && cp <= 0x0A71) return 0;
    if (cp == 0x0A75) return 0;
    if (cp >= 0x0A81 && cp <= 0x0A82) return 0;
    if (cp == 0x0ABC) return 0;
    if (cp >= 0x0AC1 && cp <= 0x0AC5) return 0;
    if (cp >= 0x0AC7 && cp <= 0x0AC8) return 0;
    if (cp == 0x0ACD) return 0;
    if (cp >= 0x0AE2 && cp <= 0x0AE3) return 0;
    if (cp == 0x0AFA && cp <= 0x0AFF) return 0;
    if (cp >= 0x0B01 && cp <= 0x0B01) return 0;
    if (cp == 0x0B3C) return 0;
    if (cp >= 0x0B3E && cp <= 0x0B3E) return 0;
    if (cp >= 0x0B3F && cp <= 0x0B3F) return 0;
    if (cp >= 0x0B41 && cp <= 0x0B44) return 0;
    if (cp >= 0x0B4D && cp <= 0x0B4D) return 0;
    if (cp == 0x0B55) return 0;
    if (cp >= 0x0B57 && cp <= 0x0B57) return 0;
    if (cp >= 0x0B62 && cp <= 0x0B63) return 0;
    if (cp == 0x0B82) return 0;
    if (cp == 0x0BBE) return 0;
    if (cp == 0x0BC0) return 0;
    if (cp == 0x0BCD) return 0;
    if (cp == 0x0BD7) return 0;
    if (cp >= 0x0C00 && cp <= 0x0C00) return 0;
    if (cp >= 0x0C04 && cp <= 0x0C04) return 0;
    if (cp == 0x0C3C) return 0;
    if (cp >= 0x0C3E && cp <= 0x0C40) return 0;
    if (cp >= 0x0C46 && cp <= 0x0C48) return 0;
    if (cp >= 0x0C4A && cp <= 0x0C4D) return 0;
    if (cp >= 0x0C55 && cp <= 0x0C56) return 0;
    if (cp >= 0x0C62 && cp <= 0x0C63) return 0;
    if (cp == 0x0C81) return 0;
    if (cp == 0x0CBC) return 0;
    if (cp == 0x0CBF) return 0;
    if (cp == 0x0CC2) return 0;
    if (cp >= 0x0CC6 && cp <= 0x0CC6) return 0;
    if (cp >= 0x0CCC && cp <= 0x0CCD) return 0;
    if (cp >= 0x0CD5 && cp <= 0x0CD6) return 0;
    if (cp >= 0x0CE2 && cp <= 0x0CE3) return 0;
    if (cp >= 0x0D00 && cp <= 0x0D01) return 0;
    if (cp >= 0x0D3B && cp <= 0x0D3C) return 0;
    if (cp == 0x0D3E) return 0;
    if (cp >= 0x0D41 && cp <= 0x0D44) return 0;
    if (cp == 0x0D4D) return 0;
    if (cp == 0x0D57) return 0;
    if (cp >= 0x0D62 && cp <= 0x0D63) return 0;
    if (cp == 0x0D81) return 0;
    if (cp == 0x0DCA) return 0;
    if (cp >= 0x0DCF && cp <= 0x0DCF) return 0;
    if (cp >= 0x0DD2 && cp <= 0x0DD4) return 0;
    if (cp == 0x0DD6) return 0;
    if (cp >= 0x0DDF && cp <= 0x0DDF) return 0;
    if (cp >= 0x0E31 && cp <= 0x0E31) return 0;
    if (cp >= 0x0E34 && cp <= 0x0E3A) return 0;
    if (cp >= 0x0E47 && cp <= 0x0E4E) return 0;
    if (cp >= 0x0EB1 && cp <= 0x0EB1) return 0;
    if (cp >= 0x0EB4 && cp <= 0x0EBC) return 0;
    if (cp >= 0x0EC8 && cp <= 0x0ECE) return 0;
    if (cp >= 0x0F18 && cp <= 0x0F19) return 0;
    if (cp == 0x0F35) return 0;
    if (cp == 0x0F37) return 0;
    if (cp == 0x0F39) return 0;
    if (cp >= 0x0F71 && cp <= 0x0F7E) return 0;
    if (cp >= 0x0F80 && cp <= 0x0F84) return 0;
    if (cp >= 0x0F86 && cp <= 0x0F87) return 0;
    if (cp >= 0x0F8D && cp <= 0x0F97) return 0;
    if (cp >= 0x0F99 && cp <= 0x0FBC) return 0;
    if (cp == 0x0FC6) return 0;
    if (cp >= 0x102D && cp <= 0x1030) return 0;
    if (cp >= 0x1032 && cp <= 0x1037) return 0;
    if (cp >= 0x1039 && cp <= 0x103A) return 0;
    if (cp >= 0x103D && cp <= 0x103E) return 0;
    if (cp >= 0x1058 && cp <= 0x1059) return 0;
    if (cp >= 0x105E && cp <= 0x1060) return 0;
    if (cp >= 0x1071 && cp <= 0x1074) return 0;
    if (cp == 0x1082) return 0;
    if (cp >= 0x1085 && cp <= 0x1086) return 0;
    if (cp == 0x108D) return 0;
    if (cp >= 0x109D && cp <= 0x109D) return 0;
    if (cp >= 0x135D && cp <= 0x135F) return 0;
    if (cp >= 0x1712 && cp <= 0x1714) return 0;
    if (cp >= 0x1732 && cp <= 0x1733) return 0;
    if (cp >= 0x1752 && cp <= 0x1753) return 0;
    if (cp >= 0x1772 && cp <= 0x1773) return 0;
    if (cp >= 0x17B4 && cp <= 0x17B5) return 0;
    if (cp >= 0x17B7 && cp <= 0x17BD) return 0;
    if (cp >= 0x17C6 && cp <= 0x17C6) return 0;
    if (cp >= 0x17C9 && cp <= 0x17D3) return 0;
    if (cp == 0x17DD) return 0;
    if (cp >= 0x180B && cp <= 0x180D) return 0;
    if (cp == 0x180F) return 0;
    if (cp >= 0x1885 && cp <= 0x1886) return 0;
    if (cp >= 0x18A9 && cp <= 0x18A9) return 0;
    if (cp >= 0x1920 && cp <= 0x1922) return 0;
    if (cp >= 0x1927 && cp <= 0x1928) return 0;
    if (cp == 0x1932) return 0;
    if (cp >= 0x1939 && cp <= 0x193B) return 0;
    if (cp >= 0x1A17 && cp <= 0x1A18) return 0;
    if (cp >= 0x1A1B && cp <= 0x1A1B) return 0;
    if (cp >= 0x1A56 && cp <= 0x1A56) return 0;
    if (cp >= 0x1A58 && cp <= 0x1A5E) return 0;
    if (cp >= 0x1A60 && cp <= 0x1A60) return 0;
    if (cp >= 0x1A62 && cp <= 0x1A62) return 0;
    if (cp >= 0x1A65 && cp <= 0x1A6C) return 0;
    if (cp >= 0x1A73 && cp <= 0x1A7C) return 0;
    if (cp == 0x1A7F) return 0;
    if (cp >= 0x1AB0 && cp <= 0x1ACE) return 0;
    if (cp >= 0x1B00 && cp <= 0x1B03) return 0;
    if (cp == 0x1B34) return 0;
    if (cp >= 0x1B35 && cp <= 0x1B35) return 0;
    if (cp >= 0x1B36 && cp <= 0x1B3A) return 0;
    if (cp == 0x1B3C) return 0;
    if (cp >= 0x1B42 && cp <= 0x1B42) return 0;
    if (cp >= 0x1B6B && cp <= 0x1B73) return 0;
    if (cp >= 0x1B80 && cp <= 0x1B81) return 0;
    if (cp == 0x1BA2) return 0;
    if (cp >= 0x1BA5 && cp <= 0x1BA5) return 0;
    if (cp >= 0x1BA8 && cp <= 0x1BA9) return 0;
    if (cp >= 0x1BAB && cp <= 0x1BAD) return 0;
    if (cp == 0x1BE6) return 0;
    if (cp >= 0x1BE8 && cp <= 0x1BE9) return 0;
    if (cp == 0x1BED) return 0;
    if (cp >= 0x1BEF && cp <= 0x1BF1) return 0;
    if (cp >= 0x1C2C && cp <= 0x1C33) return 0;
    if (cp >= 0x1C36 && cp <= 0x1C37) return 0;
    if (cp >= 0x1CD0 && cp <= 0x1CD2) return 0;
    if (cp >= 0x1CD4 && cp <= 0x1CE0) return 0;
    if (cp >= 0x1CE2 && cp <= 0x1CE8) return 0;
    if (cp == 0x1CED) return 0;
    if (cp == 0x1CF4) return 0;
    if (cp >= 0x1CF8 && cp <= 0x1CF9) return 0;
    if (cp >= 0x1DC0 && cp <= 0x1DFF) return 0;

    if (cp >= 0x200B && cp <= 0x200F) return 0;
    if (cp >= 0x2028 && cp <= 0x202E) return 0;
    if (cp == 0x2060) return 0;
    if (cp >= 0x2066 && cp <= 0x206F) return 0;

    if (cp >= 0x20D0 && cp <= 0x20F0) return 0;
    if (cp >= 0x2CEF && cp <= 0x2CF1) return 0;
    if (cp >= 0x2DE0 && cp <= 0x2DFF) return 0;
    if (cp >= 0xA66F && cp <= 0xA672) return 0;
    if (cp >= 0xA674 && cp <= 0xA67D) return 0;
    if (cp >= 0xA69E && cp <= 0xA69F) return 0;
    if (cp >= 0xA6F0 && cp <= 0xA6F1) return 0;
    if (cp >= 0xA802 && cp <= 0xA802) return 0;
    if (cp == 0xA806) return 0;
    if (cp == 0xA80B) return 0;
    if (cp >= 0xA825 && cp <= 0xA826) return 0;
    if (cp >= 0xA82C && cp <= 0xA82C) return 0;
    if (cp >= 0xA8C4 && cp <= 0xA8C5) return 0;
    if (cp >= 0xA8E0 && cp <= 0xA8F1) return 0;
    if (cp >= 0xA8FF && cp <= 0xA8FF) return 0;
    if (cp >= 0xA926 && cp <= 0xA92D) return 0;
    if (cp >= 0xA947 && cp <= 0xA951) return 0;
    if (cp >= 0xA980 && cp <= 0xA982) return 0;
    if (cp == 0xA9B3) return 0;
    if (cp >= 0xA9B6 && cp <= 0xA9B9) return 0;
    if (cp == 0xA9BC) return 0;
    if (cp == 0xA9BD) return 0;
    if (cp >= 0xA9E5 && cp <= 0xA9E5) return 0;
    if (cp >= 0xAA29 && cp <= 0xAA2E) return 0;
    if (cp >= 0xAA31 && cp <= 0xAA32) return 0;
    if (cp >= 0xAA35 && cp <= 0xAA36) return 0;
    if (cp == 0xAA43) return 0;
    if (cp == 0xAA4C) return 0;
    if (cp == 0xAA7C) return 0;
    if (cp == 0xAAB0) return 0;
    if (cp >= 0xAAB2 && cp <= 0xAAB4) return 0;
    if (cp >= 0xAAB7 && cp <= 0xAAB8) return 0;
    if (cp >= 0xAABE && cp <= 0xAABF) return 0;
    if (cp == 0xAAC1) return 0;
    if (cp >= 0xAAEC && cp <= 0xAAED) return 0;
    if (cp == 0xAAF6) return 0;
    if (cp == 0xABE5) return 0;
    if (cp == 0xABE8) return 0;
    if (cp == 0xABED) return 0;
    if (cp == 0xFB1E) return 0;
    if (cp >= 0xFE00 && cp <= 0xFE0F) return 0;
    if (cp >= 0xFE20 && cp <= 0xFE2F) return 0;
    if (cp == 0xFEFF) return 0;
    if (cp >= 0xFFF9 && cp <= 0xFFFB) return 0;

    if (cp >= 0x101FD && cp <= 0x101FD) return 0;
    if (cp >= 0x102E0 && cp <= 0x102E0) return 0;
    if (cp >= 0x10376 && cp <= 0x1037A) return 0;
    if (cp >= 0x10A01 && cp <= 0x10A03) return 0;
    if (cp >= 0x10A05 && cp <= 0x10A06) return 0;
    if (cp >= 0x10A0C && cp <= 0x10A0F) return 0;
    if (cp >= 0x10A38 && cp <= 0x10A3A) return 0;
    if (cp == 0x10A3F) return 0;
    if (cp >= 0x10AE5 && cp <= 0x10AE6) return 0;
    if (cp >= 0x10D24 && cp <= 0x10D27) return 0;
    if (cp >= 0x10EAB && cp <= 0x10EAC) return 0;
    if (cp >= 0x10EFD && cp <= 0x10EFF) return 0;
    if (cp >= 0x10F46 && cp <= 0x10F50) return 0;
    if (cp >= 0x10F82 && cp <= 0x10F85) return 0;
    if (cp >= 0x11001 && cp <= 0x11001) return 0;
    if (cp >= 0x11038 && cp <= 0x11046) return 0;
    if (cp >= 0x11070 && cp <= 0x11070) return 0;
    if (cp >= 0x11073 && cp <= 0x11074) return 0;
    if (cp >= 0x1107F && cp <= 0x11081) return 0;
    if (cp >= 0x110B3 && cp <= 0x110B6) return 0;
    if (cp >= 0x110B9 && cp <= 0x110BA) return 0;
    if (cp >= 0x110C2 && cp <= 0x110C2) return 0;
    if (cp >= 0x11100 && cp <= 0x11102) return 0;
    if (cp >= 0x11127 && cp <= 0x1112B) return 0;
    if (cp >= 0x1112D && cp <= 0x11134) return 0;
    if (cp == 0x11173) return 0;
    if (cp >= 0x11180 && cp <= 0x11181) return 0;
    if (cp >= 0x111B6 && cp <= 0x111BE) return 0;
    if (cp >= 0x111C9 && cp <= 0x111CC) return 0;
    if (cp == 0x111CF) return 0;
    if (cp >= 0x1122F && cp <= 0x11231) return 0;
    if (cp == 0x11234) return 0;
    if (cp >= 0x11236 && cp <= 0x11237) return 0;
    if (cp == 0x1123E) return 0;
    if (cp == 0x11241) return 0;
    if (cp >= 0x112DF && cp <= 0x112DF) return 0;
    if (cp >= 0x112E3 && cp <= 0x112EA) return 0;
    if (cp >= 0x11300 && cp <= 0x11301) return 0;
    if (cp >= 0x1133B && cp <= 0x1133C) return 0;
    if (cp == 0x1133E) return 0;
    if (cp >= 0x11340 && cp <= 0x11340) return 0;
    if (cp >= 0x11357 && cp <= 0x11357) return 0;
    if (cp >= 0x11366 && cp <= 0x1136C) return 0;
    if (cp >= 0x11370 && cp <= 0x11374) return 0;
    if (cp >= 0x11438 && cp <= 0x1143F) return 0;
    if (cp >= 0x11442 && cp <= 0x11444) return 0;
    if (cp == 0x11446) return 0;
    if (cp >= 0x1145E && cp <= 0x1145E) return 0;
    if (cp == 0x114B0) return 0;
    if (cp >= 0x114B3 && cp <= 0x114B8) return 0;
    if (cp == 0x114BA) return 0;
    if (cp >= 0x114BD && cp <= 0x114BD) return 0;
    if (cp >= 0x114BF && cp <= 0x114C0) return 0;
    if (cp >= 0x114C2 && cp <= 0x114C3) return 0;
    if (cp >= 0x115AF && cp <= 0x115AF) return 0;
    if (cp >= 0x115B2 && cp <= 0x115B5) return 0;
    if (cp >= 0x115BC && cp <= 0x115BD) return 0;
    if (cp >= 0x115BF && cp <= 0x115C0) return 0;
    if (cp >= 0x115DC && cp <= 0x115DD) return 0;
    if (cp >= 0x11633 && cp <= 0x1163A) return 0;
    if (cp == 0x1163D) return 0;
    if (cp >= 0x1163F && cp <= 0x11640) return 0;
    if (cp == 0x116AB) return 0;
    if (cp >= 0x116AD && cp <= 0x116AD) return 0;
    if (cp >= 0x116B0 && cp <= 0x116B5) return 0;
    if (cp == 0x116B7) return 0;
    if (cp >= 0x1171D && cp <= 0x1171F) return 0;
    if (cp >= 0x11722 && cp <= 0x11725) return 0;
    if (cp >= 0x11727 && cp <= 0x1172B) return 0;
    if (cp >= 0x1182F && cp <= 0x11837) return 0;
    if (cp >= 0x11839 && cp <= 0x1183A) return 0;
    if (cp >= 0x11930 && cp <= 0x11930) return 0;
    if (cp >= 0x1193B && cp <= 0x1193C) return 0;
    if (cp >= 0x1193E && cp <= 0x1193E) return 0;
    if (cp >= 0x11943 && cp <= 0x11943) return 0;
    if (cp >= 0x119D4 && cp <= 0x119D7) return 0;
    if (cp >= 0x119DA && cp <= 0x119DB) return 0;
    if (cp == 0x119E0) return 0;
    if (cp >= 0x11A01 && cp <= 0x11A0A) return 0;
    if (cp >= 0x11A33 && cp <= 0x11A38) return 0;
    if (cp >= 0x11A3B && cp <= 0x11A3E) return 0;
    if (cp == 0x11A47) return 0;
    if (cp >= 0x11A51 && cp <= 0x11A56) return 0;
    if (cp >= 0x11A59 && cp <= 0x11A5B) return 0;
    if (cp >= 0x11A8A && cp <= 0x11A96) return 0;
    if (cp >= 0x11A98 && cp <= 0x11A99) return 0;
    if (cp >= 0x11C30 && cp <= 0x11C36) return 0;
    if (cp >= 0x11C38 && cp <= 0x11C3D) return 0;
    if (cp == 0x11C3F) return 0;
    if (cp >= 0x11C92 && cp <= 0x11CA7) return 0;
    if (cp >= 0x11CAA && cp <= 0x11CB0) return 0;
    if (cp >= 0x11CB2 && cp <= 0x11CB3) return 0;
    if (cp >= 0x11CB5 && cp <= 0x11CB6) return 0;
    if (cp >= 0x11D31 && cp <= 0x11D36) return 0;
    if (cp == 0x11D3A) return 0;
    if (cp >= 0x11D3C && cp <= 0x11D3D) return 0;
    if (cp >= 0x11D3F && cp <= 0x11D45) return 0;
    if (cp == 0x11D47) return 0;
    if (cp >= 0x11D90 && cp <= 0x11D91) return 0;
    if (cp == 0x11D95) return 0;
    if (cp == 0x11D97) return 0;
    if (cp >= 0x11EF3 && cp <= 0x11EF4) return 0;
    if (cp >= 0x11F00 && cp <= 0x11F01) return 0;
    if (cp >= 0x11F36 && cp <= 0x11F3A) return 0;
    if (cp >= 0x11F40 && cp <= 0x11F40) return 0;
    if (cp >= 0x11F42 && cp <= 0x11F42) return 0;
    if (cp >= 0x13440 && cp <= 0x13440) return 0;
    if (cp >= 0x13447 && cp <= 0x13455) return 0;
    if (cp >= 0x16AF0 && cp <= 0x16AF4) return 0;
    if (cp >= 0x16B30 && cp <= 0x16B36) return 0;
    if (cp >= 0x16F4F && cp <= 0x16F4F) return 0;
    if (cp >= 0x16F8F && cp <= 0x16F92) return 0;
    if (cp >= 0x16FE4 && cp <= 0x16FE4) return 0;
    if (cp >= 0x1BC9D && cp <= 0x1BC9E) return 0;
    if (cp >= 0x1CF00 && cp <= 0x1CF2D) return 0;
    if (cp >= 0x1CF30 && cp <= 0x1CF46) return 0;
    if (cp == 0x1D165) return 0;
    if (cp >= 0x1D167 && cp <= 0x1D169) return 0;
    if (cp >= 0x1D16E && cp <= 0x1D172) return 0;
    if (cp >= 0x1D17B && cp <= 0x1D182) return 0;
    if (cp >= 0x1D185 && cp <= 0x1D18B) return 0;
    if (cp >= 0x1D1AA && cp <= 0x1D1AD) return 0;
    if (cp >= 0x1D242 && cp <= 0x1D244) return 0;
    if (cp >= 0x1DA00 && cp <= 0x1DA36) return 0;
    if (cp >= 0x1DA3B && cp <= 0x1DA6C) return 0;
    if (cp == 0x1DA75) return 0;
    if (cp == 0x1DA84) return 0;
    if (cp >= 0x1DA9B && cp <= 0x1DA9F) return 0;
    if (cp >= 0x1DAA1 && cp <= 0x1DAAF) return 0;
    if (cp >= 0x1E000 && cp <= 0x1E006) return 0;
    if (cp >= 0x1E008 && cp <= 0x1E018) return 0;
    if (cp >= 0x1E01B && cp <= 0x1E021) return 0;
    if (cp >= 0x1E023 && cp <= 0x1E024) return 0;
    if (cp >= 0x1E026 && cp <= 0x1E02A) return 0;
    if (cp >= 0x1E08F && cp <= 0x1E08F) return 0;
    if (cp >= 0x1E130 && cp <= 0x1E136) return 0;
    if (cp >= 0x1E2AE && cp <= 0x1E2AE) return 0;
    if (cp >= 0x1E2EC && cp <= 0x1E2EF) return 0;
    if (cp >= 0x1E4EC && cp <= 0x1E4EF) return 0;
    if (cp >= 0x1E8D0 && cp <= 0x1E8D6) return 0;
    if (cp >= 0x1E944 && cp <= 0x1E94A) return 0;
    if (cp >= 0xE0020 && cp <= 0xE007F) return 0;
    if (cp >= 0xE0100 && cp <= 0xE01EF) return 0;

    if (cp >= 0x1100 && cp <= 0x115F) return 2;
    if (cp >= 0x231A && cp <= 0x231B) return 2;
    if (cp == 0x2329) return 2;
    if (cp == 0x232A) return 2;
    if (cp >= 0x23E9 && cp <= 0x23EC) return 2;
    if (cp == 0x23F0) return 2;
    if (cp == 0x23F3) return 2;
    if (cp >= 0x25FD && cp <= 0x25FE) return 2;
    if (cp >= 0x2614 && cp <= 0x2615) return 2;
    if (cp >= 0x2648 && cp <= 0x2653) return 2;
    if (cp == 0x267F) return 2;
    if (cp >= 0x2693 && cp <= 0x2693) return 2;
    if (cp >= 0x26A1 && cp <= 0x26A1) return 2;
    if (cp >= 0x26AA && cp <= 0x26AB) return 2;
    if (cp >= 0x26BD && cp <= 0x26BE) return 2;
    if (cp >= 0x26C4 && cp <= 0x26C5) return 2;
    if (cp == 0x26CE) return 2;
    if (cp == 0x26D4) return 2;
    if (cp == 0x26EA) return 2;
    if (cp >= 0x26F2 && cp <= 0x26F3) return 2;
    if (cp == 0x26F5) return 2;
    if (cp == 0x26FA) return 2;
    if (cp == 0x26FD) return 2;
    if (cp >= 0x2702 && cp <= 0x2702) return 2;
    if (cp == 0x2705) return 2;
    if (cp >= 0x2708 && cp <= 0x270D) return 2;
    if (cp == 0x270F) return 2;
    if (cp == 0x2712) return 2;
    if (cp == 0x2714) return 2;
    if (cp == 0x2716) return 2;
    if (cp == 0x271D) return 2;
    if (cp == 0x2721) return 2;
    if (cp == 0x2728) return 2;
    if (cp >= 0x2733 && cp <= 0x2734) return 2;
    if (cp == 0x2744) return 2;
    if (cp == 0x2747) return 2;
    if (cp == 0x274C) return 2;
    if (cp == 0x274E) return 2;
    if (cp >= 0x2753 && cp <= 0x2755) return 2;
    if (cp == 0x2757) return 2;
    if (cp >= 0x2763 && cp <= 0x2764) return 2;
    if (cp >= 0x2795 && cp <= 0x2797) return 2;
    if (cp == 0x27A1) return 2;
    if (cp == 0x27B0) return 2;
    if (cp == 0x27BF) return 2;
    if (cp >= 0x2934 && cp <= 0x2935) return 2;
    if (cp >= 0x2B05 && cp <= 0x2B07) return 2;
    if (cp >= 0x2B1B && cp <= 0x2B1C) return 2;
    if (cp == 0x2B50) return 2;
    if (cp == 0x2B55) return 2;
    if (cp >= 0x2B1B && cp <= 0x2B1C) return 2;
    if (cp >= 0x2E80 && cp <= 0x2E99) return 2;
    if (cp >= 0x2E9B && cp <= 0x2EF3) return 2;
    if (cp >= 0x2F00 && cp <= 0x2FD5) return 2;
    if (cp >= 0x2FF0 && cp <= 0x2FFB) return 2;
    if (cp >= 0x3000 && cp <= 0x303E) return 2;
    if (cp >= 0x303F && cp <= 0x3096) return 2;
    if (cp >= 0x3099 && cp <= 0x30FF) return 2;
    if (cp >= 0x3105 && cp <= 0x312F) return 2;
    if (cp >= 0x3131 && cp <= 0x318E) return 2;
    if (cp >= 0x3190 && cp <= 0x31E3) return 2;
    if (cp >= 0x31F0 && cp <= 0x321E) return 2;
    if (cp >= 0x3220 && cp <= 0x3247) return 2;
    if (cp >= 0x3250 && cp <= 0x4DBF) return 2;
    if (cp >= 0x4E00 && cp <= 0x9FFF) return 2;
    if (cp >= 0xA000 && cp <= 0xA014) return 2;
    if (cp == 0xA015) return 2;
    if (cp >= 0xA016 && cp <= 0xA48C) return 2;
    if (cp >= 0xA490 && cp <= 0xA4C6) return 2;
    if (cp >= 0xA960 && cp <= 0xA97C) return 2;
    if (cp >= 0xAC00 && cp <= 0xD7A3) return 2;
    if (cp >= 0xD7B0 && cp <= 0xD7C6) return 2;
    if (cp >= 0xD7CB && cp <= 0xD7FB) return 2;
    if (cp >= 0xF900 && cp <= 0xFAFF) return 2;
    if (cp >= 0xFE10 && cp <= 0xFE19) return 2;
    if (cp >= 0xFE30 && cp <= 0xFE52) return 2;
    if (cp >= 0xFE54 && cp <= 0xFE66) return 2;
    if (cp >= 0xFE68 && cp <= 0xFE6B) return 2;
    if (cp >= 0xFF01 && cp <= 0xFF60) return 2;
    if (cp >= 0xFFE0 && cp <= 0xFFE6) return 2;
    if (cp >= 0x16FE0 && cp <= 0x16FE1) return 2;
    if (cp == 0x16FE3) return 2;
    if (cp >= 0x17000 && cp <= 0x187F7) return 2;
    if (cp >= 0x18800 && cp <= 0x18CD5) return 2;
    if (cp >= 0x18D00 && cp <= 0x18D08) return 2;
    if (cp >= 0x1B000 && cp <= 0x1B11E) return 2;
    if (cp >= 0x1B150 && cp <= 0x1B152) return 2;
    if (cp >= 0x1B164 && cp <= 0x1B167) return 2;
    if (cp >= 0x1B170 && cp <= 0x1B2FB) return 2;
    if (cp >= 0x1F004 && cp <= 0x1F004) return 2;
    if (cp >= 0x1F0CF && cp <= 0x1F0CF) return 2;
    if (cp >= 0x1F18E && cp <= 0x1F18E) return 2;
    if (cp >= 0x1F191 && cp <= 0x1F19A) return 2;
    if (cp >= 0x1F1AD && cp <= 0x1F1E5) return 2;
    if (cp >= 0x1F201 && cp <= 0x1F20F) return 2;
    if (cp == 0x1F21A) return 2;
    if (cp == 0x1F22F) return 2;
    if (cp >= 0x1F232 && cp <= 0x1F23A) return 2;
    if (cp >= 0x1F23C && cp <= 0x1F23F) return 2;
    if (cp >= 0x1F249 && cp <= 0x1F24F) return 2;
    if (cp >= 0x1F250 && cp <= 0x1F251) return 2;
    if (cp >= 0x1F252 && cp <= 0x1F2FF) return 2;
    if (cp >= 0x1F300 && cp <= 0x1F321) return 2;
    if (cp >= 0x1F324 && cp <= 0x1F393) return 2;
    if (cp >= 0x1F396 && cp <= 0x1F397) return 2;
    if (cp >= 0x1F399 && cp <= 0x1F39B) return 2;
    if (cp >= 0x1F39E && cp <= 0x1F3F0) return 2;
    if (cp >= 0x1F3F3 && cp <= 0x1F3F5) return 2;
    if (cp >= 0x1F3F7 && cp <= 0x1F4FD) return 2;
    if (cp == 0x1F4FF) return 2;
    if (cp >= 0x1F500 && cp <= 0x1F53D) return 2;
    if (cp >= 0x1F549 && cp <= 0x1F54E) return 2;
    if (cp >= 0x1F550 && cp <= 0x1F567) return 2;
    if (cp >= 0x1F56F && cp <= 0x1F570) return 2;
    if (cp >= 0x1F573 && cp <= 0x1F57A) return 2;
    if (cp == 0x1F587) return 2;
    if (cp >= 0x1F58A && cp <= 0x1F58D) return 2;
    if (cp == 0x1F590) return 2;
    if (cp >= 0x1F595 && cp <= 0x1F596) return 2;
    if (cp >= 0x1F5A4 && cp <= 0x1F5A5) return 2;
    if (cp == 0x1F5A8) return 2;
    if (cp >= 0x1F5B1 && cp <= 0x1F5B2) return 2;
    if (cp == 0x1F5BC) return 2;
    if (cp >= 0x1F5C2 && cp <= 0x1F5C4) return 2;
    if (cp >= 0x1F5D1 && cp <= 0x1F5D3) return 2;
    if (cp >= 0x1F5DC && cp <= 0x1F5DE) return 2;
    if (cp == 0x1F5E1) return 2;
    if (cp >= 0x1F5E3 && cp <= 0x1F5E3) return 2;
    if (cp >= 0x1F5E8 && cp <= 0x1F5E8) return 2;
    if (cp >= 0x1F5EF && cp <= 0x1F5EF) return 2;
    if (cp == 0x1F5F3) return 2;
    if (cp >= 0x1F5FA && cp <= 0x1F64F) return 2;
    if (cp >= 0x1F680 && cp <= 0x1F6C5) return 2;
    if (cp >= 0x1F6CB && cp <= 0x1F6D2) return 2;
    if (cp >= 0x1F6D5 && cp <= 0x1F6D7) return 2;
    if (cp >= 0x1F6DC && cp <= 0x1F6E5) return 2;
    if (cp == 0x1F6E9) return 2;
    if (cp >= 0x1F6EB && cp <= 0x1F6EC) return 2;
    if (cp == 0x1F6F0) return 2;
    if (cp >= 0x1F6F3 && cp <= 0x1F6FC) return 2;
    if (cp >= 0x1F7E0 && cp <= 0x1F7EB) return 2;
    if (cp >= 0x1F7F0 && cp <= 0x1F7F0) return 2;
    if (cp >= 0x1F90C && cp <= 0x1F93A) return 2;
    if (cp >= 0x1F93C && cp <= 0x1F945) return 2;
    if (cp >= 0x1F947 && cp <= 0x1F9FF) return 2;
    if (cp >= 0x1FA00 && cp <= 0x1FA53) return 2;
    if (cp >= 0x1FA60 && cp <= 0x1FA6D) return 2;
    if (cp >= 0x1FA70 && cp <= 0x1FA7C) return 2;
    if (cp >= 0x1FA80 && cp <= 0x1FA88) return 2;
    if (cp >= 0x1FA90 && cp <= 0x1FABD) return 2;
    if (cp >= 0x1FABF && cp <= 0x1FAC5) return 2;
    if (cp >= 0x1FACE && cp <= 0x1FADB) return 2;
    if (cp >= 0x1FAE0 && cp <= 0x1FAE8) return 2;
    if (cp >= 0x1FAF0 && cp <= 0x1FAF8) return 2;
    if (cp >= 0x20000 && cp <= 0x2FFFD) return 2;
    if (cp >= 0x30000 && cp <= 0x3FFFD) return 2;

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
            width += ZEPHIO_TAB_SIZE - (width % ZEPHIO_TAB_SIZE);
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
            cw = ZEPHIO_TAB_SIZE - (width % ZEPHIO_TAB_SIZE);
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
            cw = ZEPHIO_TAB_SIZE - (line_width % ZEPHIO_TAB_SIZE);
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
            col += ZEPHIO_TAB_SIZE - (col % ZEPHIO_TAB_SIZE);
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
            cw = ZEPHIO_TAB_SIZE - (cur_col % ZEPHIO_TAB_SIZE);
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
    if (tab_size <= 0) tab_size = ZEPHIO_TAB_SIZE;
    return tab_size - (col % tab_size);
}

int tui_text_expand_tabs(const char *text, size_t len, int tab_size,
                         char *out, size_t out_size)
{
    if (!text || !out || out_size == 0) return -1;

    if (tab_size <= 0) tab_size = ZEPHIO_TAB_SIZE;

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
