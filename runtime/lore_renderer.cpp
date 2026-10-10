#include "lore_renderer.h"
#include <iostream>
#include <fstream>
#include <cmath>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if defined(LORE_TARGET_ANDROID)
#include <GLES3/gl3.h>
#endif

namespace lore {

Renderer& Renderer::instance() {
    static Renderer inst;
    return inst;
}

Renderer::Renderer() {
    load_font();
}

Renderer::~Renderer() {}

void Renderer::load_font() {
    std::vector<std::string> font_paths = {
        "third_party/Roboto-Regular.ttf",
        "../third_party/Roboto-Regular.ttf",
        "/system/fonts/Roboto-Regular.ttf"
    };

    for (const auto& path : font_paths) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            font_buffer_.resize(size);
            if (file.read(reinterpret_cast<char*>(font_buffer_.data()), size)) {
                font_loaded_ = true;
                break;
            }
        }
    }
}

void Renderer::init_graphics(int screen_width, int screen_height, float density) {
    screen_w_ = screen_width;
    screen_h_ = screen_height;
    density_ = density;
    update_layout(screen_width, screen_height, density);
}

void Renderer::update_layout(int screen_width, int screen_height, float density) {
    screen_w_ = screen_width;
    screen_h_ = screen_height;
    density_ = density;
    compute_layout();
    needs_redraw_ = true;
}

void Renderer::compute_layout() {
    layout_rects_.clear();
    std::string root = get_root_handle();
    if (root.empty()) return;

    layout_rects_[root] = Rect{0.0f, 0.0f, static_cast<float>(screen_w_), static_cast<float>(screen_h_)};

    auto children = get_child_handles(root);
    float current_y = 120.0f * density_;

    for (const auto& child : children) {
        float w = 240.0f * density_;
        float h = 60.0f * density_;

        if (child == "title") {
            w = screen_w_ * 0.9f;
            h = 64.0f * density_;
        } else if (child == "tapBtn") {
            w = 200.0f * density_;
            h = 64.0f * density_;
        } else if (child == "resetBtn") {
            w = 200.0f * density_;
            h = 56.0f * density_;
        }

        float x = (screen_w_ - w) / 2.0f;
        layout_rects_[child] = Rect{x, current_y, w, h};
        current_y += h + 28.0f * density_;
    }
}

const Rect* Renderer::get_element_rect(const std::string& handle) const {
    auto it = layout_rects_.find(handle);
    if (it != layout_rects_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string Renderer::hit_test(float x, float y) const {
    std::string root = get_root_handle();
    if (root.empty()) return "";

    auto children = get_child_handles(root);
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto r_it = layout_rects_.find(*it);
        if (r_it != layout_rects_.end() && r_it->second.contains(x, y)) {
            return *it;
        }
    }

    auto root_it = layout_rects_.find(root);
    if (root_it != layout_rects_.end() && root_it->second.contains(x, y)) {
        return root;
    }
    return "";
}

void Renderer::on_touch_down(float x, float y) {
    std::string handle = hit_test(x, y);
    if (!handle.empty()) {
        pressed_handle_ = handle;
        trigger_event(handle, EventType::PRESSED);
        needs_redraw_ = true;
    }
}

void Renderer::on_touch_up(float x, float y) {
    if (!pressed_handle_.empty()) {
        std::string hit = hit_test(x, y);
        trigger_event(pressed_handle_, EventType::RELEASED);
        if (hit == pressed_handle_) {
            trigger_event(pressed_handle_, EventType::TAPPED);
        }
        pressed_handle_.clear();
        needs_redraw_ = true;
    }
}

void Renderer::render_frame() {
#if defined(LORE_TARGET_ANDROID)
    auto bg_color = get_root_background_color();
    glClearColor(bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render elements here via GLES3
#endif
    needs_redraw_ = false;
}

} // namespace lore
