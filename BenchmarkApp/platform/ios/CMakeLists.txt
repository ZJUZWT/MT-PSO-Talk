if(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(PSOBenchmarkApp MACOSX_BUNDLE benchmark_app.mm)
    target_link_libraries(PSOBenchmarkApp
        benchmark_orchestrator
        "-framework UIKit"
        "-framework Metal"
        "-framework MetalKit"
    )
    set_target_properties(PSOBenchmarkApp PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pso.benchmark"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
endif()
