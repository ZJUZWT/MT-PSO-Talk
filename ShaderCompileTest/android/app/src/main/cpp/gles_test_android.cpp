// ====================================================================
// gles_test_android.cpp – OpenGL ES 3.2 benchmark with glColorMask
//
// Android version: uses EGL to create an offscreen GLES 3.2 context.
// Measures GPU time using GL_TIME_ELAPSED (EXT_disjoint_timer_query).
// ====================================================================

#include "benchmark.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>

// Fallback definition for older NDK versions
#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR 0x0040
#endif

#include <android/log.h>
#include <stdexcept>
#include <string>
#include <vector>

#define LOGTAG "PSO-Bench-GLES"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGTAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__)

// ── Helper: compile a single shader ────────────────────────────────
static GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOGE("Shader compile error: %s", log);
        glDeleteShader(shader);
        throw std::runtime_error("GLES shader compilation failed");
    }
    return shader;
}

// ── Helper: link a program ─────────────────────────────────────────
static GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        LOGE("Program link error: %s", log);
        glDeleteProgram(prog);
        throw std::runtime_error("GLES program link failed");
    }
    return prog;
}

// ── Grid vertex data (32x32 grid = 1024 quads = 2048 triangles = 6144 vertices)
static constexpr int GRID_SIZE = 32;
static constexpr int GRID_VERTEX_COUNT = GRID_SIZE * GRID_SIZE * 6;

static void generateGridVertices(std::vector<float>& out) {
    out.clear();
    out.reserve(GRID_VERTEX_COUNT * 3);
    float step = 2.0f / GRID_SIZE;
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            float x0 = -1.0f + x * step;
            float y0 = -1.0f + y * step;
            float x1 = x0 + step;
            float y1 = y0 + step;
            out.push_back(x0); out.push_back(y0); out.push_back(0.0f);
            out.push_back(x1); out.push_back(y0); out.push_back(0.0f);
            out.push_back(x0); out.push_back(y1); out.push_back(0.0f);
            out.push_back(x1); out.push_back(y0); out.push_back(0.0f);
            out.push_back(x1); out.push_back(y1); out.push_back(0.0f);
            out.push_back(x0); out.push_back(y1); out.push_back(0.0f);
        }
    }
}
// ── OpenGL ES shader source (ESSL 320) ─────────────────────────────
static const char* kVertSrc = R"(#version 320 es
precision highp float;

layout(location = 0) in vec3 inPos;
out vec4 heavyColor;

uniform int loopCount;
uniform float seed;

void main() {
    gl_Position = vec4(inPos, 1.0);

    vec4 acc = vec4(inPos, 1.0) * seed;

    for (int i = 0; i < loopCount; i++) {
        acc = sin(acc) * cos(acc)
            + exp(acc * 0.01) - log(abs(acc) + 1.0);
    }

    heavyColor = acc;
}
)";

static const char* kFragSrc = R"(#version 320 es
precision highp float;

in vec4 heavyColor;
out vec4 outColor;

void main() {
    outColor = heavyColor;
}
)";

