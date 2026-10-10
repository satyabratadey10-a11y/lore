#ifndef LORE_RENDERER_H
#define LORE_RENDERER_H

#include "lore_runtime.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace lore {

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
};

class Renderer {
public:
    static Renderer& instance();

    void init_graphics(int screen_width, int screen_height, float density = 2.0f);
    void update_layout(int screen_width, int screen_height, float density = 2.0f);
    void render_frame();

    void on_touch_down(float x, float y);
    void on_touch_up(float x, float y);

    const Rect* get_element_rect(const std::string& handle) const;
    std::string hit_test(float x, float y) const;

    bool needs_redraw() const { return needs_redraw_; }
    void request_redraw() { needs_redraw_ = true; }

private:
    Renderer();
    ~Renderer();

    int screen_w_ = 1080;
    int screen_h_ = 2400;
    float density_ = 2.0f;
    bool needs_redraw_ = true;
    std::string pressed_handle_;

    std::unordered_map<std::string, Rect> layout_rects_;

    // Font data
    std::vector<uint8_t> font_buffer_;
    bool font_loaded_ = false;

    void load_font();
    void compute_layout();
};

} // namespace lore

#endif // LORE_RENDERER_H
