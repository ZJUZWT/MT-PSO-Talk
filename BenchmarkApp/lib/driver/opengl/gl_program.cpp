#include "driver/opengl/gl_program.h"

#if defined(__ANDROID__)

#include <GLES3/gl3.h>

#include "driver/common/timing_scope.h"

namespace benchmark {

static GLuint compile_shader(GLenum type, const std::string& source, int64_t& duration_us, std::string& status) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);

    {
        TimingScope scope(duration_us);
        glCompileShader(shader);
    }

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        status = std::string("shader_compile_failed: ") + log;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLCompileResult compile_gl_program(
    const std::string& vert_source,
    const std::string& frag_source) {

    GLCompileResult result;

    GLuint vert = compile_shader(GL_VERTEX_SHADER, vert_source, result.compile_vertex_us, result.status);
    if (vert == 0) return result;

    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, frag_source, result.compile_fragment_us, result.status);
    if (frag == 0) {
        glDeleteShader(vert);
        return result;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);

    {
        TimingScope scope(result.link_program_us);
        glLinkProgram(program);
    }

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        result.status = std::string("link_failed: ") + log;
    }

    result.total_us = result.compile_vertex_us + result.compile_fragment_us + result.link_program_us;

    // Cleanup
    glDeleteProgram(program);
    glDeleteShader(vert);
    glDeleteShader(frag);

    return result;
}

}  // namespace benchmark

#endif  // __ANDROID__
