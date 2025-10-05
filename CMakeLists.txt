cmake_minimum_required(VERSION 3.10)
project(Compiler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3")

# Source files (excluding main.c for library)
set(LIB_SOURCES
        src/codeGeneration/constants.h
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
        src/errorHandling/errors.c
        src/codeGeneration/codeGenOperations.h
        src/codeGeneration/codeGenOperations.c
        src/codeGeneration/codeGenExpressions.c
        src/codeGeneration/codeGenExpressions.h
        src/codeGeneration/registerHandling.h
        src/codeGeneration/registerHandling.c
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
add_executable(orn src/main.c)
target_link_libraries(orn compiler_lib)

# Build runtime library
add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/runtime.o
        COMMAND as --64 -o ${CMAKE_BINARY_DIR}/runtime.o ${CMAKE_SOURCE_DIR}/runtime/runtime.s
        DEPENDS ${CMAKE_SOURCE_DIR}/runtime/runtime.s
        COMMENT "Assembling runtime library"
)

add_custom_target(runtime DEPENDS ${CMAKE_BINARY_DIR}/runtime.o)

# Create a simple test program
file(WRITE ${CMAKE_SOURCE_DIR}/program.orn
        "int x = 42;\n"
        "string msg = \"Hello from file!\";\n"
        "bool flag = true;\n"
        "print(msg);\n"
        "print(x);\n"
        "print(flag);\n"
)

add_custom_target(run_program
        COMMAND ${CMAKE_BINARY_DIR}/orn ${CMAKE_SOURCE_DIR}/program.orn
        COMMAND as --64 -o ${CMAKE_BINARY_DIR}/program.o ${CMAKE_BINARY_DIR}/output.s
        COMMAND ld -o ${CMAKE_BINARY_DIR}/program ${CMAKE_BINARY_DIR}/program.o ${CMAKE_BINARY_DIR}/runtime.o
        COMMAND ${CMAKE_BINARY_DIR}/program && echo
        DEPENDS orn runtime
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Compile, link and run generated program"
)