cmake_minimum_required(VERSION 3.29)
project(Cinterpreter C)

set(CMAKE_C_STANDARD 11)

add_executable(Cinterpreter src/main.c
        src/lexer/lexer.c
        src/lexer/lexer.h
        src/parser/parser.c
        src/parser/parser.h
        src/errorHandling/errorHandling.c
        src/errorHandling/errorHandling.h
        src/testing/testing.c
        src/testing/testing.h)
