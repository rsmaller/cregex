set(CREGEX_PROJECT_VERSION "2.0")
set(CREGEX_PROJECT_REVISION "0")
cmake_minimum_required(VERSION 3.11.0)
project(exampleregex VERSION ${CREGEX_PROJECT_VERSION})
project(cregex VERSION ${CREGEX_PROJECT_VERSION})
add_executable(exampleregex example/example.c)
target_include_directories(exampleregex PRIVATE ./include)

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    IF (MSVC)
        set(CMAKE_C_FLAGS_DEBUG  "${CMAKE_C_FLAGS_DEBUG} /W4")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4")
    ELSE()
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -Werror -Wcast-qual -Wconversion -std=c99")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -Werror -Wcast-qual -Wconversion -std=c99")
    ENDIF()
ENDIF()

IF (CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT SANITIZE STREQUAL "address")
    IF (MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2")
    ELSE()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c99")
    ENDIF()
ENDIF()

IF (LINK STREQUAL "Static")
    add_library(cregex STATIC ${CMAKE_SOURCE_DIR}/src/cregex_impl.c
            include/cregex.h include/cregex_impl.h)
    target_compile_definitions(cregex PRIVATE CREGEX_STATIC)
ELSE()
    add_library(cregex SHARED ${CMAKE_SOURCE_DIR}/src/cregex_impl.c
            include/cregex.h include/cregex_impl.h)
ENDIF()

IF (SANITIZE STREQUAL "Address")
    IF (MSVC)
        target_compile_options(exampleregex PRIVATE /fsanitize=address)
    ELSE()
        target_compile_options(exampleregex PRIVATE -g -fsanitize=address -fno-omit-frame-pointer -O0)
        target_compile_options(cregex PRIVATE -g -fsanitize=address -fno-omit-frame-pointer -O0)
        target_link_options(exampleregex PRIVATE -fsanitize=address)
        target_link_options(cregex PRIVATE -fsanitize=address)
    ENDIF()
ENDIF()

IF (EXAMPLE_TYPE STREQUAL "Single")
ELSEIF(EXAMPLE_TYPE STREQUAL "Multi")
     target_compile_definitions(exampleregex PRIVATE CREGEX_EXAMPLE_MULTI_MATCH)
ENDIF()

target_include_directories(cregex PRIVATE ./include)
target_link_libraries(exampleregex cregex)

# CMake Package Generator
set(CMAKE_INSTALL_PREFIX "/usr/local/")
install(TARGETS cregex
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)
install(DIRECTORY include/
        DESTINATION include/cregex/
)

set(CPACK_PACKAGE_MAINTAINER "Ryan Smaller")
set(CPACK_PACKAGE_NAME "libcregex")
set(CPACK_PACKAGE_VERSION ${CREGEX_PROJECT_VERSION}-${CREGEX_PROJECT_REVISION})
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "A lightweight regex engine library written in C.")
set(CPACK_PACKAGE_DEPENDS "libc6")
set_target_properties(cregex PROPERTIES
        VERSION ${CREGEX_PROJECT_VERSION}
        SOVERSION 0
)
include(CPack)

# Unit Tests
find_package(Python3)
add_custom_target(unittest COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/unittest.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} USES_TERMINAL)
