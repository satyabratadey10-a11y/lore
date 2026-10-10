#include <iostream>
#include <cassert>
#include <string>

#include "source_pos.h"
#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "checker.h"

using namespace lore;

std::shared_ptr<ProgramNode> parse_source(const std::string& name, const std::string& src) {
    Lexer lexer(name, src);
    auto tokens = lexer.tokenize();
    Parser parser(name, tokens);
    return parser.parse_program();
}

void test_check_counter_valid() {
    std::string src = R"(
package:name : "com.axilo.counter".
package:id : "counter1".
package:label : "Counter".

permission
  InputHarness : true.
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
.endbgatt.

state count is 0.

put text title inside bg.
title text is "Taps: {count}".
title colour is CS:white.
.endtitleatt.

put button tapBtn inside bg.
tapBtn text is "Tap me".
tapBtn image is "examples/assets/btn_up.png".
.endtapBtnatt.

when tapBtn is tapped
  count is count plus 1.
EndOfWhen.
)";

    auto ast = parse_source("valid_counter.lore", src);
    Checker checker(".");
    checker.check_program(ast);
    std::cout << "  [PASS] test_check_counter_valid\n";
}

void test_check_mismatched_end_att() {
    std::string src = R"(
package:name : "com.axilo.test".
package:id : "test1".
package:label : "Test".

root is bg.
bg size is fullscreen.
.endtitleatt.
)";

    auto ast = parse_source("bad_att.lore", src);
    Checker checker(".");
    bool caught = false;
    try {
        checker.check_program(ast);
    } catch (const CompileError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("Mismatched attribute terminator") != std::string::npos);
        std::cout << "  [PASS] Caught expected error: " << msg << "\n";
    }
    assert(caught);
}

void test_check_missing_parent() {
    std::string src = R"(
package:name : "com.axilo.test".
package:id : "test1".
package:label : "Test".

root is bg.
.endbgatt.

put button btn inside nonExistentContainer.
.endbtnatt.
)";

    auto ast = parse_source("missing_parent.lore", src);
    Checker checker(".");
    bool caught = false;
    try {
        checker.check_program(ast);
    } catch (const CompileError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("does not exist") != std::string::npos);
        std::cout << "  [PASS] Caught expected error: " << msg << "\n";
    }
    assert(caught);
}

void test_check_unknown_colour_ref() {
    std::string src = R"(
package:name : "com.axilo.test".
package:id : "test1".
package:label : "Test".

HCCS
  "#4287f5" is "blue".
EndOfHCCS.

root is bg.
bg colour is CS:magenta.
.endbgatt.
)";

    auto ast = parse_source("bad_color.lore", src);
    Checker checker(".");
    bool caught = false;
    try {
        checker.check_program(ast);
    } catch (const CompileError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("Unknown colour reference") != std::string::npos);
        std::cout << "  [PASS] Caught expected error: " << msg << "\n";
    }
    assert(caught);
}

void test_check_missing_image_file() {
    std::string src = R"(
package:name : "com.axilo.test".
package:id : "test1".
package:label : "Test".

root is bg.
bg image is "assets/does_not_exist_anywhere.png".
.endbgatt.
)";

    auto ast = parse_source("missing_img.lore", src);
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

void test_check_invalid_image_extension() {
    std::string src = R"(
package:name : "com.axilo.test".
package:id : "test1".
package:label : "Test".

root is bg.
bg image is "assets/wall.bmp".
.endbgatt.
)";

    auto ast = parse_source("bad_ext.lore", src);
    Checker checker(".");
    bool caught = false;
    try {
        checker.check_program(ast);
    } catch (const CompileError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("Invalid image extension") != std::string::npos);
        std::cout << "  [PASS] Caught expected error: " << msg << "\n";
    }
    assert(caught);
}

void test_check_undeclared_state_in_assignment() {
    std::string src = R"(
package:name : "com.axilo.test".
package:id : "test1".
package:label : "Test".

root is bg.
.endbgatt.

put button btn inside bg.
.endbtnatt.

when btn is tapped
  undeclaredVar is 10.
EndOfWhen.
)";

    auto ast = parse_source("bad_state.lore", src);
    Checker checker(".");
    bool caught = false;
    try {
        checker.check_program(ast);
    } catch (const CompileError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("undeclared state variable") != std::string::npos);
        std::cout << "  [PASS] Caught expected error: " << msg << "\n";
    }
    assert(caught);
}

void test_check_display_warning() {
    std::string src = R"(
package:name : "com.axilo.test".
package:id : "test1".
package:label : "Test".

permission
  ShowDisplay : true.
  OutputDisplay : true.
EndOfPermission.

root is bg.
.endbgatt.
)";

    auto ast = parse_source("warn.lore", src);
    Checker checker(".");
    checker.check_program(ast);
    assert(!checker.warnings().empty());
    assert(checker.warnings()[0].find("already includes 'OutputDisplay : true'") != std::string::npos);
    std::cout << "  [PASS] test_check_display_warning (Warning caught: " << checker.warnings()[0] << ")\n";
}

int main() {
    std::cout << "Running LORE Semantic Checker Test Suite (M2):\n";
    test_check_counter_valid();
    test_check_mismatched_end_att();
    test_check_missing_parent();
    test_check_unknown_colour_ref();
    test_check_missing_image_file();
    test_check_invalid_image_extension();
    test_check_undeclared_state_in_assignment();
    test_check_display_warning();
    std::cout << "All M2 Semantic Checker tests passed successfully!\n";
    return 0;
}
