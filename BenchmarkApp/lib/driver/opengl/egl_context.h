#pragma once

namespace benchmark {

class EGLHeadlessContext {
public:
    EGLHeadlessContext() = default;
    ~EGLHeadlessContext();

    bool init();
    void make_current();
    void destroy();
    bool is_available() const;

private:
    bool available_ = false;

#if defined(__ANDROID__)
    void* display_ = nullptr;   // EGLDisplay
    void* surface_ = nullptr;   // EGLSurface
    void* context_ = nullptr;   // EGLContext
#endif
};

}  // namespace benchmark
