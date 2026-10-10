#include "lore_runtime.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace lore {

struct PackageInfo {
    std::string name = "com.axilo.app";
    std::string id = "app1";
    std::string label = "LORE App";
};

struct Permissions {
    bool input_harness = false;
    int eachtimevalue = 1;
    bool show_display = false;
    bool output_display = false;
};

struct UIElement {
    ElementKind kind = ElementKind::BG;
    std::string handle;
    std::string parent_handle;
    std::vector<std::string> child_handles;

    bool fullscreen = false;
    Dimension width;
    Dimension height;
    std::string color_spec;
    std::string text;
    Dimension font_size;
    Align align = Align::NONE;
    std::string image;
    std::string image_pressed;
    std::string image_disabled;
    ImageFit image_fit = ImageFit::NONE;
    std::string image_tint;

    bool is_pressed = false;
    bool is_disabled = false;
};

class RuntimeState {
public:
    static RuntimeState& instance() {
        static RuntimeState inst;
        return inst;
    }

    PackageInfo package;
    Permissions permissions;
    std::unordered_map<std::string, std::string> colors; // name -> hex ("blue" -> "#4287f5")
    std::unordered_map<std::string, int64_t> state_ints;
    std::unordered_map<std::string, std::string> state_strs;
    std::unordered_map<std::string, UIElement> elements;
    std::string root_handle;

    std::unordered_map<std::string, std::vector<std::pair<EventType, std::function<void()>>>> handlers;

    Stage current_stage = Stage::INPUT;
    bool display_visible = true;

    void reset() {
        package = PackageInfo{};
        permissions = Permissions{};
        colors.clear();
        state_ints.clear();
        state_strs.clear();
        elements.clear();
        root_handle.clear();
        handlers.clear();
        current_stage = Stage::INPUT;
        display_visible = true;
    }
};

void set_package(const std::string& name, const std::string& id, const std::string& label) {
    auto& s = RuntimeState::instance();
    s.package.name = name;
    s.package.id = id;
    s.package.label = label;
}

void set_perm_input_harness(bool enabled) {
    RuntimeState::instance().permissions.input_harness = enabled;
}

void set_perm_eachtimevalue(int count) {
    RuntimeState::instance().permissions.eachtimevalue = count;
}

void set_perm_show_display(bool enabled) {
    RuntimeState::instance().permissions.show_display = enabled;
}

void set_perm_output_display(bool enabled) {
    RuntimeState::instance().permissions.output_display = enabled;
}

void register_color(const std::string& name, const std::string& hex_code) {
    RuntimeState::instance().colors[name] = hex_code;
}

ColorRGBA resolve_color(const std::string& color_spec) {
    auto& s = RuntimeState::instance();
    std::string hex = color_spec;
    if (color_spec.rfind("CS:", 0) == 0) {
        std::string name = color_spec.substr(3);
        auto it = s.colors.find(name);
        if (it != s.colors.end()) {
            hex = it->second;
        } else {
            std::cerr << "[LORE Runtime] Unknown color shortcut: " << color_spec << std::endl;
            return {1.0f, 1.0f, 1.0f, 1.0f};
        }
    }

    if (!hex.empty() && hex[0] == '#') {
        hex = hex.substr(1);
    }
    if (hex.length() == 6) {
        uint32_t val = 0;
        std::stringstream ss;
        ss << std::hex << hex;
        ss >> val;
        float r = ((val >> 16) & 0xFF) / 255.0f;
        float g = ((val >> 8) & 0xFF) / 255.0f;
        float b = (val & 0xFF) / 255.0f;
        return {r, g, b, 1.0f};
    }
    return {1.0f, 1.0f, 1.0f, 1.0f};
}

ColorRGBA get_root_background_color() {
    auto& s = RuntimeState::instance();
    if (!s.root_handle.empty()) {
        auto it = s.elements.find(s.root_handle);
        if (it != s.elements.end() && !it->second.color_spec.empty()) {
            return resolve_color(it->second.color_spec);
        }
    }
    return resolve_color("CS:blue");
}

void declare_state_int(const std::string& name, int64_t initial_value) {
    RuntimeState::instance().state_ints[name] = initial_value;
}

void declare_state_str(const std::string& name, const std::string& initial_value) {
    RuntimeState::instance().state_strs[name] = initial_value;
}

int64_t get_state_int(const std::string& name) {
    auto& m = RuntimeState::instance().state_ints;
    auto it = m.find(name);
    return it != m.end() ? it->second : 0;
}

std::string get_state_str(const std::string& name) {
    auto& s = RuntimeState::instance();
    auto it_s = s.state_strs.find(name);
    if (it_s != s.state_strs.end()) return it_s->second;
    auto it_i = s.state_ints.find(name);
    if (it_i != s.state_ints.end()) return std::to_string(it_i->second);
    return "";
}

void set_state_int(const std::string& name, int64_t value) {
    RuntimeState::instance().state_ints[name] = value;
}

void set_state_str(const std::string& name, const std::string& value) {
    RuntimeState::instance().state_strs[name] = value;
}

