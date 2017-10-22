project(sql_project)
cmake_minimum_required(VERSION 2.8.0)

add_subdirectory(json11)

add_compile_options(--std=c++11 -g)

set(MODULES filesystem recordmanager ddf common)
set(NOEMPTY_MODULES "")
foreach(M ${MODULES})
    file(GLOB_RECURSE SOURCES ${M}/*.cpp)
    if(SOURCES)
        add_library(${M} STATIC ${SOURCES})
        target_link_libraries(${M} json11)
        target_include_directories(${M} PUBLIC .)
        target_include_directories(${M} PUBLIC ${MODULES})
        set(NOEMPTY_MODULES ${NOEMPTY_MODULES} ${M})
    endif()
endforeach()

message(STATUS NOEMPTY_MODULES ${NOEMPTY_MODULES})

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