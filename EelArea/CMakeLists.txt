cmake_minimum_required(VERSION 3.10)
project(Eelproject)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
include_directories(
        mnn/include
        src
)

link_directories(mnn/lib)

add_executable(Eelproject src/main.cpp src/StickDiscern.cpp)
target_link_libraries(Eelproject ${OpenCV_LIBS})
