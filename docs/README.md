# LORE Programming Language

LORE is an English-like programming language for high-performance native user interfaces and logic on Android and embedded systems.

## Features

- **Controlled English-like Syntax**: Clear, declarative statements that read like natural language.
- **Pure Native Execution**: Compiles to native C++17 and machine code (`.so`) via Clang and the Android NDK.
- **Zero Java / Kotlin Overhead**: Runs directly inside `NativeActivity` (`hasCode="false"`) with hardware-accelerated OpenGL ES 3.0.
- **Zero-Gradle Packaging**: Produces signed Android APKs using direct toolchain calls (`aapt2`, `zipalign`, `apksigner`).
- **Embeddable Host Mode**: Generates an embeddable `.so` exporting a clean C ABI (`lore_init`, `lore_input`, `lore_stage`, `lore_render`).

## Repository Layout

```
lore/
  spec/             # Official grammar specification & reference source
  docs/             # Architecture, design decisions, and documentation
  compiler/         # C++17 LORE compiler (lexer, parser, checker, codegen)
  runtime/          # NativeActivity, OpenGL ES 3.0, 2D renderer, font & textures
  android/          # Manifest templates & zero-Gradle packaging tools
  third_party/      # stb_image, stb_truetype, OFL fonts
  examples/         # hello.lore (blue screen), counter.lore (interactive app)
  tests/            # Compiler unit tests and golden files
  .github/workflows # Continuous integration pipeline
```

## Quick Start

### 1. Building the LORE Compiler

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Verify compiler version:
```bash
./build/lore --version
```

### 2. Checking a LORE Source File

```bash
./build/lore check examples/counter.lore
```

### 3. Packaging an Android APK

```bash
./build/lore build examples/counter.lore --target android-apk --out dist/
```

### 4. Running Unit Tests

```bash
ctest --test-dir build --output-on-failure
```
