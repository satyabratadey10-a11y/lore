#include <iostream>
#include <cassert>
#include <string>

#include "lore_runtime.h"
#include "lore_renderer.h"

using namespace lore;

void setup_counter_program() {
    set_package("com.axilo.counter", "counter1", "Counter");
    set_perm_input_harness(true);
    set_perm_eachtimevalue(1);
    set_perm_show_display(true);

    register_color("blue", "#4287f5");
    register_color("red", "#e30707");
    register_color("white", "#ffffff");

    create_root(ElementKind::BG, "bg");
    set_size_fullscreen("bg");
    set_element_color("bg", "CS:blue");
    end_attributes("bg");

    declare_state_int("count", 0);

    create_element(ElementKind::TEXT, "title", "bg");
    set_element_text("title", "Taps: {count}");
    set_element_color("title", "CS:white");
    set_element_font_size("title", 32.0f, Unit::SP);
    set_element_align("title", Align::CENTER);
    end_attributes("title");

    create_element(ElementKind::BUTTON, "tapBtn", "bg");
    set_element_text("tapBtn", "Tap me");
    set_element_width("tapBtn", 200.0f, Unit::DP);
    set_element_height("tapBtn", 64.0f, Unit::DP);
    end_attributes("tapBtn");

    create_element(ElementKind::BUTTON, "resetBtn", "bg");
    set_element_text("resetBtn", "Reset");
    end_attributes("resetBtn");

    on_event("tapBtn", EventType::TAPPED, []() {
        set_state_int("count", get_state_int("count") + 1);
        if (get_state_int("count") > 99) {
            set_state_int("count", 0);
        }
    });

    on_event("resetBtn", EventType::TAPPED, []() {
        set_state_int("count", 0);
    });
}

void test_state_interpolation_and_event() {
    setup_counter_program();

    // Initial check
    assert(get_state_int("count") == 0);
    assert(get_rendered_text("title") == "Taps: 0");

    // Fire tapBtn event
    trigger_event("tapBtn", EventType::TAPPED);
    assert(get_state_int("count") == 1);
    assert(get_rendered_text("title") == "Taps: 1");

    // Multiple taps
    for (int i = 0; i < 5; ++i) {
        trigger_event("tapBtn", EventType::TAPPED);
    }
    assert(get_state_int("count") == 6);
    assert(get_rendered_text("title") == "Taps: 6");

    // Reset button tap
    trigger_event("resetBtn", EventType::TAPPED);
    assert(get_state_int("count") == 0);
    assert(get_rendered_text("title") == "Taps: 0");

    std::cout << "  [PASS] test_state_interpolation_and_event\n";
}

void test_touch_hit_test_and_layout() {
    setup_counter_program();

    Renderer& r = Renderer::instance();
    r.init_graphics(1080, 2400, 2.0f);

    const Rect* tap_rect = r.get_element_rect("tapBtn");
    assert(tap_rect != nullptr);
    assert(tap_rect->w > 0);
    assert(tap_rect->h > 0);

    // Hit test inside tapBtn
    float tap_x = tap_rect->x + tap_rect->w / 2.0f;
    float tap_y = tap_rect->y + tap_rect->h / 2.0f;
    assert(r.hit_test(tap_x, tap_y) == "tapBtn");

    int prev_count = get_state_int("count");

    // Simulate touch down and touch up
    r.on_touch_down(tap_x, tap_y);
    r.on_touch_up(tap_x, tap_y);

    // Verified: touch input changes the state and title text!
    assert(get_state_int("count") == prev_count + 1);
    assert(get_rendered_text("title") == "Taps: " + std::to_string(prev_count + 1));

    std::cout << "  [PASS] test_touch_hit_test_and_layout\n";
}

int main() {
    std::cout << "Running LORE Milestone M5 Runtime & Interaction Test Suite:\n";
    test_state_interpolation_and_event();
    test_touch_hit_test_and_layout();
    std::cout << "All M5 Text, Button, and State tests passed successfully!\n";
    return 0;
}