// ====================================================================
// Public entry point
// ====================================================================
BenchmarkResult runGLESBenchmark(int iterations, int loopCount) {
    BenchmarkResult result;
    result.backend    = "OpenGL ES";
    result.testCase   = "glColorMask(0,0,0,0) (runtime state)";
    result.drawCalls  = iterations;
    result.iterations = iterations;
    result.success    = false;
    result.gpuTimeMs  = 0.0;

    // Generate grid vertices
    std::vector<float> gridVertices;
    generateGridVertices(gridVertices);

    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;

    try {
        // ── Create EGL context ─────────────────────────────────────
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            throw std::runtime_error("eglGetDisplay failed");
        }
        eglInitialize(display, nullptr, nullptr);

        // Request OpenGL ES 3.2
        const EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
            EGL_RED_SIZE,        8,
            EGL_GREEN_SIZE,      8,
            EGL_BLUE_SIZE,       8,
            EGL_ALPHA_SIZE,      8,
            EGL_NONE
        };

        EGLConfig config;
        EGLint numConfigs;
        eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
        if (numConfigs == 0) {
            throw std::runtime_error("No suitable EGL config found");
        }

        const EGLint contextAttribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_MINOR_VERSION, 2,
            EGL_NONE
        };
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
        if (context == EGL_NO_CONTEXT) {
            throw std::runtime_error("eglCreateContext failed");
        }

        // Create a small PBuffer surface (offscreen)
        const EGLint pbufferAttribs[] = {
            EGL_WIDTH,  64,
            EGL_HEIGHT, 64,
            EGL_NONE
        };
        surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
        eglMakeCurrent(display, surface, surface, context);

        LOGI("Renderer: %s", glGetString(GL_RENDERER));
        LOGI("Version:  %s", glGetString(GL_VERSION));

        // ── Check for timer query extension ────────────────────────
        // On Android, GL_EXT_disjoint_timer_query is commonly available
        std::string extensions((const char*)glGetString(GL_EXTENSIONS));
        bool hasTimerQuery = (extensions.find("GL_EXT_disjoint_timer_query") != std::string::npos);

        // Try to get function pointers for timer query
        typedef void (*PFNGLGENQUERIESEXTPROC)(GLsizei, GLuint*);
        typedef void (*PFNGLDELETEQUERIESEXTPROC)(GLsizei, const GLuint*);
        typedef void (*PFNGLBEGINQUERYEXTPROC)(GLenum, GLuint);
        typedef void (*PFNGLENDQUERYEXTPROC)(GLenum);
        typedef void (*PFNGLGETQUERYOBJECTUI64VEXTPROC)(GLuint, GLenum, GLuint64*);

        auto glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC)eglGetProcAddress("glGenQueriesEXT");
        auto glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC)eglGetProcAddress("glDeleteQueriesEXT");
        auto glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC)eglGetProcAddress("glBeginQueryEXT");
        auto glEndQueryEXT = (PFNGLENDQUERYEXTPROC)eglGetProcAddress("glEndQueryEXT");
        auto glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)eglGetProcAddress("glGetQueryObjectui64vEXT");

        if (!hasTimerQuery || !glGenQueriesEXT || !glBeginQueryEXT) {
            LOGI("GL_EXT_disjoint_timer_query not available, falling back to glFinish timing");
            hasTimerQuery = false;
        } else {
            LOGI("GL_EXT_disjoint_timer_query available – using GPU timer");
        }

        // ── Compile shaders ────────────────────────────────────────
        GLuint vs   = compileShader(GL_VERTEX_SHADER, kVertSrc);
        GLuint fs   = compileShader(GL_FRAGMENT_SHADER, kFragSrc);
        GLuint prog = linkProgram(vs, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);

        LOGI("Program linked (shader compiled WITHOUT knowledge of ColorMask)");

        // ── Set uniforms (runtime loop count + seed) ───────────────
        glUseProgram(prog);
        GLint locLoop = glGetUniformLocation(prog, "loopCount");
        GLint locSeed = glGetUniformLocation(prog, "seed");
        if (locLoop >= 0) glUniform1i(locLoop, loopCount);
        if (locSeed >= 0) glUniform1f(locSeed, 1.23f);

        // ── Create VAO + VBO ───────────────────────────────────────
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float),
                     gridVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        // ── Create offscreen FBO ───────────────────────────────────
        GLuint fbo, rbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 64, 64);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer incomplete");
        }

        glViewport(0, 0, 64, 64);

        // ── Warm-up pass ───────────────────────────────────────────
        // IMPORTANT: warm-up with colorMask ENABLED (all true)!
        // If we warm up with colorMask=false, the driver may
        // JIT-recompile the shader for that state combination.
        glUseProgram(prog);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
        glFinish();

        // ── Timed pass ─────────────────────────────────────────────
        if (hasTimerQuery) {
            // Use GPU timer query
            GLuint timerQuery;
            glGenQueriesEXT(1, &timerQuery);

            #define GL_TIME_ELAPSED_EXT 0x88BF
            glBeginQueryEXT(GL_TIME_ELAPSED_EXT, timerQuery);

            glUseProgram(prog);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glClear(GL_COLOR_BUFFER_BIT);

            for (int i = 0; i < iterations; i++) {
                glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
            }

            glEndQueryEXT(GL_TIME_ELAPSED_EXT);
            glFinish();

            #define GL_QUERY_RESULT_EXT 0x8866
            GLuint64 elapsed = 0;
            glGetQueryObjectui64vEXT(timerQuery, GL_QUERY_RESULT_EXT, &elapsed);

            result.gpuTimeMs = static_cast<double>(elapsed) / 1e6;
            glDeleteQueriesEXT(1, &timerQuery);
        } else {
            // Fallback: use CPU-side timing with glFinish
            glFinish();

            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            glUseProgram(prog);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glClear(GL_COLOR_BUFFER_BIT);

            for (int i = 0; i < iterations; i++) {
                glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
            }

            glFinish();
            clock_gettime(CLOCK_MONOTONIC, &end);

            double startMs = start.tv_sec * 1000.0 + start.tv_nsec / 1e6;
            double endMs   = end.tv_sec * 1000.0 + end.tv_nsec / 1e6;
            result.gpuTimeMs = endMs - startMs;
        }

        result.success = true;

        // Mark which timing method was used
        if (hasTimerQuery) {
            result.testCase = "glColorMask(0,0,0,0) (GPU timer query)";
        } else {
            result.testCase = "glColorMask(0,0,0,0) (CPU fallback - NOT comparable!)";
        }

        // ── Cleanup ────────────────────────────────────────────────
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(prog);

    } catch (const std::exception& e) {
        LOGE("Error: %s", e.what());
    }

    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (surface != EGL_NO_SURFACE) eglDestroySurface(display, surface);
        if (context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
        eglTerminate(display);
    }

    return result;
}

