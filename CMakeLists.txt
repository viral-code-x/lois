cmake_minimum_required(VERSION 3.16)

project(lois VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(lois
    src/main.c
    src/lexer.c
    src/parser.c
    src/interpreter.c
    src/value.c
)

target_link_libraries(lois PRIVATE m)