std::string interpolate_string(const std::string& template_str) {
    std::string result;
    size_t i = 0;
    while (i < template_str.size()) {
        if (template_str[i] == '{') {
            size_t close = template_str.find('}', i + 1);
            if (close != std::string::npos) {
                std::string var_name = template_str.substr(i + 1, close - i - 1);
                result += get_state_str(var_name);
                i = close + 1;
                continue;
            }
        }
        result += template_str[i];
        ++i;
    }
    return result;
}

void create_root(ElementKind kind, const std::string& handle) {
    auto& s = RuntimeState::instance();
    s.elements.clear();
    s.handlers.clear();
    s.root_handle = handle;
    UIElement el;
    el.kind = kind;
    el.handle = handle;
    s.elements[handle] = el;
}

void create_element(ElementKind kind, const std::string& handle, const std::string& parent_handle) {
    auto& s = RuntimeState::instance();
    UIElement el;
    el.kind = kind;
    el.handle = handle;
    el.parent_handle = parent_handle;
    s.elements[handle] = el;
    auto& p_children = s.elements[parent_handle].child_handles;
    if (std::find(p_children.begin(), p_children.end(), handle) == p_children.end()) {
        p_children.push_back(handle);
    }
}

void set_size_fullscreen(const std::string& handle) {
    RuntimeState::instance().elements[handle].fullscreen = true;
}

void set_element_width(const std::string& handle, float val, Unit unit) {
    RuntimeState::instance().elements[handle].width = {val, unit};
}

void set_element_height(const std::string& handle, float val, Unit unit) {
    RuntimeState::instance().elements[handle].height = {val, unit};
}

void set_element_color(const std::string& handle, const std::string& color_spec) {
    RuntimeState::instance().elements[handle].color_spec = color_spec;
}

void set_element_text(const std::string& handle, const std::string& text) {
    RuntimeState::instance().elements[handle].text = text;
}

void set_element_font_size(const std::string& handle, float val, Unit unit) {
    RuntimeState::instance().elements[handle].font_size = {val, unit};
}

void set_element_align(const std::string& handle, Align align) {
    RuntimeState::instance().elements[handle].align = align;
}

void set_element_image(const std::string& handle, const std::string& asset_path) {
    RuntimeState::instance().elements[handle].image = asset_path;
}

void set_element_image_pressed(const std::string& handle, const std::string& asset_path) {
    RuntimeState::instance().elements[handle].image_pressed = asset_path;
}

void set_element_image_disabled(const std::string& handle, const std::string& asset_path) {
    RuntimeState::instance().elements[handle].image_disabled = asset_path;
}

void set_element_image_fit(const std::string& handle, ImageFit fit) {
    RuntimeState::instance().elements[handle].image_fit = fit;
}

void set_element_image_tint(const std::string& handle, const std::string& color_spec) {
    RuntimeState::instance().elements[handle].image_tint = color_spec;
}

void end_attributes(const std::string& handle) {
    // Attributes closed for handle
}

void on_event(const std::string& handle, EventType event, std::function<void()> handler) {
    RuntimeState::instance().handlers[handle].emplace_back(event, handler);
}

void trigger_event(const std::string& handle, EventType event) {
    auto& s = RuntimeState::instance();
    auto it = s.handlers.find(handle);
    if (it != s.handlers.end()) {
        for (const auto& pair : it->second) {
            if (pair.first == event) {
                pair.second();
            }
        }
    }
}

std::string get_root_handle() {
    return RuntimeState::instance().root_handle;
}

std::vector<std::string> get_child_handles(const std::string& parent_handle) {
    auto& s = RuntimeState::instance();
    auto it = s.elements.find(parent_handle);
    if (it != s.elements.end()) {
        return it->second.child_handles;
    }
    return {};
}

std::string get_rendered_text(const std::string& handle) {
    auto& s = RuntimeState::instance();
    auto it = s.elements.find(handle);
    if (it != s.elements.end()) {
        return interpolate_string(it->second.text);
    }
    return "";
}

void show_display_at(Stage stage) {
    auto& s = RuntimeState::instance();
    s.current_stage = stage;
    s.display_visible = true;
}

void hide_display() {
    RuntimeState::instance().display_visible = false;
}

// Default fallback hook if not linked with user program
__attribute__((weak)) void lore_user_program_setup() {
    // Default setup
}

} // namespace lore

// ============================================================================
// Standard C ABI implementation
// ============================================================================
extern "C" {

void lore_init(const char* config) {
    std::cout << "[LORE ABI] lore_init called (config: " << (config ? config : "null") << ")\n";
    lore::lore_user_program_setup();
}

void lore_input(const void* signal) {
    std::cout << "[LORE ABI] lore_input called\n";
}

void lore_stage(int stage) {
    std::cout << "[LORE ABI] lore_stage: " << stage << "\n";
    lore::RuntimeState::instance().current_stage = static_cast<lore::Stage>(stage);
}

void lore_render(void* surface) {
    auto& s = lore::RuntimeState::instance();
    if (!s.display_visible && !s.permissions.show_display) {
        return;
    }
    // Rendering occurs here or via OpenGL in NativeActivity
}

}
