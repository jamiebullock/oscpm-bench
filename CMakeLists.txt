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

# liblo is built by its own CMake project so that lo_server, which the address
# space comparison dispatches through, is available. Its options are plain
# variables here, which needs CMP0077.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(WITH_STATIC ON)
set(WITH_TOOLS OFF)
set(WITH_TESTS OFF)
set(WITH_EXAMPLES OFF)
set(WITH_CPP_TESTS OFF)
FetchContent_Declare(
    liblo
    GIT_REPOSITORY https://github.com/radarsat1/liblo.git
    GIT_TAG 025f277275e6b81032a72dfb0b131adab80363e6 # 0.36
    SOURCE_SUBDIR cmake
    SYSTEM
)

# The remaining comparators are compiled from single source files below; a
# SOURCE_SUBDIR without a CMakeLists.txt keeps their own build systems out of
# this one.
FetchContent_Declare(
    libo
    GIT_REPOSITORY https://github.com/CNMAT/libo.git
    GIT_TAG 874edd2ab76a1931a2d8efb81196c07aa346f650 # master, 2023-07-16
    SOURCE_SUBDIR no-cmake
)
FetchContent_Declare(
    cnmat_osc
    GIT_REPOSITORY https://github.com/CNMAT/OSC.git
    GIT_TAG d0a965d42b1d9bb0b7dad3cee7ec87c24a3602e2 # 4.0.0
    SOURCE_SUBDIR no-cmake
)
FetchContent_Declare(
    rtosc
    GIT_REPOSITORY https://github.com/fundamental/rtosc.git
    GIT_TAG f3882fedab283e028514d40415ab64ea48d4b3a1 # v0.3.1
    SOURCE_SUBDIR no-cmake
)
# oscpkt is published as a release archive on its author's site, which has no
# HTTPS and no second host carrying the same archive.
FetchContent_Declare(
    oscpkt
    URL http://gruntthepeon.free.fr/oscpkt/oscpkt-1.2.tar.gz
    URL_HASH SHA256=7adf8b827eec90c9806adbd4a880b0573fe355e813e618e0041ac8bb2a29e6fa
    SOURCE_SUBDIR no-cmake
)

FetchContent_MakeAvailable(oscpm benchmark liblo libo cnmat_osc rtosc oscpkt)

set(comparatorWarnings $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)

add_library(libo_match STATIC ${libo_SOURCE_DIR}/osc_match.c)
target_compile_options(libo_match PRIVATE ${comparatorWarnings})

# libo and CNMAT/OSC both define osc_match; the CNMAT/OSC copy is renamed.
add_library(cnmat_osc_match STATIC ${cnmat_osc_SOURCE_DIR}/OSCMatch.c)
target_compile_definitions(cnmat_osc_match PRIVATE osc_match=cnmat_osc_match)
target_compile_options(cnmat_osc_match PRIVATE ${comparatorWarnings})

add_library(rtosc_ports STATIC
    ${rtosc_SOURCE_DIR}/src/arg-val-cmp.c
    ${rtosc_SOURCE_DIR}/src/arg-val-math.c
    ${rtosc_SOURCE_DIR}/src/dispatch.c
    ${rtosc_SOURCE_DIR}/src/pretty-format.c
    ${rtosc_SOURCE_DIR}/src/rtosc-time.c
    ${rtosc_SOURCE_DIR}/src/rtosc.c
    ${rtosc_SOURCE_DIR}/src/util.c
    ${rtosc_SOURCE_DIR}/src/cpp/ports.cpp
    ${rtosc_SOURCE_DIR}/src/cpp/ports-runtime.cpp
)
target_include_directories(rtosc_ports SYSTEM PUBLIC ${rtosc_SOURCE_DIR}/include)
target_compile_options(rtosc_ports PRIVATE ${comparatorWarnings})

add_library(oscpkt_header INTERFACE)
target_include_directories(oscpkt_header SYSTEM INTERFACE ${oscpkt_SOURCE_DIR})

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
    src/cnmat_osc_matcher.cpp
    src/liblo_matcher.cpp
    src/liblo_space.cpp
    src/libo_matcher.cpp
    src/main.cpp
    src/oscpkt_matcher.cpp
    src/oscpm_matcher.cpp
    src/oscpm_space.cpp
    src/pairs.cpp
    src/rtosc_matcher.cpp
    src/rtosc_space.cpp
    src/space.cpp
)
if(OSCPM_BENCH_JUCE)
    list(APPEND compareSources src/juce_matcher.cpp)
endif()

add_executable(oscpm_compare ${compareSources})

target_link_libraries(oscpm_compare PRIVATE
    oscpm::oscpm
    benchmark::benchmark
    liblo_static
    libo_match
    cnmat_osc_match
    rtosc_ports
    oscpkt_header
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
