file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-basic-gaussseidel
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-basic-gaussseidel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(riscv-vp-gaussseidel
        main.cpp)

target_link_libraries(riscv-vp-gaussseidel rv32 platform-basic-gaussseidel platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscv-vp RUNTIME DESTINATION bin)
