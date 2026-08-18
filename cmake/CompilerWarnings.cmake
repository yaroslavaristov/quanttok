# CompilerWarnings.cmake
#
# Sets strict, HFT-style compiler warnings on a given target, and optionally
# wires up clang-tidy as a build-time static analysis pass via CMake's
# CMAKE_CXX_CLANG_TIDY mechanism (not a separate, easy-to-forget CI step —
# if clang-tidy is enabled, every local build runs it too).
#
# Usage:
#   include(cmake/CompilerWarnings.cmake)
#   quanttok_set_warnings(my_target)

function(quanttok_set_warnings target_name)
    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )

    if(QUANTTOK_WARNINGS_AS_ERRORS)
        list(APPEND CLANG_WARNINGS -Werror)
        list(APPEND GCC_WARNINGS -Werror)
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(${target_name} PRIVATE ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target_name} PRIVATE ${GCC_WARNINGS})
    else()
        message(WARNING "quanttok: no strict warning set configured for compiler '${CMAKE_CXX_COMPILER_ID}' — building with default flags only.")
    endif()
endfunction()

# --- Optional clang-tidy integration -----------------------------------
# Enabled via -DQUANTTOK_ENABLE_CLANG_TIDY=ON. Off by default: clang-tidy
# meaningfully slows down every compilation unit, so it should be an
# explicit opt-in for local dev and a dedicated CI job — not a tax on every
# contributor's default `cmake --build .`.
option(QUANTTOK_ENABLE_CLANG_TIDY "Run clang-tidy as part of the build (uses .clang-tidy at repo root)" OFF)

if(QUANTTOK_ENABLE_CLANG_TIDY)
    find_program(QUANTTOK_CLANG_TIDY_EXE NAMES "clang-tidy")
    if(QUANTTOK_CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${QUANTTOK_CLANG_TIDY_EXE}")
        message(STATUS "quanttok: clang-tidy enabled (${QUANTTOK_CLANG_TIDY_EXE})")
    else()
        message(WARNING "quanttok: QUANTTOK_ENABLE_CLANG_TIDY is ON but clang-tidy was not found on PATH — skipping.")
    endif()
endif()
