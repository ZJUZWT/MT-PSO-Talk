# Mac benchmark tool — Mesa Lavapipe Vulkan + Metal PSO compilation
add_executable(mac_benchmark
    mac_benchmark.cpp
    mac_benchmark_metal.mm
)

target_include_directories(mac_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/third_party/mesa/include
)

target_link_libraries(mac_benchmark PRIVATE
    "-framework Metal"
    "-framework Foundation"
)

# Ensure the .mm file is compiled as ObjC++
set_source_files_properties(mac_benchmark_metal.mm PROPERTIES
    LANGUAGE OBJCXX
    COMPILE_FLAGS "-fobjc-arc"
)
