cmake_minimum_required (VERSION 3.20)

option(BUILD_SHARED_LIBS "Build libraries as shared by default" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

#if(NOT CMAKE_BUILD_TYPE)
#set(CMAKE_BUILD_TYPE Release)
#endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wunreachable-code-aggressive -Wpedantic \
-Waddress -Waddress-of-temporary -Wabstract-final-class -Wmissing-braces \
-Wdeprecated -Wno-logical-op-parentheses -Wno-deprecated-literal-operator")
#set(CMAKE_CXX_FLAGS "-W1 -EHsc")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

project (ProjectRoot)

add_subdirectory(external)
add_subdirectory (src)
