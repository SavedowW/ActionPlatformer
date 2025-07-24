cmake_minimum_required (VERSION 3.20)

#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
#endif()

#set(CMAKE_CXX_FLAGS "-Wall -EHsc")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

project (ProjectRoot)

add_subdirectory(external)
add_subdirectory (src)
