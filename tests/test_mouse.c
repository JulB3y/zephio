#include "util.h"
#include "zephio_mouse.h"

/* ── mouse_action_name ──────────────────────────────────────────── */

TEST_BEGIN(mouse_action_press)
{
    TEST_STR_EQ(zephio_mouse_action_name(ZEPHIO_MOUSE_PRESS), "Press");
}

TEST_BEGIN(mouse_action_release)
{
    TEST_STR_EQ(zephio_mouse_action_name(ZEPHIO_MOUSE_RELEASE), "Release");
}

TEST_BEGIN(mouse_action_motion)
{
    TEST_STR_EQ(zephio_mouse_action_name(ZEPHIO_MOUSE_MOTION), "Motion");
}

TEST_BEGIN(mouse_action_wheel_up)
{
    TEST_STR_EQ(zephio_mouse_action_name(ZEPHIO_MOUSE_WHEEL_UP), "WheelUp");
}

TEST_BEGIN(mouse_action_wheel_down)
{
    TEST_STR_EQ(zephio_mouse_action_name(ZEPHIO_MOUSE_WHEEL_DOWN), "WheelDown");
}

TEST_BEGIN(mouse_action_unknown)
{
    TEST_STR_EQ(zephio_mouse_action_name((ZephioMouseAction)99), "Unknown");
}

/* ── mouse_button_name ──────────────────────────────────────────── */

TEST_BEGIN(mouse_button_none)
{
    TEST_STR_EQ(zephio_mouse_button_name(ZEPHIO_MOUSE_BTN_NONE), "None");
}

TEST_BEGIN(mouse_button_left)
{
    TEST_STR_EQ(zephio_mouse_button_name(ZEPHIO_MOUSE_BTN_LEFT), "Left");
}

TEST_BEGIN(mouse_button_middle)
{
    TEST_STR_EQ(zephio_mouse_button_name(ZEPHIO_MOUSE_BTN_MIDDLE), "Middle");
}

TEST_BEGIN(mouse_button_right)
{
    TEST_STR_EQ(zephio_mouse_button_name(ZEPHIO_MOUSE_BTN_RIGHT), "Right");
}

TEST_BEGIN(mouse_button_unknown)
{
    TEST_STR_EQ(zephio_mouse_button_name((ZephioMouseButton)99), "Unknown");
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running mouse tests...\n\n");

    TEST_RUN(mouse_action_press);
    TEST_RUN(mouse_action_release);
    TEST_RUN(mouse_action_motion);
    TEST_RUN(mouse_action_wheel_up);
    TEST_RUN(mouse_action_wheel_down);
    TEST_RUN(mouse_action_unknown);

    TEST_RUN(mouse_button_none);
    TEST_RUN(mouse_button_left);
    TEST_RUN(mouse_button_middle);
    TEST_RUN(mouse_button_right);
    TEST_RUN(mouse_button_unknown);

    TEST_SUMMARY();
}
