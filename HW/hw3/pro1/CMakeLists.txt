cmake_minimum_required(VERSION 3.8)

# Define the project name of these source code
project (pro2)

# Define the C++ standard -std=c++17
# This is required to match the systemc installed with Ubuntu
set (CMAKE_CXX_STANDARD 17)

# Add optimization & warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

# Set clock period = 1 ns
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D CLOCK_PERIOD=1.0")

# Source files
file(GLOB SRC "*.cpp")

# ---- Cameraman: 512 x 512 ----
add_executable(pro2_cam ${SRC})
target_compile_definitions(pro2_cam PRIVATE WIDTH=512 HEIGHT=512)
target_link_libraries(pro2_cam systemc)

# ---- Lena: 256 x 256 ----
add_executable(pro2_lena ${SRC})
target_compile_definitions(pro2_lena PRIVATE WIDTH=256 HEIGHT=256)
target_link_libraries(pro2_lena systemc)

# Add test sobel filter command
set(OUT_CAM "out_cameraman.bmp")
add_custom_target(cam
  COMMAND ./pro2_cam ../img/cameraman.bmp ./${OUT_CAM}
	DEPENDS pro2_cam
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}	
)

set(OUT_LENA "out_lena_color_256.bmp")
add_custom_target(lena
  COMMAND ./pro2_lena ../img/lena_color_256.bmp ./${OUT_LENA}
	DEPENDS pro2_lena
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}	
)

# Compare output & golden
add_custom_target(check_cam
    COMMAND tail -c +1079 ${CMAKE_BINARY_DIR}/${OUT_CAM} > out_cameraman.bin
    COMMAND tail -c +1079 ${CMAKE_SOURCE_DIR}/golden/gold_cameraman.bmp > gold_cameraman.bin
    COMMAND diff -s out_cameraman.bin gold_cameraman.bin
    COMMAND rm out_cameraman.bin gold_cameraman.bin
    DEPENDS cam
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Checking Cameraman output pixels..."
)

add_custom_target(check_lena
    COMMAND tail -c +1079 ${CMAKE_BINARY_DIR}/${OUT_LENA} > out_lena_color_256.bin
    COMMAND tail -c +1079 ${CMAKE_SOURCE_DIR}/golden/gold_lena_color_256.bmp > gold_lena_color_256.bin
    COMMAND diff -s out_lena_color_256.bin gold_lena_color_256.bin
    COMMAND rm out_lena_color_256.bin gold_lena_color_256.bin
    DEPENDS lena
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Checking Lena output pixels..."
)
