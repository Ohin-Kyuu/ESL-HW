file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-medsch-acc
ethernet.cpp
display.cpp
# Add
Target.cpp
${HEADERS})

target_include_directories(platform-medsch-acc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(platform-medsch-acc systemc)

add_executable(riscv-vp-medsch-acc
        main.cpp)

target_link_libraries(riscv-vp-medsch-acc rv32 platform-medsch-acc platform-common gdb-mc ${Boost_LIBRARIES} systemc pthread)

INSTALL(TARGETS riscv-vp-medsch-acc RUNTIME DESTINATION bin)
