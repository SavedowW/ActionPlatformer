cmake_minimum_required (VERSION 3.20)

option(BUILD_SHARED_LIBS "Build libraries as shared by default" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

#if(NOT CMAKE_BUILD_TYPE)
#set(CMAKE_BUILD_TYPE Release)
#endif()

project (ProjectRoot)

message("compiler: ${CMAKE_CXX_COMPILER_ID}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wunreachable-code-aggressive -Wpedantic \
        -Waddress -Waddress-of-temporary -Wabstract-final-class -Wmissing-braces \
        -Wdeprecated -Wno-logical-op-parentheses -Wno-deprecated-literal-operator")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/W1 /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else ()
    message( FATAL_ERROR "Unsupported compiler" )
endif()

add_subdirectory(external)
add_subdirectory (src)
