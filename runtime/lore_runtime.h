#ifndef LORE_RUNTIME_H
#define LORE_RUNTIME_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>

#ifdef _WIN32
  #define LORE_EXPORT __declspec(dllexport)
#else
  #define LORE_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Standard C ABI exported by generated LORE library
// ============================================================================
LORE_EXPORT void lore_init(const char* config);
LORE_EXPORT void lore_input(const void* signal);
LORE_EXPORT void lore_stage(int stage);
LORE_EXPORT void lore_render(void* surface);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace lore {

enum class ElementKind {
    BG,
    TEXT,
    BUTTON,
    IMAGE,
    ROW,
    COLUMN,
    BOX
};

enum class Unit {
    NONE,
    DP,
    SP,
    PX,
    PERCENT
};

struct Dimension {
    float value = 0.0f;
    Unit unit = Unit::NONE;
};

enum class ImageFit {
    NONE,
    COVER,
    CONTAIN,
    STRETCH,
    TILE
};

enum class Align {
    NONE,
    START,
    CENTER,
    END,
    WRAP
};

enum class EventType {
    TAPPED,
    PRESSED,
    RELEASED,
    CHANGED,
    INPUT
};

enum class Stage {
    INPUT = 0,
    PROCESS = 1,
    OUTPUT = 2
};

struct ColorRGBA {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

// ============================================================================
// LORE Runtime API called by generated C++
// ============================================================================

// Package info
void set_package(const std::string& name, const std::string& id, const std::string& label);

// Permissions
void set_perm_input_harness(bool enabled);
void set_perm_eachtimevalue(int count);
void set_perm_show_display(bool enabled);
void set_perm_output_display(bool enabled);

// HCCS (Colour Shortcuts)
void register_color(const std::string& name, const std::string& hex_code);
ColorRGBA resolve_color(const std::string& color_spec);
ColorRGBA get_root_background_color();

// State management
void declare_state_int(const std::string& name, int64_t initial_value);
void declare_state_str(const std::string& name, const std::string& initial_value);
int64_t get_state_int(const std::string& name);
std::string get_state_str(const std::string& name);
void set_state_int(const std::string& name, int64_t value);
void set_state_str(const std::string& name, const std::string& value);
std::string interpolate_string(const std::string& template_str);

// UI Element Tree
void create_root(ElementKind kind, const std::string& handle);
void create_element(ElementKind kind, const std::string& handle, const std::string& parent_handle);
void set_size_fullscreen(const std::string& handle);
void set_element_width(const std::string& handle, float val, Unit unit);
void set_element_height(const std::string& handle, float val, Unit unit);
void set_element_color(const std::string& handle, const std::string& color_spec);
void set_element_text(const std::string& handle, const std::string& text);
void set_element_font_size(const std::string& handle, float val, Unit unit);
void set_element_align(const std::string& handle, Align align);
void set_element_image(const std::string& handle, const std::string& asset_path);
void set_element_image_pressed(const std::string& handle, const std::string& asset_path);
void set_element_image_disabled(const std::string& handle, const std::string& asset_path);
void set_element_image_fit(const std::string& handle, ImageFit fit);
void set_element_image_tint(const std::string& handle, const std::string& color_spec);
void end_attributes(const std::string& handle);

// Events
void on_event(const std::string& handle, EventType event, std::function<void()> handler);
void trigger_event(const std::string& handle, EventType event);

// Element queries
std::string get_root_handle();
std::vector<std::string> get_child_handles(const std::string& parent_handle);
std::string get_rendered_text(const std::string& handle);

// Stage & display statements
void show_display_at(Stage stage);
void hide_display();

// Setup hook implemented by user-generated LORE program
extern "C" void lore_user_program_setup();

} // namespace lore
#endif

#endif // LORE_RUNTIME_H
