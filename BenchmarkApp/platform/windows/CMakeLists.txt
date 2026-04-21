if(WIN32)
    add_executable(compression_benchmark_cli benchmark_cli.cpp)
    target_link_libraries(compression_benchmark_cli
        PRIVATE
            benchmark_orchestrator
    )
endif()
