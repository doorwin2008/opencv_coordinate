cmake_minimum_required(VERSION 3.5)

project(warpPerspective LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(OpenCV 4 REQUIRED )
include_directories(
    ${OpenCV_INCLUDE_DIRS}/include
    )
message(STATUS "-OpenCV_INCLUDE_DIRS--- = ${OpenCV_INCLUDE_DIRS}/include")
# -------------- Compile CUDA for FP16 inference if needed  ------------------#
option(USE_CUDA "Enable CUDA support" ON)
if (NOT APPLE AND USE_CUDA)
    find_package(CUDA REQUIRED)
    include_directories(${CUDA_INCLUDE_DIRS})
    add_definitions(-DUSE_CUDA)
else ()
    set(USE_CUDA OFF)
endif ()
add_executable(OpencvWarp

    main.cpp
)
target_link_libraries(OpencvWarp PRIVATE  ${OpenCV_LIBS} )
