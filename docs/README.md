# LORE Programming Language

LORE is an English-like programming language for high-performance native user interfaces and logic on Android and embedded systems.

## Features

- **Controlled English-like Syntax**: Clear, declarative statements ending with `.` that read like natural language.
- **Pure Native Execution**: Compiles `.lore` to native C++17 and machine code (`.so`) via Clang and the Android NDK.
- **Zero Java / Kotlin Overhead**: Runs directly inside `NativeActivity` (`hasCode="false"`) with hardware-accelerated OpenGL ES 3.0.
- **Zero-Gradle Packaging**: Produces signed Android APKs using direct toolchain calls (`aapt2`, `zipalign`, `apksigner`).
- **Embeddable Host Mode**: Generates an embeddable `.so` exporting a clean C ABI (`lore_init`, `lore_input`, `lore_stage`, `lore_render`).
- **Texture & Font Pipeline**: Fast asset handling via vendored public-domain `stb_image` and `stb_truetype`.

## Repository Layout

```
lore/
  spec/             # Official grammar specification (LORE_grammar.md) & reference source (counter.lore)
  docs/             # QUESTIONS.md, ARCHITECTURE.md, and README.md
  compiler/         # C++17 LORE compiler (lexer, parser, checker, codegen)
  runtime/          # NativeActivity, OpenGL ES 3.0, 2D renderer, font & textures
  android/          # Manifest templates & zero-Gradle packaging tools (package_apk.py, debug.keystore)
  third_party/      # stb_image.h, stb_truetype.h, Roboto-Regular.ttf (OFL)
  examples/         # hello.lore (blue screen), counter.lore (demo), stage_demo.lore (stages)
  tests/            # Comprehensive unit test suites (M1 through M7)
  .github/workflows # GitHub Actions CI building compiler, running tests, & packaging APKs
```

## Quick Start

### 1. Building the LORE Compiler

Build locally in Termux or any standard Linux/macOS host with CMake and C++17:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Verify compiler version:
```bash
./build/lore --version
# Output: LORE Compiler v0.1.0 (English-like UI language)
```

### 2. Checking LORE Source Files

Use `lore check` for fast syntax and semantic validation:

```bash
./build/lore check examples/counter.lore
./build/lore check examples/stage_demo.lore
```

### 3. Building Host Shared Libraries (`.so`)

Compile any `.lore` file into a standalone embeddable `.so`:

```bash
./build/lore build examples/counter.lore --target so --out dist/
```

This exports the standard C ABI:
```c
void lore_init(const char* config);     // Initialize program & permissions
void lore_input(const void* signal);    // Deliver input signal (eachtimevalue : 1)
void lore_stage(int stage);            // Transition stages (0=input, 1=process, 2=output)
void lore_render(void* surface);        // Render frame to canvas/surface
```

### 4. Running Unit Tests

Run all 6 test suites across compiler and runtime:

```bash
ctest --test-dir build --output-on-failure
```

Test suites include:
- `test_parser`: Tokenizer, AST generation, and block terminator checks.
- `test_checker`: Semantic validation for handles, HCCS colors, assets, and types.
- `test_codegen`: C++ code generation targeting the runtime API.
- `test_runtime_m5`: Text rendering, string interpolation (`{count}`), and touch dispatch.
- `test_images_m6`: Image textures, button state switching (`image:pressed`), fit modes, and tints.
- `test_stages_m7`: Permission flags, display stages (`Input`, `Process`, `Output`), and headless mode.

## Example Applications

### 1. Interactive Counter (`examples/counter.lore`)
The flagship LORE demo application showcasing:
- `InputHarness : true and eachtimevalue : 1.`
- `ShowDisplay : true.`
- Hardcoded Colour Shortcuts (HCCS) with `#4287f5` as `"blue"`.
- Root background texture with `assets/wall.png` and `cover` fit.
- Reactive state variable `state count is 0.`.
- Text interpolation `title text is "Taps: {count}".`.
- Image buttons with dynamic pressed states (`tapBtn image:pressed is "assets/btn_down.png".`).
- Tinted reset button with `resetBtn image-tint is CS:red.`.
- Event logic with conditional reset:
  ```lore
  when tapBtn is tapped
    count is count plus 1.
    if count is greater than 99 then
      count is 0.
    EndOfIf.
  EndOfWhen.
  ```

### 2. Stage Driven Pipeline (`examples/stage_demo.lore`)
Demonstrates time-sequenced stage displays:
- `OutputDisplay : true.` (display shown only at specific stages)
- `show display at output.`
- Inactive during Input and Process stages; automatically renders upon entering Output stage.

### 3. Hello Blue Screen (`examples/hello.lore`)
Minimalist headless or raw screen test displaying a fullscreen blue canvas.

## Android Installation and Running

Artifacts produced by GitHub Actions CI can be installed directly onto any `arm64-v8a` device:

```bash
# Install Counter demo
adb install -r counter-debug.apk
adb shell am start -n com.axilo.counter/android.app.NativeActivity

# Install Stage Demo
adb install -r stage-demo-debug.apk
adb shell am start -n com.axilo.stagedemo/android.app.NativeActivity

# Install Hello Blue Screen
adb install -r hello-debug.apk
adb shell am start -n com.axilo.hello/android.app.NativeActivity
```

View native logs in real time:
```bash
adb logcat -s LORE:V LORE-Renderer:V
```
