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
    add_library(app SHARED
        runtime/lore_runtime.cpp
        runtime/lore_native_activity.cpp
        runtime/native_app_glue/android_native_app_glue.c
    )
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
    add_library(app SHARED
        runtime/lore_runtime.cpp
    )
    target_include_directories(app PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    )
endif()

# -----------------------------------------------------------------------------
# Unit Tests
# -----------------------------------------------------------------------------
enable_testing()

file(GLOB TEST_SOURCES "tests/*.cpp")
if(TEST_SOURCES)
    add_executable(lore_tests ${TEST_SOURCES})
    target_include_directories(lore_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/compiler
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    )
    target_link_libraries(lore_tests PRIVATE lore_compiler_lib)
    add_test(NAME lore_tests COMMAND lore_tests)
endif()
