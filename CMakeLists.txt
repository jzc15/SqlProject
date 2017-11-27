project(sql_project)
cmake_minimum_required(VERSION 2.8.0)

add_subdirectory(json11)
find_package(FLEX)
find_package(BISON)

# 编译选项
add_compile_options(--std=c++11 -g)

# 模块
set(MODULES filesystem recordmanager ddf common Bplustree frontend)
set(NOEMPTY_MODULES "")
foreach(M ${MODULES})
    file(GLOB_RECURSE SOURCES ${M}/*.cpp)
    if(SOURCES)
        add_library(${M} STATIC ${SOURCES})
        target_link_libraries(${M} json11 ${NOEMPTY_MODULES})
        target_include_directories(${M} PUBLIC .)
        target_include_directories(${M} PUBLIC ${MODULES})
        set(NOEMPTY_MODULES ${NOEMPTY_MODULES} ${M})
    endif()
endforeach()

# flex&yacc
BISON_TARGET(Parser frontend/parse.y ${CMAKE_CURRENT_BINARY_DIR}/frontend_parse.cc COMPILE_FLAGS --defines=${CMAKE_CURRENT_BINARY_DIR}/frontend_parse.h)
FLEX_TARGET(Lexer frontend/scan.l ${CMAKE_CURRENT_BINARY_DIR}/frontend_lex.cc COMPILE_FLAGS -i)
target_sources(frontend PUBLIC ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})

file(GLOB TARGETS_CPP src/*.cpp)

foreach(TARGET_PATH ${TARGETS_CPP})
    string(REGEX REPLACE ".*/(.*)" "\\1" TARGET ${TARGET_PATH})
    string(REPLACE ".cpp" "" TARGET ${TARGET})
    message(STATUS TARGET ${TARGET})
    add_executable(${TARGET} ${TARGET_PATH})
    target_include_directories(${TARGET} PUBLIC .)
    target_include_directories(${TARGET} PUBLIC ${MODULES})
    if(NOEMPTY_MODULES)
        target_link_libraries(${TARGET} ${NOEMPTY_MODULES})
    endif()
    target_link_libraries(${TARGET} json11)
endforeach()