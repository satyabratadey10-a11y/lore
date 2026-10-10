#include <iostream>
#include <cassert>
#include <string>
#include <fstream>
#include <cstdlib>

#include "lexer.h"
#include "parser.h"
#include "checker.h"
#include "codegen_cpp.h"

using namespace lore;

void test_codegen_counter() {
    std::string src = R"(
package:name : "com.axilo.counter".
package:id : "counter1".
package:label : "Counter".

permission
  InputHarness : true and eachtimevalue : 1.
  ShowDisplay : true.
EndOfPermission.

HCCS
  "#4287f5" is "blue".
  "#e30707" is "red".
  "#ffffff" is "white".
EndOfHCCS.

root is bg.
bg size is fullscreen.
bg colour is CS:blue.
bg image is "examples/assets/wall.png".
bg image-fit is cover.
.endbgatt.

state count is 0.

put text title inside bg.
title text is "Taps: {count}".
title colour is CS:white.
title size is 32sp.
title align is center.
.endtitleatt.

put button tapBtn inside bg.
tapBtn text is "Tap me".
tapBtn width is 200dp.
tapBtn height is 64dp.
tapBtn image is "examples/assets/btn_up.png".
tapBtn image:pressed is "examples/assets/btn_down.png".
tapBtn image-fit is stretch.
.endtapBtnatt.

put button resetBtn inside bg.
resetBtn text is "Reset".
resetBtn image is "examples/assets/btn_up.png".
resetBtn image-tint is CS:red.
.endresetBtnatt.

when tapBtn is tapped
  count is count plus 1.
  if count is greater than 99 then
    count is 0.
  EndOfIf.
EndOfWhen.

when resetBtn is tapped
  count is 0.
EndOfWhen.
)";

    Lexer lexer("counter.lore", src);
    auto tokens = lexer.tokenize();
    Parser parser("counter.lore", tokens);
    auto ast = parser.parse_program();

    Checker checker(".");
    checker.check_program(ast);

    CodegenCpp codegen;
    std::string code = codegen.generate(ast);

    // Assert key C++ constructs are present
    assert(code.find("lore_user_program_setup") != std::string::npos);
    assert(code.find("lore::set_package(\"com.axilo.counter\"") != std::string::npos);
    assert(code.find("lore::set_perm_input_harness(true)") != std::string::npos);
    assert(code.find("lore::set_perm_show_display(true)") != std::string::npos);
    assert(code.find("lore::register_color(\"blue\", \"#4287f5\")") != std::string::npos);
    assert(code.find("lore::create_root(lore::ElementKind::BG, \"bg\")") != std::string::npos);
    assert(code.find("lore::set_size_fullscreen(\"bg\")") != std::string::npos);
    assert(code.find("lore::declare_state_int(\"count\", 0)") != std::string::npos);
    assert(code.find("lore::on_event(\"tapBtn\", lore::EventType::TAPPED") != std::string::npos);
    assert(code.find("lore::set_state_int(\"count\", (lore::get_state_int(\"count\") + 1))") != std::string::npos);
    assert(code.find("if ((lore::get_state_int(\"count\") > 99))") != std::string::npos);

    std::cout << "  [PASS] test_codegen_counter\n";
}

void test_codegen_hello() {
    std::string src = R"(
package:name : "com.axilo.hello".
package:id : "hello1".
package:label : "Hello LORE".

permission
  ShowDisplay : true.
EndOfPermission.

HCCS
  "#4287f5" is "blue".
EndOfHCCS.

root is bg.
bg size is fullscreen.
bg colour is CS:blue.
.endbgatt.
)";

    Lexer lexer("hello.lore", src);
    auto tokens = lexer.tokenize();
    Parser parser("hello.lore", tokens);
    auto ast = parser.parse_program();

    CodegenCpp codegen;
    std::string code = codegen.generate(ast);

    assert(code.find("lore_user_program_setup") != std::string::npos);
    assert(code.find("lore::set_package(\"com.axilo.hello\"") != std::string::npos);
    assert(code.find("lore::create_root(lore::ElementKind::BG, \"bg\")") != std::string::npos);
    assert(code.find("lore::set_size_fullscreen(\"bg\")") != std::string::npos);
    assert(code.find("lore::set_element_color(\"bg\", \"CS:blue\")") != std::string::npos);

    std::cout << "  [PASS] test_codegen_hello\n";
}

int main() {
    std::cout << "Running LORE C++ Codegen Test Suite (M3):\n";
    test_codegen_counter();
    test_codegen_hello();
    std::cout << "All M3 Codegen tests passed successfully!\n";
    return 0;
}