// ====================================================================
// Heavy shader + glColorMask(TRUE) – forces actual GPU execution
// ====================================================================
BenchmarkResult runGLESBenchmarkColorWrite(int iterations, int loopCount) {
    BenchmarkResult result;
    result.backend    = "OpenGL ES";
    result.testCase   = "glColorMask(1,1,1,1) (heavy shader, full write)";
    result.drawCalls  = iterations;
    result.iterations = iterations;
    result.success    = false;
    result.gpuTimeMs  = 0.0;

    std::vector<float> gridVertices;
    generateGridVertices(gridVertices);

    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;

    try {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            throw std::runtime_error("eglGetDisplay failed");
        }
        eglInitialize(display, nullptr, nullptr);

        const EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
            EGL_RED_SIZE,        8,
            EGL_GREEN_SIZE,      8,
            EGL_BLUE_SIZE,       8,
            EGL_ALPHA_SIZE,      8,
            EGL_NONE
        };

        EGLConfig config;
        EGLint numConfigs;
        eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
        if (numConfigs == 0) {
            throw std::runtime_error("No suitable EGL config found");
        }

        const EGLint contextAttribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_MINOR_VERSION, 2,
            EGL_NONE
        };
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
        if (context == EGL_NO_CONTEXT) {
            throw std::runtime_error("eglCreateContext failed");
        }

        const EGLint pbufferAttribs[] = {
            EGL_WIDTH,  64,
            EGL_HEIGHT, 64,
            EGL_NONE
        };
        surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
        eglMakeCurrent(display, surface, surface, context);

        LOGI("ColorWrite Renderer: %s", glGetString(GL_RENDERER));

        // Check for timer query
        std::string extensions((const char*)glGetString(GL_EXTENSIONS));
        bool hasTimerQuery = (extensions.find("GL_EXT_disjoint_timer_query") != std::string::npos);

        typedef void (*PFNGLGENQUERIESEXTPROC)(GLsizei, GLuint*);
        typedef void (*PFNGLDELETEQUERIESEXTPROC)(GLsizei, const GLuint*);
        typedef void (*PFNGLBEGINQUERYEXTPROC)(GLenum, GLuint);
        typedef void (*PFNGLENDQUERYEXTPROC)(GLenum);
        typedef void (*PFNGLGETQUERYOBJECTUI64VEXTPROC)(GLuint, GLenum, GLuint64*);

        auto glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC)eglGetProcAddress("glGenQueriesEXT");
        auto glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC)eglGetProcAddress("glDeleteQueriesEXT");
        auto glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC)eglGetProcAddress("glBeginQueryEXT");
        auto glEndQueryEXT = (PFNGLENDQUERYEXTPROC)eglGetProcAddress("glEndQueryEXT");
        auto glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)eglGetProcAddress("glGetQueryObjectui64vEXT");

        if (!hasTimerQuery || !glGenQueriesEXT || !glBeginQueryEXT) {
            hasTimerQuery = false;
        }

        // Compile same heavy shaders
        GLuint vs   = compileShader(GL_VERTEX_SHADER, kVertSrc);
        GLuint fs   = compileShader(GL_FRAGMENT_SHADER, kFragSrc);
        GLuint prog = linkProgram(vs, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);

        glUseProgram(prog);
        GLint locLoop = glGetUniformLocation(prog, "loopCount");
        GLint locSeed = glGetUniformLocation(prog, "seed");
        if (locLoop >= 0) glUniform1i(locLoop, loopCount);
        if (locSeed >= 0) glUniform1f(locSeed, 1.23f);

        // Create VAO + VBO
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float),
                     gridVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        // Create offscreen FBO
        GLuint fbo, rbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 64, 64);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer incomplete");
        }

        glViewport(0, 0, 64, 64);

        // Warm-up with colorMask ENABLED
        glUseProgram(prog);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
        glFinish();

        // Timed pass – colorMask ENABLED
        if (hasTimerQuery) {
            GLuint timerQuery;
            glGenQueriesEXT(1, &timerQuery);

            glBeginQueryEXT(GL_TIME_ELAPSED_EXT, timerQuery);

            glUseProgram(prog);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClear(GL_COLOR_BUFFER_BIT);

            for (int i = 0; i < iterations; i++) {
                glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
            }

            glEndQueryEXT(GL_TIME_ELAPSED_EXT);
            glFinish();

            GLuint64 elapsed = 0;
            glGetQueryObjectui64vEXT(timerQuery, GL_QUERY_RESULT_EXT, &elapsed);

            result.gpuTimeMs = static_cast<double>(elapsed) / 1e6;
            glDeleteQueriesEXT(1, &timerQuery);
        } else {
            glFinish();

            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            glUseProgram(prog);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClear(GL_COLOR_BUFFER_BIT);

            for (int i = 0; i < iterations; i++) {
                glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
            }

            glFinish();
            clock_gettime(CLOCK_MONOTONIC, &end);

            double startMs = start.tv_sec * 1000.0 + start.tv_nsec / 1e6;
            double endMs   = end.tv_sec * 1000.0 + end.tv_nsec / 1e6;
            result.gpuTimeMs = endMs - startMs;
        }

        result.success = true;
        result.testCase = hasTimerQuery ? "ColorWrite RGBA (GPU timer query)" : "ColorWrite RGBA (CPU fallback)";

        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(prog);

    } catch (const std::exception& e) {
        LOGE("ColorWrite Error: %s", e.what());
    }

    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (surface != EGL_NO_SURFACE) eglDestroySurface(display, surface);
        if (context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
        eglTerminate(display);
    }

    return result;
}

