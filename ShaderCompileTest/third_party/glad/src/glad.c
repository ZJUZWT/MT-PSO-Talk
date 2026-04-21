/*
 * GLAD - OpenGL Loader implementation
 * Only loads GL 2.0+ extension functions. GL 1.1 functions come from opengl32.lib.
 */

#include <glad/glad.h>
#include <stddef.h>

/* ── Extension function pointer definitions ─────────────────────── */
PFNGLCREATESHADERPROC             glad_glCreateShader             = NULL;
PFNGLSHADERSOURCEPROC             glad_glShaderSource             = NULL;
PFNGLCOMPILESHADERPROC            glad_glCompileShader            = NULL;
PFNGLGETSHADERIVPROC              glad_glGetShaderiv              = NULL;
PFNGLGETSHADERINFOLOGPROC         glad_glGetShaderInfoLog         = NULL;
PFNGLDELETESHADERPROC             glad_glDeleteShader             = NULL;

PFNGLCREATEPROGRAMPROC            glad_glCreateProgram            = NULL;
PFNGLATTACHSHADERPROC             glad_glAttachShader             = NULL;
PFNGLLINKPROGRAMPROC              glad_glLinkProgram              = NULL;
PFNGLUSEPROGRAMPROC               glad_glUseProgram               = NULL;
PFNGLGETPROGRAMIVPROC             glad_glGetProgramiv             = NULL;
PFNGLGETPROGRAMINFOLOGPROC        glad_glGetProgramInfoLog        = NULL;
PFNGLDELETEPROGRAMPROC            glad_glDeleteProgram            = NULL;

PFNGLGENVERTEXARRAYSPROC          glad_glGenVertexArrays          = NULL;
PFNGLBINDVERTEXARRAYPROC          glad_glBindVertexArray          = NULL;
PFNGLDELETEVERTEXARRAYSPROC       glad_glDeleteVertexArrays       = NULL;
PFNGLGENBUFFERSPROC               glad_glGenBuffers               = NULL;
PFNGLBINDBUFFERPROC               glad_glBindBuffer               = NULL;
PFNGLBUFFERDATAPROC               glad_glBufferData               = NULL;
PFNGLDELETEBUFFERSPROC            glad_glDeleteBuffers            = NULL;
PFNGLVERTEXATTRIBPOINTERPROC      glad_glVertexAttribPointer      = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC  glad_glEnableVertexAttribArray  = NULL;

PFNGLGENFRAMEBUFFERSPROC          glad_glGenFramebuffers          = NULL;
PFNGLBINDFRAMEBUFFERPROC          glad_glBindFramebuffer          = NULL;
PFNGLDELETEFRAMEBUFFERSPROC       glad_glDeleteFramebuffers       = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC  glad_glFramebufferRenderbuffer  = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC   glad_glCheckFramebufferStatus   = NULL;
PFNGLGENRENDERBUFFERSPROC         glad_glGenRenderbuffers         = NULL;
PFNGLBINDRENDERBUFFERPROC         glad_glBindRenderbuffer         = NULL;
PFNGLRENDERBUFFERSTORAGEPROC      glad_glRenderbufferStorage      = NULL;
PFNGLDELETERENDERBUFFERSPROC      glad_glDeleteRenderbuffers      = NULL;

PFNGLGENQUERIESPROC               glad_glGenQueries               = NULL;
PFNGLDELETEQUERIESPROC            glad_glDeleteQueries            = NULL;
PFNGLBEGINQUERYPROC               glad_glBeginQuery               = NULL;
PFNGLENDQUERYPROC                 glad_glEndQuery                 = NULL;
PFNGLGETQUERYOBJECTUI64VPROC      glad_glGetQueryObjectui64v      = NULL;

PFNGLGETUNIFORMLOCATIONPROC       glad_glGetUniformLocation       = NULL;
PFNGLUNIFORM1IPROC                glad_glUniform1i                = NULL;
PFNGLUNIFORM1FPROC                glad_glUniform1f                = NULL;

