file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-acc-hm
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-acc-hm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(platform-acc-hm systemc)

add_executable(riscv-vp-acc-hm
        main.cpp)

target_link_libraries(riscv-vp-acc-hm rv32 platform-acc-hm platform-common gdb-mc ${Boost_LIBRARIES} systemc pthread)

INSTALL(TARGETS riscv-vp-acc-hm RUNTIME DESTINATION bin)
