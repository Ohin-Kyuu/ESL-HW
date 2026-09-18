cmake_minimum_required(VERSION 3.8)

# Define the project name of these source code
project (p1)

# Define the C++ standard -std=c++17
# This is required to match the systemc installed with Ubuntu
set (CMAKE_CXX_STANDARD 17)

# Add optimization & warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

# Set clock period = 1 ns
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D CLOCK_PERIOD=1.0")

# Define the executable name and its source code
file(GLOB SRC "*.cpp")
add_executable(p1 ${SRC})

# Define the used libraries of the executable, Equal to -l flags of g++
# In here, the libraries are given in apsolute path. hence no needs for -L
# flag
target_link_libraries(p1 systemc)

# Add test sobel filter command
set(OUT_SIG "out_signal_c.dat")
add_custom_target(signal
  COMMAND ./p1 ../signal_c.dat ../${OUT_SIG}
	DEPENDS p1
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}	
)

