# To run the code:
#  - mkdir build && cd build
#  - cmake ..
#  - make

# cmake_minimum_required(VERSION 2.6)
cmake_minimum_required(VERSION 3.13.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(rsfq_analyze VERSION 0.1)
# project(rsfq_analyze)                                  # creates ${PROJECT_NAME}

# target_compile_features(rsfq_analyze PUBLIC cxx_std_17)

set(SOURCES
  src/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Ensures that the header files of the project is included
target_include_directories(${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)
