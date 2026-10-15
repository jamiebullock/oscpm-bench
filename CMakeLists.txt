# Part of oscpm-bench
#
# SPDX-FileCopyrightText: 2026 Jamie Bullock
# SPDX-License-Identifier: Zlib

cmake_minimum_required(VERSION 3.25)

project(oscpm-bench LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(OSCPM_BENCH_JUCE "Include JUCE's OSCAddressPattern in the comparison" ON)

include(FetchContent)

FetchContent_Declare(
    oscpm
    GIT_REPOSITORY https://github.com/jamiebullock/oscpm.git
    GIT_TAG ff146c10ab1db38a9745a4ac98219ae51f4ff3ec # v0.9.8
)

set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_ENABLE_INSTALL OFF)
set(BENCHMARK_ENABLE_WERROR OFF)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG 192ef10025eb2c4cdd392bc502f0c852196baa48 # v1.9.5
    SYSTEM
)

# The comparators are compiled from single source files below; a SOURCE_SUBDIR
# without a CMakeLists.txt keeps their own build systems out of this one.
FetchContent_Declare(
    liblo
    GIT_REPOSITORY https://github.com/radarsat1/liblo.git
    GIT_TAG 025f277275e6b81032a72dfb0b131adab80363e6 # 0.36
    SOURCE_SUBDIR no-cmake
)
FetchContent_Declare(
    libo
    GIT_REPOSITORY https://github.com/CNMAT/libo.git
    GIT_TAG 874edd2ab76a1931a2d8efb81196c07aa346f650 # master, 2023-07-16
    SOURCE_SUBDIR no-cmake
)
FetchContent_Declare(
    rtosc
    GIT_REPOSITORY https://github.com/fundamental/rtosc.git
    GIT_TAG f3882fedab283e028514d40415ab64ea48d4b3a1 # v0.3.1
    SOURCE_SUBDIR no-cmake
)

FetchContent_MakeAvailable(oscpm benchmark liblo libo rtosc)

set(comparatorWarnings $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)

add_library(liblo_pattern STATIC ${liblo_SOURCE_DIR}/src/pattern_match.c)
target_compile_options(liblo_pattern PRIVATE ${comparatorWarnings})

add_library(libo_match STATIC ${libo_SOURCE_DIR}/osc_match.c)
target_compile_options(libo_match PRIVATE ${comparatorWarnings})

add_library(rtosc_match STATIC
    ${rtosc_SOURCE_DIR}/src/arg-val-math.c
    ${rtosc_SOURCE_DIR}/src/dispatch.c
    ${rtosc_SOURCE_DIR}/src/rtosc.c
)
target_include_directories(rtosc_match SYSTEM PUBLIC ${rtosc_SOURCE_DIR}/include)
target_compile_options(rtosc_match PRIVATE ${comparatorWarnings})

if(OSCPM_BENCH_JUCE)
    set(JUCE_MODULES_ONLY ON)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 3fb5cbc0340758596dac39f7c8de47d562b4896e # 9.0.4
        SYSTEM
    )
    FetchContent_MakeAvailable(juce)
endif()

set(compareSources
    src/liblo_matcher.cpp
    src/libo_matcher.cpp
    src/main.cpp
    src/oscpm_matcher.cpp
    src/pairs.cpp
    src/rtosc_matcher.cpp
)
if(OSCPM_BENCH_JUCE)
    list(APPEND compareSources src/juce_matcher.cpp)
endif()

add_executable(oscpm_compare ${compareSources})

target_link_libraries(oscpm_compare PRIVATE
    oscpm::oscpm
    benchmark::benchmark
    liblo_pattern
    libo_match
    rtosc_match
)

if(OSCPM_BENCH_JUCE)
    target_link_libraries(oscpm_compare PRIVATE juce::juce_osc)
    target_compile_definitions(oscpm_compare PRIVATE
        OSCPM_BENCH_JUCE=1
        JUCE_STANDALONE_APPLICATION=1
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
    )
endif()

target_compile_definitions(oscpm_compare PRIVATE
    OSCPM_COMPARE_COMPILER="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
)

# JUCE adds its module sources to the target that links it, so the warning
# flags go on this project's own sources only.
set_source_files_properties(${compareSources} PROPERTIES COMPILE_OPTIONS
    "$<IF:$<CXX_COMPILER_ID:MSVC>,/W4;/WX;/permissive-;/Zc:__cplusplus,-Wall;-Wextra;-Wpedantic;-Wshadow;-Wconversion;-Werror>"
)