/* ── Loader ─────────────────────────────────────────────────────── */
int gladLoadGLLoader(GLADloadproc load) {
    if (load == NULL) return 0;

    glad_glCreateShader             = (PFNGLCREATESHADERPROC)            load("glCreateShader");
    glad_glShaderSource             = (PFNGLSHADERSOURCEPROC)            load("glShaderSource");
    glad_glCompileShader            = (PFNGLCOMPILESHADERPROC)           load("glCompileShader");
    glad_glGetShaderiv              = (PFNGLGETSHADERIVPROC)             load("glGetShaderiv");
    glad_glGetShaderInfoLog         = (PFNGLGETSHADERINFOLOGPROC)        load("glGetShaderInfoLog");
    glad_glDeleteShader             = (PFNGLDELETESHADERPROC)            load("glDeleteShader");

    glad_glCreateProgram            = (PFNGLCREATEPROGRAMPROC)           load("glCreateProgram");
    glad_glAttachShader             = (PFNGLATTACHSHADERPROC)            load("glAttachShader");
    glad_glLinkProgram              = (PFNGLLINKPROGRAMPROC)             load("glLinkProgram");
    glad_glUseProgram               = (PFNGLUSEPROGRAMPROC)              load("glUseProgram");
    glad_glGetProgramiv             = (PFNGLGETPROGRAMIVPROC)            load("glGetProgramiv");
    glad_glGetProgramInfoLog        = (PFNGLGETPROGRAMINFOLOGPROC)       load("glGetProgramInfoLog");
    glad_glDeleteProgram            = (PFNGLDELETEPROGRAMPROC)           load("glDeleteProgram");

    glad_glGenVertexArrays          = (PFNGLGENVERTEXARRAYSPROC)         load("glGenVertexArrays");
    glad_glBindVertexArray          = (PFNGLBINDVERTEXARRAYPROC)         load("glBindVertexArray");
    glad_glDeleteVertexArrays       = (PFNGLDELETEVERTEXARRAYSPROC)      load("glDeleteVertexArrays");
    glad_glGenBuffers               = (PFNGLGENBUFFERSPROC)              load("glGenBuffers");
    glad_glBindBuffer               = (PFNGLBINDBUFFERPROC)              load("glBindBuffer");
    glad_glBufferData               = (PFNGLBUFFERDATAPROC)              load("glBufferData");
    glad_glDeleteBuffers            = (PFNGLDELETEBUFFERSPROC)           load("glDeleteBuffers");
    glad_glVertexAttribPointer      = (PFNGLVERTEXATTRIBPOINTERPROC)     load("glVertexAttribPointer");
    glad_glEnableVertexAttribArray  = (PFNGLENABLEVERTEXATTRIBARRAYPROC) load("glEnableVertexAttribArray");

    glad_glGenFramebuffers          = (PFNGLGENFRAMEBUFFERSPROC)         load("glGenFramebuffers");
    glad_glBindFramebuffer          = (PFNGLBINDFRAMEBUFFERPROC)         load("glBindFramebuffer");
    glad_glDeleteFramebuffers       = (PFNGLDELETEFRAMEBUFFERSPROC)      load("glDeleteFramebuffers");
    glad_glFramebufferRenderbuffer  = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) load("glFramebufferRenderbuffer");
    glad_glCheckFramebufferStatus   = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)  load("glCheckFramebufferStatus");
    glad_glGenRenderbuffers         = (PFNGLGENRENDERBUFFERSPROC)        load("glGenRenderbuffers");
    glad_glBindRenderbuffer         = (PFNGLBINDRENDERBUFFERPROC)        load("glBindRenderbuffer");
    glad_glRenderbufferStorage      = (PFNGLRENDERBUFFERSTORAGEPROC)     load("glRenderbufferStorage");
    glad_glDeleteRenderbuffers      = (PFNGLDELETERENDERBUFFERSPROC)     load("glDeleteRenderbuffers");

    glad_glGenQueries               = (PFNGLGENQUERIESPROC)              load("glGenQueries");
    glad_glDeleteQueries            = (PFNGLDELETEQUERIESPROC)           load("glDeleteQueries");
    glad_glBeginQuery               = (PFNGLBEGINQUERYPROC)              load("glBeginQuery");
    glad_glEndQuery                 = (PFNGLENDQUERYPROC)                load("glEndQuery");
    glad_glGetQueryObjectui64v      = (PFNGLGETQUERYOBJECTUI64VPROC)     load("glGetQueryObjectui64v");

    glad_glGetUniformLocation       = (PFNGLGETUNIFORMLOCATIONPROC)      load("glGetUniformLocation");
    glad_glUniform1i                = (PFNGLUNIFORM1IPROC)               load("glUniform1i");
    glad_glUniform1f                = (PFNGLUNIFORM1FPROC)               load("glUniform1f");

    /* Basic sanity check */
    if (glad_glCreateShader == NULL || glad_glCreateProgram == NULL) return 0;
    return 1;
}
