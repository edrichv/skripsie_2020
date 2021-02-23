cmake_minimum_required(VERSION 3.16.3)
project(rsfq_analyzer VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/CliOptions.cpp
    src/LogicTree.cpp
    src/main.cpp
    src/Netlist.cpp
    src/Processing.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)
