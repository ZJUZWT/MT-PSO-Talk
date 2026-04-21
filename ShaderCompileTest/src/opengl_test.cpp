// ====================================================================
// opengl_test.cpp – OpenGL benchmark with glColorMask(0,0,0,0)
//
// This test creates an OpenGL shader program (glLinkProgram) with the
// same heavy vertex shader, then sets glColorMask to all-false at draw
// time. Because OpenGL compiles shaders before knowing the render state,
// the driver cannot perform cross-stage DCE at link time.
//
// We measure pure GPU execution time using GL_TIME_ELAPSED query.
// ====================================================================

#include "benchmark.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

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
        std::cerr << "[OpenGL] Shader compile error:\n" << log << "\n";
        glDeleteShader(shader);
        throw std::runtime_error("GL shader compilation failed");
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
        std::cerr << "[OpenGL] Program link error:\n" << log << "\n";
        glDeleteProgram(prog);
        throw std::runtime_error("GL program link failed");
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

// ====================================================================
// Public entry point
// ====================================================================
BenchmarkResult runOpenGLBenchmark(int iterations, int loopCount) {
    BenchmarkResult result;
    result.backend    = "OpenGL";
    result.testCase   = "glColorMask(0,0,0,0) (runtime state)";
    result.drawCalls  = iterations;
    result.iterations = iterations;
    result.success    = false;
    result.gpuTimeMs  = 0.0;

    // Generate grid vertices
    std::vector<float> gridVertices;
    generateGridVertices(gridVertices);

    GLFWwindow* window = nullptr;

    try {
        // ── Create OpenGL context via GLFW ─────────────────────────
        if (!glfwInit()) {
            throw std::runtime_error("GLFW init failed");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window = glfwCreateWindow(64, 64, "GL Bench", nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("Failed to create GLFW window for OpenGL");
        }
        glfwMakeContextCurrent(window);

        // ── Load OpenGL functions via GLAD ─────────────────────────
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        std::cout << "[OpenGL] Renderer: " << glGetString(GL_RENDERER) << "\n";
        std::cout << "[OpenGL] Version:  " << glGetString(GL_VERSION) << "\n";

        // ── Compile shaders ────────────────────────────────────────
        std::string vertSrc = readTextFile("shaders/heavy_gl.vert");
        std::string fragSrc = readTextFile("shaders/heavy_gl.frag");

        GLuint vs   = compileShader(GL_VERTEX_SHADER, vertSrc);
        GLuint fs   = compileShader(GL_FRAGMENT_SHADER, fragSrc);
        GLuint prog = linkProgram(vs, fs);

        glDeleteShader(vs);
        glDeleteShader(fs);

        std::cout << "[OpenGL] Program linked (shader compiled WITHOUT knowledge of ColorMask)\n";

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

        // ── Create GPU timer query ─────────────────────────────────
        GLuint timerQuery;
        glGenQueries(1, &timerQuery);

        // ── Warm-up pass (trigger any lazy JIT compilation) ────────
        // IMPORTANT: warm-up with colorMask ENABLED (all true)!
        // If we warm up with colorMask=false, NVIDIA's driver will
        // JIT-recompile the shader for that state combination,
        // defeating the purpose of the test.
        glUseProgram(prog);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
        glFinish();

        // ── Timed pass ─────────────────────────────────────────────
        glBeginQuery(GL_TIME_ELAPSED, timerQuery);

        glUseProgram(prog);
        // *** THE KEY: set color mask to 0 at DRAW TIME ***
        // OpenGL compiled the shader without knowing this state,
        // so the heavy VS computation is still in the machine code.
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < iterations; i++) {
            glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
        }

        glEndQuery(GL_TIME_ELAPSED);
        glFinish();

        // ── Read timer result ──────────────────────────────────────
        GLuint64 elapsed = 0;
        glGetQueryObjectui64v(timerQuery, GL_QUERY_RESULT, &elapsed);

        result.gpuTimeMs = static_cast<double>(elapsed) / 1e6;
        result.success   = true;

        // ── Cleanup ────────────────────────────────────────────────
        glDeleteQueries(1, &timerQuery);
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(prog);

    } catch (const std::exception& e) {
        std::cerr << "[OpenGL] Error: " << e.what() << "\n";
    }

    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    return result;
}

