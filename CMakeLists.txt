cmake_minimum_required (VERSION 3.20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src")

#if(NOT CMAKE_BUILD_TYPE)
#set(CMAKE_BUILD_TYPE Release)
#endif()

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wunreachable-code-aggressive -Wpedantic \
#-Waddress -Waddress-of-temporary -Wabstract-final-class -Wmissing-braces \
#-Wdeprecated -Wno-logical-op-parentheses")
set(CMAKE_CXX_FLAGS "-W1 -EHsc")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

project (ProjectRoot)

add_subdirectory(external)
add_subdirectory (src)
