cmake_minimum_required (VERSION 3.0.2)
project("Isin Izleme - Raytracing")

#find_package(OpenGL REQUIRED)
find_package (Threads REQUIRED)

set (CMAKE_BUILD_TYPE "Debug")
set (CMAKE_CXX_FLAGS "-std=c++17")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -ggdb -Wall -Wextra")

include_directories(
    "./include/"
)
add_library(
    AssimpLib 
    SHARED 
    IMPORTED
)
set_property(
    TARGET 
    AssimpLib 
    PROPERTY IMPORTED_LOCATION 
    "/media/kaan/Data7510/GitProjects/assimp/build/bin/libassimp.so"
)
add_executable(final.out "src/final.cpp")
target_link_libraries(final.out AssimpLib)
install(TARGETS final.out DESTINATION "${PROJECT_SOURCE_DIR}/bin/")
