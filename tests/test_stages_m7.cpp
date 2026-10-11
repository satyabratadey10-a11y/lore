#include <iostream>
#include <cassert>
#include <string>

#include "lore_runtime.h"
#include "lexer.h"
#include "parser.h"
#include "checker.h"
#include "codegen_cpp.h"

using namespace lore;

void test_headless_mode() {
    reset_runtime();
    create_root(ElementKind::BG, "bg");
    end_attributes("bg");

    // Neither ShowDisplay nor OutputDisplay is granted
    assert(!is_display_active());
    set_stage(Stage::INPUT);
    assert(!is_display_active());
    set_stage(Stage::PROCESS);
    assert(!is_display_active());
    set_stage(Stage::OUTPUT);
    assert(!is_display_active());

    // Even if show_display_at is called, headless mode prohibits UI
    show_display_at(Stage::OUTPUT);
    assert(!is_display_active());

    std::cout << "  [PASS] test_headless_mode\n";
}

void test_show_display_permission() {
    reset_runtime();
    set_perm_show_display(true);
    set_perm_input_harness(true);
    create_root(ElementKind::BG, "bg");
    end_attributes("bg");

    assert(is_input_harness_active());

    // ShowDisplay is true: display active all the time across all stages
    set_stage(Stage::INPUT);
    assert(is_display_active());
    set_stage(Stage::PROCESS);
    assert(is_display_active());
    set_stage(Stage::OUTPUT);
    assert(is_display_active());

    // Explicit hide
    hide_display();
    assert(!is_display_active());

    // Re-enable
    show_display_at(Stage::OUTPUT);
    assert(is_display_active());

    std::cout << "  [PASS] test_show_display_permission\n";
}

void test_output_display_and_stages() {
    reset_runtime();
    set_perm_output_display(true);
    create_root(ElementKind::BG, "bg");
    end_attributes("bg");

    // Before any show_display_at statement, display is inactive
    set_stage(Stage::INPUT);
    assert(!is_display_active());

    // Program declares show display at output
    show_display_at(Stage::OUTPUT);

    // Current stage is INPUT -> not active
    set_stage(Stage::INPUT);
    assert(!is_display_active());

    // Stage changes to PROCESS -> not active
    set_stage(Stage::PROCESS);
    assert(!is_display_active());

    // Stage changes to OUTPUT -> active!
    set_stage(Stage::OUTPUT);
    assert(is_display_active());

    // Hide display
    hide_display();
    assert(!is_display_active());

    // Show display at output again
    show_display_at(Stage::OUTPUT);
    assert(is_display_active());

    std::cout << "  [PASS] test_output_display_and_stages\n";
}

void test_c_abi_stages() {
    reset_runtime();
    set_perm_output_display(true);
    create_root(ElementKind::BG, "bg");
    end_attributes("bg");
    show_display_at(Stage::OUTPUT);

    // Host transitions through ABI
    lore_stage(0); // INPUT
    assert(get_current_stage() == Stage::INPUT);
    assert(!is_display_active());

    lore_stage(1); // PROCESS
    assert(get_current_stage() == Stage::PROCESS);
    assert(!is_display_active());

    lore_stage(2); // OUTPUT
    assert(get_current_stage() == Stage::OUTPUT);
    assert(is_display_active());

    // Render should succeed without throwing
    lore_render(nullptr);

    std::cout << "  [PASS] test_c_abi_stages\n";
}

void test_compiler_stages_codegen() {
    std::string src = R"(
package:name : "com.axilo.stagedemo".
package:id : "stage1".
package:label : "StageDemo".

permission
  InputHarness : true and
  OutputDisplay : true.
EndOfPermission.

root is bg.
.endbgatt.

show display at output.
)";

    Lexer lexer("stage_test.lore", src);
    auto tokens = lexer.tokenize();
    Parser parser("stage_test.lore", tokens);
    auto ast = parser.parse_program();

    Checker checker(".");
    checker.check_program(ast);

    CodegenCpp codegen;
    std::string cpp = codegen.generate(ast);

    assert(cpp.find("lore::set_perm_input_harness(true);") != std::string::npos);
    assert(cpp.find("lore::set_perm_output_display(true);") != std::string::npos);
    assert(cpp.find("lore::show_display_at(lore::Stage::OUTPUT);") != std::string::npos);

    std::cout << "  [PASS] test_compiler_stages_codegen\n";
}

int main() {
    std::cout << "Running LORE Permissions & Stages Test Suite (M7):\n";
    test_headless_mode();
    test_show_display_permission();
    test_output_display_and_stages();
    test_c_abi_stages();
    test_compiler_stages_codegen();
    std::cout << "All M7 stage & permission tests passed successfully!\n";
    return 0;
}
