#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#define LORE_VERSION "0.1.0"

void print_version() {
    std::cout << "lore version " << LORE_VERSION << " (C++17)" << std::endl;
}

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " <command> [options]\n\n"
              << "Commands:\n"
              << "  build <source.lore> --target <android-apk|so> --out <dir>\n"
              << "      Compile a LORE source file to an Android APK or host shared library\n"
              << "  check <source.lore> [--dump-ast]\n"
              << "      Parse and validate a LORE file without generating output\n"
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
        std::cout << "[LORE] Checking syntax and semantics for: " << input_file << std::endl;
        // In M0, placeholder check:
        std::cout << "[LORE] Syntax check passed (M0 scaffold).\n";
        return 0;
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

        std::cout << "[LORE] Compiling " << input_file << " -> target: " << target << " (out: " << out_dir << ")\n";
        return 0;
    }

    std::cerr << "Error: Unknown command '" << command << "'. Run with --help for usage.\n";
    return 1;
}
