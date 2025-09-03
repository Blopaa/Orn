cmake_minimum_required(VERSION 3.10)
project(Compiler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Source files (excluding main.c for library)
set(LIB_SOURCES
        src/lexer/lexer.c
        src/parser/parser.c
        src/errorHandling/errorHandling.c
)

# Create compiler library
add_library(compiler_lib ${LIB_SOURCES})

target_include_directories(compiler_lib PUBLIC
        src/lexer
        src/parser
        src/errorHandling
)

# Main executable
add_executable(compiler src/main.c)
target_link_libraries(compiler compiler_lib)

# Unity testing framework
if(EXISTS "${CMAKE_SOURCE_DIR}/test/unity/src/unity.c")
    message(STATUS "Unity found - building tests")

    # Unity library
    add_library(unity test/unity/src/unity.c)
    target_include_directories(unity PUBLIC test/unity/src)

    # Test executable
    add_executable(test_runner test/test_main.c)
    target_link_libraries(test_runner compiler_lib unity)


    target_include_directories(test_runner PRIVATE
            test/unity/src
            src
            src/lexer
            src/parser
            src/errorHandling
    )

    enable_testing()
    add_test(NAME compiler_tests COMMAND test_runner)
else()
    message(WARNING "Unity not found at test/unity/src/unity.c - tests disabled")
endif()