#include "util.h"
#include "tui_input.h"

/* ── key_name ───────────────────────────────────────────────────── */

TEST_BEGIN(key_name_special)
{
    TEST_STR_EQ(tui_key_name(TUI_KEY_UNKNOWN), "Unknown");
    TEST_STR_EQ(tui_key_name(TUI_KEY_ENTER), "Enter");
    TEST_STR_EQ(tui_key_name(TUI_KEY_TAB), "Tab");
    TEST_STR_EQ(tui_key_name(TUI_KEY_BACKSPACE), "Backspace");
    TEST_STR_EQ(tui_key_name(TUI_KEY_ESCAPE), "Escape");
}

TEST_BEGIN(key_name_arrows)
{
    TEST_STR_EQ(tui_key_name(TUI_KEY_UP), "Up");
    TEST_STR_EQ(tui_key_name(TUI_KEY_DOWN), "Down");
    TEST_STR_EQ(tui_key_name(TUI_KEY_LEFT), "Left");
    TEST_STR_EQ(tui_key_name(TUI_KEY_RIGHT), "Right");
}

TEST_BEGIN(key_name_navigation)
{
    TEST_STR_EQ(tui_key_name(TUI_KEY_HOME), "Home");
    TEST_STR_EQ(tui_key_name(TUI_KEY_END), "End");
    TEST_STR_EQ(tui_key_name(TUI_KEY_INSERT), "Insert");
    TEST_STR_EQ(tui_key_name(TUI_KEY_DELETE), "Delete");
    TEST_STR_EQ(tui_key_name(TUI_KEY_PAGE_UP), "PageUp");
    TEST_STR_EQ(tui_key_name(TUI_KEY_PAGE_DOWN), "PageDown");
}

TEST_BEGIN(key_name_function)
{
    TEST_STR_EQ(tui_key_name(TUI_KEY_F1), "F1");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F2), "F2");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F3), "F3");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F4), "F4");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F5), "F5");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F6), "F6");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F7), "F7");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F8), "F8");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F9), "F9");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F10), "F10");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F11), "F11");
    TEST_STR_EQ(tui_key_name(TUI_KEY_F12), "F12");
}

TEST_BEGIN(key_name_ctrl)
{
    TEST_STR_EQ(tui_key_name(TUI_KEY_CTRL_A), "Ctrl+A");
    TEST_STR_EQ(tui_key_name(TUI_KEY_CTRL_C), "Ctrl+C");
    TEST_STR_EQ(tui_key_name(TUI_KEY_CTRL_D), "Ctrl+D");
    TEST_STR_EQ(tui_key_name(TUI_KEY_CTRL_Z), "Ctrl+Z");
}

TEST_BEGIN(key_name_events)
{
    TEST_STR_EQ(tui_key_name(TUI_EVENT_RESIZE), "Resize");
    TEST_STR_EQ(tui_key_name(TUI_EVENT_MOUSE), "Mouse");
}

TEST_BEGIN(key_name_invalid)
{
    TEST_STR_EQ(tui_key_name((TuiKey)0xFFFF), "?");
}

/* ── modifier_name ──────────────────────────────────────────────── */

TEST_BEGIN(modifier_name_none)
{
    TEST_STR_EQ(tui_modifier_name(TUI_MOD_NONE), "");
}

TEST_BEGIN(modifier_name_single)
{
    TEST_STR_EQ(tui_modifier_name(TUI_MOD_CTRL), "Ctrl+");
    TEST_STR_EQ(tui_modifier_name(TUI_MOD_SHIFT), "Shift+");
    TEST_STR_EQ(tui_modifier_name(TUI_MOD_ALT), "Alt+");
}

TEST_BEGIN(modifier_name_combinations)
{
    TEST_STR_EQ(tui_modifier_name(TUI_MOD_CTRL | TUI_MOD_SHIFT), "Ctrl+Shift+");
    TEST_STR_EQ(tui_modifier_name(TUI_MOD_CTRL | TUI_MOD_ALT), "Ctrl+Alt+");
    TEST_STR_EQ(tui_modifier_name(TUI_MOD_ALT | TUI_MOD_SHIFT), "Alt+Shift+");
}

TEST_BEGIN(modifier_name_invalid)
{
    TEST_STR_EQ(tui_modifier_name(0xFF), "?+");
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running input tests...\n\n");

    TEST_RUN(key_name_special);
    TEST_RUN(key_name_arrows);
    TEST_RUN(key_name_navigation);
    TEST_RUN(key_name_function);
    TEST_RUN(key_name_ctrl);
    TEST_RUN(key_name_events);
    TEST_RUN(key_name_invalid);

    TEST_RUN(modifier_name_none);
    TEST_RUN(modifier_name_single);
    TEST_RUN(modifier_name_combinations);
    TEST_RUN(modifier_name_invalid);

    TEST_SUMMARY();
}
