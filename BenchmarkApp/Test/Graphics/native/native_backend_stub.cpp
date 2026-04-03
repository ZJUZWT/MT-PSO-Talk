#include "Graphics/native/native_backend_stub.h"

#include "Graphics/common/timing_scope.h"

namespace benchmark {

GraphicsResult StubNativeBackend::run_compile_case(
    const std::string& workload_tier) {
    GraphicsResult result{};
    result.platform = "StubPlatform";
    result.api = "Stub";
    result.driver_mode = "Native";
    result.case_name = workload_tier;

    {
        TimingScope scope(result.total_us);
    }

    return result;
}

GraphicsBackend& make_stub_native_backend() {
    static StubNativeBackend backend;
    return backend;
}

}  // namespace benchmark
