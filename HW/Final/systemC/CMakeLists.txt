cmake_minimum_required(VERSION 3.8)

# Define the project name of these source code
project (final)

# Define the C++ standard -std=c++17
# This is required to match the systemc installed with Ubuntu
set (CMAKE_CXX_STANDARD 17)

# Add optimization & warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

# Set clock period = 1 ns
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D CLOCK_PERIOD=1.0")

# Define the executable name and its source code
file(GLOB SRC "*.cpp")
add_executable(final ${SRC})

# Define the used libraries of the executable, Equal to -l flags of g++
# In here, the libraries are given in apsolute path. hence no needs for -L
# flag
target_link_libraries(final systemc)

# Run 
set(TRAIN_FILE "train.txt")
set(QUERY_FILE "query.txt")
add_custom_target(run
    COMMAND ./final ../data/${TRAIN_FILE} ../data/${QUERY_FILE}
    DEPENDS final
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
