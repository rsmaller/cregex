set(CREGEX_PROJECT_VERSION "1.0")
set(CREGEX_PROJECT_REVISION "0")
cmake_minimum_required(VERSION 3.11.0)
project(exampleregex VERSION ${CREGEX_PROJECT_VERSION})
project(cregex VERSION ${CREGEX_PROJECT_VERSION})
add_executable(exampleregex example/example.c)
target_include_directories(exampleregex PRIVATE ./include)

IF (MSVC)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4")
ELSE()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -Werror -Wcast-qual -Wconversion -std=c11")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -Werror -Wcast-qual -Wconversion -std=c11")
ENDIF()

IF (Link STREQUAL "static")
    add_library(cregex STATIC ${CMAKE_SOURCE_DIR}/src/cregex_impl.c)
    target_compile_definitions(cregex PRIVATE CREGEX_STATIC)
ELSE()
    add_library(cregex SHARED ${CMAKE_SOURCE_DIR}/src/cregex_impl.c
            include/signal_debug.h)
ENDIF()


target_include_directories(cregex PRIVATE ./include)

IF (Sanitize STREQUAL "address")
    IF (MSVC)
        target_compile_options(exampleregex PRIVATE /fsanitize=address)
    ELSE()
        target_compile_options(exampleregex PRIVATE -g -fsanitize=address -fno-omit-frame-pointer -O0)
        target_link_options(exampleregex PRIVATE -fsanitize=address)
    ENDIF()
ENDIF()

target_link_libraries(exampleregex cregex)

# CMake Package Generator

install(TARGETS cregex
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

install(DIRECTORY include/
        DESTINATION include/cregex/
)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ryan Smaller")
set(CPACK_PACKAGE_NAME "libcregex")
set(CPACK_PACKAGE_VERSION ${CREGEX_PROJECT_VERSION}-${CREGEX_PROJECT_REVISION})
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_amd64)
set(CPACK_PACKAGE_DESCRIPTION "A lightweight regex compiler and API written in C.")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
set_target_properties(cregex PROPERTIES
        VERSION ${CREGEX_PROJECT_VERSION}
        SOVERSION 0
)

include(CPack)

# Unit Tests

find_package(Python3)
add_custom_target(unittest COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/unittest.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        USES_TERMINAL)