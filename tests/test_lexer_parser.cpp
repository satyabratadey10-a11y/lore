#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "source_pos.h"
#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"

using namespace lore;

void test_lexer_basic() {
    std::string src = "root is bg. bg colour is CS:blue. bg width is 200dp. .endbgatt.";
    Lexer lexer("test.lore", src);
    auto tokens = lexer.tokenize();

    assert(tokens.size() > 5);
    assert(tokens[0].type == TokenType::KW_ROOT);
    assert(tokens[1].type == TokenType::KW_IS);
    assert(tokens[2].type == TokenType::IDENT);
    assert(tokens[3].type == TokenType::DOT);

    // Dimension check
    bool found_dim = false;
    for (const auto& t : tokens) {
        if (t.type == TokenType::DIMENSION) {
            assert(t.num_value == 200.0);
            assert(t.unit == "dp");
            found_dim = true;
        }
    }
    assert(found_dim);

    // Attr end check
    assert(tokens[tokens.size() - 2].type == TokenType::ATTR_END);
    assert(tokens[tokens.size() - 2].handle == "bg");
    std::cout << "  [PASS] test_lexer_basic\n";
}

void test_lexer_block_terminators() {
    std::string src = "EndOfPermission. EndOfHCCS. EndOfWhen. EndOfIf. EndOfRepeat.";
    Lexer lexer("test.lore", src);
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::END_OF_PERMISSION);
    assert(tokens[1].type == TokenType::END_OF_HCCS);
    assert(tokens[2].type == TokenType::END_OF_WHEN);
    assert(tokens[3].type == TokenType::END_OF_IF);
    assert(tokens[4].type == TokenType::END_OF_REPEAT);
    std::cout << "  [PASS] test_lexer_block_terminators\n";
}

void test_parser_counter_lore() {
    std::string src = R"(
package:name : "com.axilo.counter".
package:id : "counter1".
package:label : "Counter".

import [ UI ].

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
bg image is "assets/wall.png".
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
tapBtn image is "assets/btn_up.png".
tapBtn image:pressed is "assets/btn_down.png".
tapBtn image-fit is stretch.
.endtapBtnatt.

put button resetBtn inside bg.
resetBtn text is "Reset".
resetBtn image is "assets/btn_up.png".
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

    assert(ast != nullptr);
    assert(ast->header != nullptr);
    assert(ast->header->name == "com.axilo.counter");
    assert(ast->header->id == "counter1");
    assert(ast->header->label == "Counter");

    assert(ast->imports.size() == 1);
    assert(ast->imports[0]->item == "UI");

    assert(ast->permissions != nullptr);
    assert(ast->permissions->items.size() == 3);

    assert(ast->hccs != nullptr);
    assert(ast->hccs->items.size() == 3);

    assert(ast->root != nullptr);
    assert(ast->root->kind == "bg");

    assert(!ast->top_items.empty());
    std::cout << "  [PASS] test_parser_counter_lore\n";
}

void test_parser_hello_lore() {
    std::string src = R"(
package:name : "com.axilo.hello".
package:id : "hello1".
package:lable : "Hello LORE".

import [ UI ].

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

    assert(ast != nullptr);
    assert(ast->header->label == "Hello LORE"); // tested lable alias
    assert(ast->hccs->items.size() == 1);
    assert(ast->root->kind == "bg");
    std::cout << "  [PASS] test_parser_hello_lore\n";
}

void test_parser_error_missing_end_of_permission() {
    std::string src = R"(
package:name : "com.axilo.bad".
package:id : "bad1".
package:label : "Bad".
permission
  ShowDisplay : true.
root is bg.
)";

    bool caught = false;
    try {
        Lexer lexer("bad.lore", src);
        auto tokens = lexer.tokenize();
        Parser parser("bad.lore", tokens);
        parser.parse_program();
    } catch (const CompileError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("EndOfPermission.") != std::string::npos);
        std::cout << "  [PASS] Caught expected error: " << msg << "\n";
    }
    assert(caught);
}

void test_parser_error_missing_end_of_when() {
    std::string src = R"(
package:name : "com.axilo.bad".
package:id : "bad1".
package:label : "Bad".
root is bg.
.endbgatt.
when btn is tapped
  count is 1.
)";

    bool caught = false;
    try {
        Lexer lexer("bad.lore", src);
        auto tokens = lexer.tokenize();
        Parser parser("bad.lore", tokens);
        parser.parse_program();
    } catch (const CompileError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("EndOfWhen.") != std::string::npos);
        std::cout << "  [PASS] Caught expected error: " << msg << "\n";
    }
    assert(caught);
}

void test_parser_display_statements() {
    std::string src = R"(
package:name : "com.axilo.stages".
package:id : "stages1".
package:label : "Stages".
permission
  OutputDisplay : true.
EndOfPermission.
root is bg.
.endbgatt.
show display at input.
show display at process.
show display at output.
hide display.
)";

    Lexer lexer("stages.lore", src);
    auto tokens = lexer.tokenize();
    Parser parser("stages.lore", tokens);
    auto ast = parser.parse_program();

    assert(ast != nullptr);
    assert(ast->top_items.size() == 5); // .endbgatt. + 4 display statements
    std::cout << "  [PASS] test_parser_display_statements\n";
}

int main() {
    std::cout << "Running LORE Lexer and Parser Test Suite:\n";
    test_lexer_basic();
    test_lexer_block_terminators();
    test_parser_counter_lore();
    test_parser_hello_lore();
    test_parser_error_missing_end_of_permission();
    test_parser_error_missing_end_of_when();
    test_parser_display_statements();
    std::cout << "All M1 Lexer and Parser tests passed successfully!\n";
    return 0;
}