// ── Baseline shader source (ESSL 320) ──────────────────────────────
static const char* kBaselineVertSrc = R"(#version 320 es
precision highp float;

layout(location = 0) in vec3 inPos;
out vec4 baseColor;

uniform int loopCount;   // unused
uniform float seed;      // unused

void main() {
    gl_Position = vec4(inPos, 1.0);
    baseColor = vec4(0.5, 0.5, 0.5, 1.0);
}
)";

static const char* kBaselineFragSrc = R"(#version 320 es
precision highp float;

in vec4 baseColor;
out vec4 outColor;

void main() {
    outColor = baseColor;
}
)";

// ====================================================================
// Baseline benchmark: empty shaders + normal colorWriteMask
// ====================================================================
BenchmarkResult runGLESBaseline(int iterations) {
    BenchmarkResult result;
    result.backend    = "OpenGL ES";
    result.testCase   = "Baseline (empty shader, normal colorWriteMask)";
    result.drawCalls  = iterations;
    result.iterations = iterations;
    result.success    = false;
    result.gpuTimeMs  = 0.0;

    std::vector<float> gridVertices;
    generateGridVertices(gridVertices);

    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;

    try {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            throw std::runtime_error("eglGetDisplay failed");
        }
        eglInitialize(display, nullptr, nullptr);

        const EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
            EGL_RED_SIZE,        8,
            EGL_GREEN_SIZE,      8,
            EGL_BLUE_SIZE,       8,
            EGL_ALPHA_SIZE,      8,
            EGL_NONE
        };

        EGLConfig config;
        EGLint numConfigs;
        eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
        if (numConfigs == 0) {
            throw std::runtime_error("No suitable EGL config found");
        }

        const EGLint contextAttribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_MINOR_VERSION, 2,
            EGL_NONE
        };
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
        if (context == EGL_NO_CONTEXT) {
            throw std::runtime_error("eglCreateContext failed");
        }

        const EGLint pbufferAttribs[] = {
            EGL_WIDTH,  64,
            EGL_HEIGHT, 64,
            EGL_NONE
        };
        surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
        eglMakeCurrent(display, surface, surface, context);

        // Check for timer query
        std::string extensions((const char*)glGetString(GL_EXTENSIONS));
        bool hasTimerQuery = (extensions.find("GL_EXT_disjoint_timer_query") != std::string::npos);

        typedef void (*PFNGLGENQUERIESEXTPROC)(GLsizei, GLuint*);
        typedef void (*PFNGLDELETEQUERIESEXTPROC)(GLsizei, const GLuint*);
        typedef void (*PFNGLBEGINQUERYEXTPROC)(GLenum, GLuint);
        typedef void (*PFNGLENDQUERYEXTPROC)(GLenum);
        typedef void (*PFNGLGETQUERYOBJECTUI64VEXTPROC)(GLuint, GLenum, GLuint64*);

        auto glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC)eglGetProcAddress("glGenQueriesEXT");
        auto glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC)eglGetProcAddress("glDeleteQueriesEXT");
        auto glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC)eglGetProcAddress("glBeginQueryEXT");
        auto glEndQueryEXT = (PFNGLENDQUERYEXTPROC)eglGetProcAddress("glEndQueryEXT");
        auto glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)eglGetProcAddress("glGetQueryObjectui64vEXT");

        if (!hasTimerQuery || !glGenQueriesEXT || !glBeginQueryEXT) {
            hasTimerQuery = false;
        }

        // Compile baseline shaders
        GLuint vs   = compileShader(GL_VERTEX_SHADER, kBaselineVertSrc);
        GLuint fs   = compileShader(GL_FRAGMENT_SHADER, kBaselineFragSrc);
        GLuint prog = linkProgram(vs, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);

        glUseProgram(prog);

        // Create VAO + VBO
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float),
                     gridVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        // Create offscreen FBO
        GLuint fbo, rbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 64, 64);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer incomplete");
        }

        glViewport(0, 0, 64, 64);

        // Warm-up
        glUseProgram(prog);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
        glFinish();

        // Timed pass - normal colorMask
        if (hasTimerQuery) {
            GLuint timerQuery;
            glGenQueriesEXT(1, &timerQuery);

            glBeginQueryEXT(GL_TIME_ELAPSED_EXT, timerQuery);

            glUseProgram(prog);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClear(GL_COLOR_BUFFER_BIT);

            for (int i = 0; i < iterations; i++) {
                glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
            }

            glEndQueryEXT(GL_TIME_ELAPSED_EXT);
            glFinish();

            GLuint64 elapsed = 0;
            glGetQueryObjectui64vEXT(timerQuery, GL_QUERY_RESULT_EXT, &elapsed);

            result.gpuTimeMs = static_cast<double>(elapsed) / 1e6;
            glDeleteQueriesEXT(1, &timerQuery);
        } else {
            glFinish();

            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            glUseProgram(prog);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClear(GL_COLOR_BUFFER_BIT);

            for (int i = 0; i < iterations; i++) {
                glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
            }

            glFinish();
            clock_gettime(CLOCK_MONOTONIC, &end);

            double startMs = start.tv_sec * 1000.0 + start.tv_nsec / 1e6;
            double endMs   = end.tv_sec * 1000.0 + end.tv_nsec / 1e6;
            result.gpuTimeMs = endMs - startMs;
        }

        result.success = true;
        result.testCase = hasTimerQuery ? "Baseline (GPU timer query)" : "Baseline (CPU fallback)";

        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(prog);

    } catch (const std::exception& e) {
        LOGE("Baseline Error: %s", e.what());
    }

    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (surface != EGL_NO_SURFACE) eglDestroySurface(display, surface);
        if (context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
        eglTerminate(display);
    }

    return result;
}