// ====================================================================
// Heavy shader + glColorMask(TRUE) – forces actual GPU execution
// ====================================================================
BenchmarkResult runOpenGLBenchmarkColorWrite(int iterations, int loopCount) {
    BenchmarkResult result;
    result.backend    = "OpenGL";
    result.testCase   = "glColorMask(1,1,1,1) (heavy shader, full write)";
    result.drawCalls  = iterations;
    result.iterations = iterations;
    result.success    = false;
    result.gpuTimeMs  = 0.0;

    std::vector<float> gridVertices;
    generateGridVertices(gridVertices);

    GLFWwindow* window = nullptr;

    try {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW init failed");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window = glfwCreateWindow(64, 64, "GL ColorWrite", nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("Failed to create GLFW window for OpenGL");
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        std::cout << "[OpenGL ColorWrite] Renderer: " << glGetString(GL_RENDERER) << "\n";

        // Compile shaders (same heavy shader)
        std::string vertSrc = readTextFile("shaders/heavy_gl.vert");
        std::string fragSrc = readTextFile("shaders/heavy_gl.frag");

        GLuint vs   = compileShader(GL_VERTEX_SHADER, vertSrc);
        GLuint fs   = compileShader(GL_FRAGMENT_SHADER, fragSrc);
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

        // Timed pass – colorMask ENABLED (GPU must actually compute everything)
        GLuint timerQuery;
        glGenQueries(1, &timerQuery);

        glBeginQuery(GL_TIME_ELAPSED, timerQuery);

        glUseProgram(prog);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < iterations; i++) {
            glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
        }

        glEndQuery(GL_TIME_ELAPSED);
        glFinish();

        GLuint64 elapsed = 0;
        glGetQueryObjectui64v(timerQuery, GL_QUERY_RESULT, &elapsed);

        result.gpuTimeMs = static_cast<double>(elapsed) / 1e6;
        result.success   = true;

        glDeleteQueries(1, &timerQuery);
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(prog);

    } catch (const std::exception& e) {
        std::cerr << "[OpenGL ColorWrite] Error: " << e.what() << "\n";
    }

    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    return result;
}

// ====================================================================
// Baseline benchmark: empty shaders + normal colorWriteMask
// ====================================================================
BenchmarkResult runOpenGLBaseline(int iterations) {
    BenchmarkResult result;
    result.backend    = "OpenGL";
    result.testCase   = "Baseline (empty shader, normal colorWriteMask)";
    result.drawCalls  = iterations;
    result.iterations = iterations;
    result.success    = false;
    result.gpuTimeMs  = 0.0;

    std::vector<float> gridVertices;
    generateGridVertices(gridVertices);

    GLFWwindow* window = nullptr;

    try {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW init failed");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window = glfwCreateWindow(64, 64, "GL Baseline", nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("Failed to create GLFW window for OpenGL");
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // Compile baseline shaders
        std::string vertSrc = readTextFile("shaders/baseline_gl.vert");
        std::string fragSrc = readTextFile("shaders/baseline_gl.frag");

        GLuint vs   = compileShader(GL_VERTEX_SHADER, vertSrc);
        GLuint fs   = compileShader(GL_FRAGMENT_SHADER, fragSrc);
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

        // Warm-up with normal colorMask
        glUseProgram(prog);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
        glFinish();

        // Timed pass - normal colorMask (writing color)
        GLuint timerQuery;
        glGenQueries(1, &timerQuery);

        glBeginQuery(GL_TIME_ELAPSED, timerQuery);

        glUseProgram(prog);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < iterations; i++) {
            glDrawArrays(GL_TRIANGLES, 0, GRID_VERTEX_COUNT);
        }

        glEndQuery(GL_TIME_ELAPSED);
        glFinish();

        GLuint64 elapsed = 0;
        glGetQueryObjectui64v(timerQuery, GL_QUERY_RESULT, &elapsed);

        result.gpuTimeMs = static_cast<double>(elapsed) / 1e6;
        result.success   = true;

        glDeleteQueries(1, &timerQuery);
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(prog);

    } catch (const std::exception& e) {
        std::cerr << "[OpenGL Baseline] Error: " << e.what() << "\n";
    }

    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    return result;
}
