cmake_minimum_required(VERSION 3.10)
project(Compiler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Source files (excluding main.c for library)
set(LIB_SOURCES
        src/lexer/lexer.c
        src/parser/parser.c
        src/errorHandling/errorHandling.c
        src/parser/parserHelpers.c
        src/semantic/typeChecker.c
        src/semantic/typeChecker.h
        src/semantic/symbolTable.c
        src/semantic/symbolTable.h
        src/codeGeneration/codeGeneration.c
        src/codeGeneration/codeGeneration.h
        src/codeGeneration/asmEmitter.c
        src/codeGeneration/stringHandling.c
        src/codeGeneration/helpers.c
        src/codeGeneration/asmTemplate.h
        src/semantic/builtIns.c
        src/semantic/builtIns.h
        src/semantic/semanticHelpers.c
        src/semantic/semanticHelpers.h
)

# Create compiler library
add_library(compiler_lib ${LIB_SOURCES})

target_include_directories(compiler_lib PUBLIC
        src/lexer
        src/parser
        src/errorHandling
        src/semantic
)

# Main executable
add_executable(compiler src/main.c)
target_link_libraries(compiler compiler_lib)

# Build runtime library
add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/runtime.o
        COMMAND as --64 -o ${CMAKE_BINARY_DIR}/runtime.o ${CMAKE_SOURCE_DIR}/runtime/runtime.s
        DEPENDS ${CMAKE_SOURCE_DIR}/runtime/runtime.s
        COMMENT "Assembling runtime library"
)

add_custom_target(runtime DEPENDS ${CMAKE_BINARY_DIR}/runtime.o)

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
            src/semantic
    )

    enable_testing()
    add_test(NAME compiler_tests COMMAND test_runner)
else()
    message(WARNING "Unity not found at test/unity/src/unity.c - tests disabled")
endif()

add_custom_target(run_program
        COMMAND ${CMAKE_BINARY_DIR}/compiler
        COMMAND as --64 -o ${CMAKE_BINARY_DIR}/program.o ${CMAKE_SOURCE_DIR}/output.s
        COMMAND ld -o ${CMAKE_BINARY_DIR}/program ${CMAKE_BINARY_DIR}/program.o ${CMAKE_BINARY_DIR}/runtime.o
        COMMAND ${CMAKE_BINARY_DIR}/program && echo
        DEPENDS compiler runtime
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Compile, link and run generated program"
)

find_package(Doxygen REQUIRED)

if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
else()
    message(WARNING "Doxygen not found, documentation will not be generated")
endif()
