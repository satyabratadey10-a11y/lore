#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

#include "source_pos.h"
#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "checker.h"

#define LORE_VERSION "0.1.0"

namespace lore {

std::string read_file_to_string(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace lore

void print_version() {
    std::cout << "lore version " << LORE_VERSION << " (C++17)" << std::endl;
}

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " <command> [options]\n\n"
              << "Commands:\n"
              << "  check <source.lore> [--dump-ast]\n"
              << "      Parse and validate a LORE file without generating output\n"
              << "  build <source.lore> --target <android-apk|so> --out <dir>\n"
              << "      Compile a LORE source file to an Android APK or host shared library\n"
              << "  --version, -v\n"
              << "      Display LORE compiler version\n"
              << "  --help, -h\n"
              << "      Display this help message\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    if (command == "--version" || command == "-v") {
        print_version();
        return 0;
    }

    if (command == "--help" || command == "-h") {
        print_usage(argv[0]);
        return 0;
    }

    if (command == "check") {
        if (argc < 3) {
            std::cerr << "Error: 'check' command requires a .lore input file.\n";
            return 1;
        }
        std::string input_file = argv[2];
        bool dump_ast_flag = false;
        for (int i = 3; i < argc; ++i) {
            if (std::string(argv[i]) == "--dump-ast") {
                dump_ast_flag = true;
            }
        }

        try {
            std::string source = lore::read_file_to_string(input_file);
            lore::Lexer lexer(input_file, source);
            auto tokens = lexer.tokenize();

            lore::Parser parser(input_file, tokens);
            auto ast = parser.parse_program();

            std::string base_dir = ".";
            size_t slash = input_file.find_last_of("/\\");
            if (slash != std::string::npos) {
                base_dir = input_file.substr(0, slash);
            }

            lore::Checker checker(base_dir);
            checker.check_program(ast);

            for (const auto& w : checker.warnings()) {
                std::cout << "[LORE Warning] " << w << "\n";
            }

            if (dump_ast_flag && ast) {
                ast->dump(std::cout);
            }

            std::cout << "[LORE] Syntax and semantic check passed: " << input_file << "\n";
            return 0;
        } catch (const lore::CompileError& e) {
            std::cerr << "Error at " << e.what() << "\n";
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    if (command == "build") {
        if (argc < 3) {
            std::cerr << "Error: 'build' command requires a .lore input file.\n";
            return 1;
        }
        std::string input_file = argv[2];
        std::string target = "android-apk";
        std::string out_dir = "build";

        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--target" && i + 1 < argc) {
                target = argv[++i];
            } else if (arg == "--out" && i + 1 < argc) {
                out_dir = argv[++i];
            }
        }

        try {
            std::string source = lore::read_file_to_string(input_file);
            lore::Lexer lexer(input_file, source);
            auto tokens = lexer.tokenize();

            lore::Parser parser(input_file, tokens);
            auto ast = parser.parse_program();

            std::cout << "[LORE] Compiling " << input_file << " -> target: " << target << " (out: " << out_dir << ")\n";
            return 0;
        } catch (const lore::CompileError& e) {
            std::cerr << "Error at " << e.what() << "\n";
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    std::cerr << "Error: Unknown command '" << command << "'. Run with --help for usage.\n";
    return 1;
}
