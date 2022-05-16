file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-basic-gaussian
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-basic-gaussian PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(riscv-vp-gaussian
        main.cpp)

target_link_libraries(riscv-vp-gaussian rv32 platform-basic-gaussian platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscv-vp RUNTIME DESTINATION bin)
