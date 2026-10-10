#include <iostream>
#include <cassert>
#include <string>

#include "stb_image.h"
#include "lore_runtime.h"
#include "lore_renderer.h"
#include "lexer.h"
#include "parser.h"
#include "checker.h"

using namespace lore;

void test_stb_image_loading() {
    int w = 0, h = 0, channels = 0;
    std::string path = "examples/assets/wall.png";
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) {
        // Try ../
        path = "../examples/assets/wall.png";
        data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    }
    assert(data != nullptr);
    assert(w == 64);
    assert(h == 64);
    stbi_image_free(data);
    std::cout << "  [PASS] test_stb_image_loading (64x64 RGBA)\n";
}

void test_image_pressed_state_switching() {
    create_root(ElementKind::BG, "bg");
    set_size_fullscreen("bg");
    set_element_image("bg", "assets/wall.png");
    set_element_image_fit("bg", ImageFit::COVER);
    end_attributes("bg");

    create_element(ElementKind::BUTTON, "tapBtn", "bg");
    set_element_image("tapBtn", "assets/btn_up.png");
    set_element_image_pressed("tapBtn", "assets/btn_down.png");
    set_element_image_fit("tapBtn", ImageFit::STRETCH);
    end_attributes("tapBtn");

    create_element(ElementKind::BUTTON, "resetBtn", "bg");
    set_element_image("resetBtn", "assets/btn_up.png");
    register_color("red", "#e30707");
    set_element_image_tint("resetBtn", "CS:red");
    end_attributes("resetBtn");

    // 1. Idle state check
    assert(get_active_texture_path("tapBtn") == "assets/btn_up.png");
    assert(get_element_image_fit("bg") == ImageFit::COVER);
    assert(get_element_image_fit("tapBtn") == ImageFit::STRETCH);

    // 2. Tint check on resetBtn
    ColorRGBA tint = get_element_image_tint("resetBtn");
    assert(tint.r > 0.8f); // Red component #e30707

    // 3. Press button -> switches texture to image:pressed
    set_element_pressed("tapBtn", true);
    assert(get_active_texture_path("tapBtn") == "assets/btn_down.png");

    // 4. Release button -> switches back
    set_element_pressed("tapBtn", false);
    assert(get_active_texture_path("tapBtn") == "assets/btn_up.png");

    std::cout << "  [PASS] test_image_pressed_state_switching\n";
}

void test_missing_image_compile_error() {
    std::string src = R"(
package:name : "com.axilo.test".
package:id : "test1".
package:label : "Test".

root is bg.
bg image is "assets/does_not_exist.png".
.endbgatt.
)";

    Lexer lexer("bad_img.lore", src);
    auto tokens = lexer.tokenize();
    Parser parser("bad_img.lore", tokens);
    auto ast = parser.parse_program();

    Checker checker(".");
    bool caught = false;
    try {
        checker.check_program(ast);
    } catch (const CompileError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("Image file not found") != std::string::npos);
        std::cout << "  [PASS] Caught expected error: " << msg << "\n";
    }
    assert(caught);
}

int main() {
    std::cout << "Running LORE Image Textures Test Suite (M6):\n";
    test_stb_image_loading();
    test_image_pressed_state_switching();
    test_missing_image_compile_error();
    std::cout << "All M6 Image and Texture tests passed successfully!\n";
    return 0;
}
