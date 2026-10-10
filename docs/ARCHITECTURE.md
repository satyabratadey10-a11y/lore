# LORE Architecture and Implementation

LORE is a controlled English-like programming language tailored for native UI and logic, compiling down to native machine code via C++ and Clang.

## 1. System Pipeline

```
                     +--------------------+
                     |    app.lore        |
                     +--------------------+
                               |
                               v
                     +--------------------+
                     |   LORE Compiler    |
                     | (lexer/parser/     |
                     |  checker/codegen)  |
                     +--------------------+
                               |
                               v
                     +--------------------+
                     |  Generated C++17   |
                     +--------------------+
                               |
              +----------------+----------------+
              |                                 |
              v                                 v
     [ Android NDK Clang ]                [ Host Clang ]
     with NativeActivity & GLES3          with C ABI export
              |                                 |
              v                                 v
   lib/arm64-v8a/libapp.so                   libapp.so
              |                        (lore_init, lore_input,
              v                         lore_stage, lore_render)
    [ aapt2 + zipalign +
        apksigner ]
              |
              v
       final app.apk
```

## 2. Directory Layout

- `spec/`: Reference grammar (`LORE_grammar.md`) and reference program (`counter.lore`).
- `docs/`:
  - `QUESTIONS.md`: Track design decisions, ambiguities, and resolutions.
  - `ARCHITECTURE.md`: Architecture specification and milestone tracking.
  - `README.md`: User documentation, installation, and usage instructions.
- `compiler/`:
  - `main.cpp`: CLI interface (`lore check`, `lore build`, `lore --version`).
  - `token.h`: Lexer tokens and source positions.
  - `lexer.h`, `lexer.cpp`: Lexical analyzer for English-like keywords, units, strings, comments.
  - `ast.h`: AST nodes representing program structure.
  - `parser.h`, `parser.cpp`: Recursive descent parser emitting friendly error messages.
  - `checker.h`, `checker.cpp`: Semantic validator (handles, colors, assets, types).
  - `codegen_cpp.h`, `codegen_cpp.cpp`: C++ generator targeting runtime API.
- `runtime/`:
  - `lore_runtime.h`: Core types, runtime functions, and exported C ABI declarations.
  - `lore_runtime.cpp`: State store, color resolution, element tree, event dispatch.
  - `lore_native_activity.cpp`: Android `NativeActivity` lifecycle, EGL + OpenGL ES 3.0 renderer, touch input handling.
  - `native_app_glue/`: Google `android_native_app_glue` integration.
- `android/`:
  - `AndroidManifest.xml.template`: NativeActivity manifest template with `hasCode="false"`.
  - `package_apk.py`: Zero-Gradle APK packaging pipeline (`aapt2` -> zip injection -> `zipalign` -> `apksigner`).
  - `debug.keystore`: Standard debug keystore for development signing.
- `third_party/`:
  - `stb_image.h`: Lightweight image loader.
  - `stb_truetype.h`: Lightweight TrueType font rasterizer.
  - `Roboto-Regular.ttf` & `OFL.txt`: Open Font License typography asset.
- `examples/`:
  - `hello.lore`: Blue screen minimal app.
  - `counter.lore`: Reference counter app with image buttons and state interpolation.
- `tests/`:
  - Unit tests for compiler components and golden file verification.
- `.github/workflows/`:
  - `build.yml`: GitHub Actions CI pipeline building compiler, running tests, compiling APK, and building host `.so`.

## 3. Milestones & Progress

- [x] **M0 Scaffold**: Repo layout, CMake build, NativeActivity APK packager, CI workflow, green test suite.
- [x] **M1 Lexer + Parser**: Full EBNF grammar, AST dump, tests for all rules & friendly errors.
- [x] **M2 Checker**: Semantic checks (handles, CS:name, images, types, warnings).
- [x] **M3 Codegen**: LORE -> C++ against runtime API, compiles for host & NDK.
- [ ] **M4 Blue Screen**: hello.lore -> full-screen blue canvas on Android.
- [ ] **M5 Text + Button + State**: {count} interpolation, tap handling, redraw loop.
- [ ] **M6 Images**: Texture loading, pressed/disabled states, fit modes, tints.
- [ ] **M7 Permissions + Stages**: ShowDisplay, OutputDisplay, InputHarness, stage statements.
- [ ] **M8 Demo App**: Finished counter demo + stage demo, release artifacts, instructions.
