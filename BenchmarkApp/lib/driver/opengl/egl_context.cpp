#include "driver/opengl/egl_context.h"

#if defined(__ANDROID__)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

namespace benchmark {

#if defined(__ANDROID__)

bool EGLHeadlessContext::init() {
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) return false;

    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) return false;

    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(display, config_attribs, &config, 1, &num_configs) || num_configs == 0) {
        eglTerminate(display);
        return false;
    }

    const EGLint pbuffer_attribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };
    EGLSurface surface = eglCreatePbufferSurface(display, config, pbuffer_attribs);
    if (surface == EGL_NO_SURFACE) {
        eglTerminate(display);
        return false;
    }

    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT) {
        eglDestroySurface(display, surface);
        eglTerminate(display);
        return false;
    }

    display_ = reinterpret_cast<void*>(display);
    surface_ = reinterpret_cast<void*>(surface);
    context_ = reinterpret_cast<void*>(context);

    make_current();
    available_ = true;
    return true;
}

void EGLHeadlessContext::make_current() {
    if (display_ && context_) {
        eglMakeCurrent(
            reinterpret_cast<EGLDisplay>(display_),
            reinterpret_cast<EGLSurface>(surface_),
            reinterpret_cast<EGLSurface>(surface_),
            reinterpret_cast<EGLContext>(context_));
    }
}

void EGLHeadlessContext::destroy() {
    if (display_) {
        EGLDisplay display = reinterpret_cast<EGLDisplay>(display_);
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_) {
            eglDestroyContext(display, reinterpret_cast<EGLContext>(context_));
            context_ = nullptr;
        }
        if (surface_) {
            eglDestroySurface(display, reinterpret_cast<EGLSurface>(surface_));
            surface_ = nullptr;
        }
        eglTerminate(display);
        display_ = nullptr;
    }
    available_ = false;
}

#else

// Stub for non-Android platforms
bool EGLHeadlessContext::init() {
    available_ = false;
    return false;
}

void EGLHeadlessContext::make_current() {}

void EGLHeadlessContext::destroy() {
    available_ = false;
}

#endif

EGLHeadlessContext::~EGLHeadlessContext() {
    destroy();
}

bool EGLHeadlessContext::is_available() const {
    return available_;
}

}  // namespace benchmark
