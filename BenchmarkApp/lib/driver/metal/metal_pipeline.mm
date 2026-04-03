#include "driver/metal/metal_pipeline.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
#import <Metal/Metal.h>
#include "driver/common/timing_scope.h"
#endif

namespace benchmark {

#if defined(__APPLE__) && TARGET_OS_IPHONE

MetalCompileResult compile_metal_pipeline(void* device_ptr, const std::string& msl_source) {
    MetalCompileResult result;

    id<MTLDevice> device = (__bridge id<MTLDevice>)device_ptr;
    if (!device) {
        result.status = "failed_no_device";
        return result;
    }

    NSString* source = [NSString stringWithUTF8String:msl_source.c_str()];
    NSError* error = nil;

    // 1. Compile library from MSL source
    id<MTLLibrary> library = nil;
    {
        TimingScope scope(result.create_library_us);
        library = [device newLibraryWithSource:source options:nil error:&error];
    }
    if (!library) {
        result.status = "failed_library";
        if (error) {
            result.status += ": ";
            result.status += [[error localizedDescription] UTF8String];
        }
        return result;
    }

    // 2. Get vertex and fragment functions
    id<MTLFunction> vert_fn = nil;
    id<MTLFunction> frag_fn = nil;
    {
        TimingScope scope(result.create_functions_us);
        vert_fn = [library newFunctionWithName:@"vertex_main"];
        frag_fn = [library newFunctionWithName:@"fragment_main"];
    }
    if (!vert_fn || !frag_fn) {
        result.status = "failed_functions";
        return result;
    }

    // 3. Create render pipeline state
    {
        MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
        desc.vertexFunction = vert_fn;
        desc.fragmentFunction = frag_fn;
        desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

        TimingScope scope(result.create_pipeline_us);
        id<MTLRenderPipelineState> pso = [device newRenderPipelineStateWithDescriptor:desc error:&error];
        if (!pso) {
            result.status = "failed_pipeline";
            if (error) {
                result.status += ": ";
                result.status += [[error localizedDescription] UTF8String];
            }
        }
    }

    result.total_us = result.create_library_us + result.create_functions_us + result.create_pipeline_us;
    return result;
}

#else

// Stub for non-iOS
MetalCompileResult compile_metal_pipeline(void* /*device*/, const std::string& /*msl_source*/) {
    MetalCompileResult result;
    result.status = "unavailable";
    return result;
}

#endif

}  // namespace benchmark
