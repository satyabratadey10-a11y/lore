cmake_minimum_required(VERSION 3.10)
project(lore VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# -----------------------------------------------------------------------------
# Compiler Library and Executable
# -----------------------------------------------------------------------------
file(GLOB COMPILER_LIB_SOURCES "compiler/*.cpp")
list(FILTER COMPILER_LIB_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

add_library(lore_compiler_lib STATIC ${COMPILER_LIB_SOURCES})
target_include_directories(lore_compiler_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/compiler)

add_executable(lore compiler/main.cpp)
target_link_libraries(lore PRIVATE lore_compiler_lib)

# -----------------------------------------------------------------------------
# Runtime Shared Library (Host or Android NDK)
# -----------------------------------------------------------------------------
if(DEFINED CMAKE_ANDROID_NDK OR DEFINED ANDROID_NDK OR DEFINED ANDROID_ABI)
    message(STATUS "Configuring for Android NDK target (ABI: ${ANDROID_ABI})")
    set(ANDROID_APP_SOURCES
        runtime/lore_runtime.cpp
        runtime/lore_renderer.cpp
        runtime/lore_native_activity.cpp
        runtime/native_app_glue/android_native_app_glue.c
    )
    if(DEFINED LORE_GENERATED_CPP AND EXISTS "${LORE_GENERATED_CPP}")
        list(APPEND ANDROID_APP_SOURCES "${LORE_GENERATED_CPP}")
    endif()
    add_library(app SHARED ${ANDROID_APP_SOURCES})
    target_compile_definitions(app PRIVATE LORE_TARGET_ANDROID=1)
    target_include_directories(app PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime/native_app_glue
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    )
    target_link_libraries(app
        android
        log
        EGL
        GLESv3
    )
else()
    # Host embeddable library (.so)
    message(STATUS "Configuring for Host target (.so C ABI)")
    set(HOST_APP_SOURCES
        runtime/lore_runtime.cpp
        runtime/lore_renderer.cpp
    )
    if(DEFINED LORE_GENERATED_CPP AND EXISTS "${LORE_GENERATED_CPP}")
        list(APPEND HOST_APP_SOURCES "${LORE_GENERATED_CPP}")
    endif()
    add_library(app SHARED ${HOST_APP_SOURCES})
    target_include_directories(app PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    )
endif()

# -----------------------------------------------------------------------------
# Unit Tests
# -----------------------------------------------------------------------------
enable_testing()

add_executable(test_parser tests/test_lexer_parser.cpp)
target_include_directories(test_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/compiler)
target_link_libraries(test_parser PRIVATE lore_compiler_lib)
add_test(NAME test_parser COMMAND test_parser)

add_executable(test_checker tests/test_checker.cpp)
target_include_directories(test_checker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/compiler)
target_link_libraries(test_checker PRIVATE lore_compiler_lib)
add_test(NAME test_checker COMMAND test_checker)

add_executable(test_codegen tests/test_codegen.cpp)
target_include_directories(test_codegen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/compiler)
target_link_libraries(test_codegen PRIVATE lore_compiler_lib)
add_test(NAME test_codegen COMMAND test_codegen)

add_executable(test_runtime_m5 tests/test_runtime_m5.cpp)
target_include_directories(test_runtime_m5 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_link_libraries(test_runtime_m5 PRIVATE app)
add_test(NAME test_runtime_m5 COMMAND test_runtime_m5)

add_executable(test_images_m6 tests/test_images_m6.cpp)
target_include_directories(test_images_m6 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_link_libraries(test_images_m6 PRIVATE lore_compiler_lib app)
add_test(NAME test_images_m6 COMMAND test_images_m6)

add_executable(test_stages_m7 tests/test_stages_m7.cpp)
target_include_directories(test_stages_m7 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_link_libraries(test_stages_m7 PRIVATE lore_compiler_lib app)
add_test(NAME test_stages_m7 COMMAND test_stages_m7)

