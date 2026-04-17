set(IOS_BUNDLE_IDENTIFIER "com.pso.benchmark" CACHE STRING "Bundle identifier for the iOS benchmark app")
set(IOS_DEVELOPMENT_TEAM "" CACHE STRING "Apple development team identifier for signing")
set(IOS_CODE_SIGN_IDENTITY "" CACHE STRING "Optional code signing identity for iOS builds")
set(IOS_PROVISIONING_PROFILE_SPECIFIER "" CACHE STRING "Optional provisioning profile specifier")
set(IOS_CODE_SIGN_STYLE "Automatic" CACHE STRING "Xcode code signing style")

if(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(PSOBenchmarkApp MACOSX_BUNDLE benchmark_app.mm)
    target_link_libraries(PSOBenchmarkApp
        benchmark_orchestrator
        "-framework UIKit"
        "-framework Metal"
        "-framework MetalKit"
    )
    set_target_properties(PSOBenchmarkApp PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "${IOS_BUNDLE_IDENTIFIER}"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${IOS_BUNDLE_IDENTIFIER}"
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "${IOS_CODE_SIGN_STYLE}"
    )

    if(IOS_DEVELOPMENT_TEAM)
        set_property(TARGET PSOBenchmarkApp PROPERTY
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${IOS_DEVELOPMENT_TEAM}")
    endif()

    if(IOS_CODE_SIGN_IDENTITY)
        set_property(TARGET PSOBenchmarkApp PROPERTY
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${IOS_CODE_SIGN_IDENTITY}")
    endif()

    if(IOS_PROVISIONING_PROFILE_SPECIFIER)
        set_property(TARGET PSOBenchmarkApp PROPERTY
            XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "${IOS_PROVISIONING_PROFILE_SPECIFIER}")
    endif()
endif()
