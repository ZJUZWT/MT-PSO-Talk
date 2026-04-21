/*
 * GLAD 2 - OpenGL Loader (GL 4.3 Core profile)
 *
 * Minimal hand-written loader for the ShaderCompileTest benchmark.
 * 
 * On Windows, GL 1.1 functions are exported directly by opengl32.dll.
 * We only need to load GL 2.0+ extension functions via wglGetProcAddress.
 *
 * For a full-featured version, generate from: https://gen.glad.sh/
 */

#ifndef GLAD_GL_H_
#define GLAD_GL_H_

/* Prevent Windows from including its own gl.h */
#define __gl_h_
#define __GL_H__
#define __gl_h
#define _GL_H

#ifdef _WIN32
#ifndef APIENTRY
#define APIENTRY __stdcall
#endif
#ifndef WINGDIAPI
#define WINGDIAPI __declspec(dllimport)
#endif
#else
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef WINGDIAPI
#define WINGDIAPI
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Basic GL types ─────────────────────────────────────────────── */
typedef unsigned int   GLenum;
typedef unsigned char  GLboolean;
typedef unsigned int   GLbitfield;
typedef void           GLvoid;
typedef signed char    GLbyte;
typedef short          GLshort;
typedef int            GLint;
typedef unsigned char  GLubyte;
typedef unsigned short GLushort;
typedef unsigned int   GLuint;
typedef int            GLsizei;
typedef float          GLfloat;
typedef float          GLclampf;
typedef double         GLdouble;
typedef double         GLclampd;
typedef char           GLchar;
typedef long long      GLint64;
typedef unsigned long long GLuint64;

#ifdef _WIN64
typedef signed long long int   GLsizeiptr;
typedef signed long long int   GLintptr;
#else
typedef signed long int   GLsizeiptr;
typedef signed long int   GLintptr;
#endif

typedef GLuint64 GLuint64EXT;

/* ── GL constants ───────────────────────────────────────────────── */
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_NO_ERROR                       0
#define GL_NONE                           0

#define GL_TRIANGLES                      0x0004
#define GL_FLOAT                          0x1406

#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_DEPTH_BUFFER_BIT               0x00000100

#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4

#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_RGBA8                          0x8058

#define GL_TIME_ELAPSED                   0x88BF
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867

#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02

/* ================================================================
 * GL 1.1 functions – provided directly by opengl32.dll on Windows.
 * We declare them as regular extern functions (not function pointers).
 * ================================================================ */
WINGDIAPI void           APIENTRY glClear(GLbitfield mask);
WINGDIAPI void           APIENTRY glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
WINGDIAPI void           APIENTRY glViewport(GLint x, GLint y, GLsizei w, GLsizei h);
WINGDIAPI void           APIENTRY glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a);
WINGDIAPI void           APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count);
WINGDIAPI void           APIENTRY glFinish(void);
WINGDIAPI const GLubyte* APIENTRY glGetString(GLenum name);
WINGDIAPI GLenum         APIENTRY glGetError(void);

/* ================================================================
 * GL 2.0+ extension functions – loaded at runtime via GLAD.
 * These are declared as function pointers.
 * ================================================================ */

/* Shader */
typedef GLuint (APIENTRY *PFNGLCREATESHADERPROC)(GLenum type);
typedef void   (APIENTRY *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void   (APIENTRY *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void   (APIENTRY *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef void   (APIENTRY *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   (APIENTRY *PFNGLDELETESHADERPROC)(GLuint shader);

/* Program */
typedef GLuint (APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
typedef void   (APIENTRY *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void   (APIENTRY *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void   (APIENTRY *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void   (APIENTRY *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void   (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void   (APIENTRY *PFNGLDELETEPROGRAMPROC)(GLuint program);

/* VAO / VBO */
typedef void   (APIENTRY *PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef void   (APIENTRY *PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void   (APIENTRY *PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
typedef void   (APIENTRY *PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void   (APIENTRY *PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void   (APIENTRY *PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void   (APIENTRY *PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
typedef void   (APIENTRY *PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void   (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);

/* FBO / RBO */
typedef void   (APIENTRY *PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint *framebuffers);
typedef void   (APIENTRY *PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
typedef void   (APIENTRY *PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n, const GLuint *framebuffers);
typedef void   (APIENTRY *PFNGLFRAMEBUFFERRENDERBUFFERPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef GLenum (APIENTRY *PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
typedef void   (APIENTRY *PFNGLGENRENDERBUFFERSPROC)(GLsizei n, GLuint *renderbuffers);
typedef void   (APIENTRY *PFNGLBINDRENDERBUFFERPROC)(GLenum target, GLuint renderbuffer);
typedef void   (APIENTRY *PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void   (APIENTRY *PFNGLDELETERENDERBUFFERSPROC)(GLsizei n, const GLuint *renderbuffers);

/* Queries */
typedef void   (APIENTRY *PFNGLGENQUERIESPROC)(GLsizei n, GLuint *ids);
typedef void   (APIENTRY *PFNGLDELETEQUERIESPROC)(GLsizei n, const GLuint *ids);
typedef void   (APIENTRY *PFNGLBEGINQUERYPROC)(GLenum target, GLuint id);
typedef void   (APIENTRY *PFNGLENDQUERYPROC)(GLenum target);
typedef void   (APIENTRY *PFNGLGETQUERYOBJECTUI64VPROC)(GLuint id, GLenum pname, GLuint64 *params);

/* Uniforms */
typedef GLint  (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);
typedef void   (APIENTRY *PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void   (APIENTRY *PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);

/* ── Extension function pointers (extern) ───────────────────────── */
extern PFNGLCREATESHADERPROC             glad_glCreateShader;
extern PFNGLSHADERSOURCEPROC             glad_glShaderSource;
extern PFNGLCOMPILESHADERPROC            glad_glCompileShader;
extern PFNGLGETSHADERIVPROC              glad_glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC         glad_glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC             glad_glDeleteShader;

extern PFNGLCREATEPROGRAMPROC            glad_glCreateProgram;
extern PFNGLATTACHSHADERPROC             glad_glAttachShader;
extern PFNGLLINKPROGRAMPROC              glad_glLinkProgram;
extern PFNGLUSEPROGRAMPROC               glad_glUseProgram;
extern PFNGLGETPROGRAMIVPROC             glad_glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC        glad_glGetProgramInfoLog;
extern PFNGLDELETEPROGRAMPROC            glad_glDeleteProgram;

extern PFNGLGENVERTEXARRAYSPROC          glad_glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC          glad_glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC       glad_glDeleteVertexArrays;
extern PFNGLGENBUFFERSPROC               glad_glGenBuffers;
extern PFNGLBINDBUFFERPROC               glad_glBindBuffer;
extern PFNGLBUFFERDATAPROC               glad_glBufferData;
extern PFNGLDELETEBUFFERSPROC            glad_glDeleteBuffers;
extern PFNGLVERTEXATTRIBPOINTERPROC      glad_glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC  glad_glEnableVertexAttribArray;

extern PFNGLGENFRAMEBUFFERSPROC          glad_glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC          glad_glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC       glad_glDeleteFramebuffers;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC  glad_glFramebufferRenderbuffer;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC   glad_glCheckFramebufferStatus;
extern PFNGLGENRENDERBUFFERSPROC         glad_glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC         glad_glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC      glad_glRenderbufferStorage;
extern PFNGLDELETERENDERBUFFERSPROC      glad_glDeleteRenderbuffers;

extern PFNGLGENQUERIESPROC               glad_glGenQueries;
extern PFNGLDELETEQUERIESPROC            glad_glDeleteQueries;
extern PFNGLBEGINQUERYPROC               glad_glBeginQuery;
extern PFNGLENDQUERYPROC                 glad_glEndQuery;
extern PFNGLGETQUERYOBJECTUI64VPROC      glad_glGetQueryObjectui64v;

extern PFNGLGETUNIFORMLOCATIONPROC       glad_glGetUniformLocation;
extern PFNGLUNIFORM1IPROC                glad_glUniform1i;
extern PFNGLUNIFORM1FPROC                glad_glUniform1f;

/* ── Convenience macros: map glXxx -> glad_glXxx for extensions ── */
#define glCreateShader             glad_glCreateShader
#define glShaderSource             glad_glShaderSource
#define glCompileShader            glad_glCompileShader
#define glGetShaderiv              glad_glGetShaderiv
#define glGetShaderInfoLog         glad_glGetShaderInfoLog
#define glDeleteShader             glad_glDeleteShader

#define glCreateProgram            glad_glCreateProgram
#define glAttachShader             glad_glAttachShader
#define glLinkProgram              glad_glLinkProgram
#define glUseProgram               glad_glUseProgram
#define glGetProgramiv             glad_glGetProgramiv
#define glGetProgramInfoLog        glad_glGetProgramInfoLog
#define glDeleteProgram            glad_glDeleteProgram

#define glGenVertexArrays          glad_glGenVertexArrays
#define glBindVertexArray          glad_glBindVertexArray
#define glDeleteVertexArrays       glad_glDeleteVertexArrays
#define glGenBuffers               glad_glGenBuffers
#define glBindBuffer               glad_glBindBuffer
#define glBufferData               glad_glBufferData
#define glDeleteBuffers            glad_glDeleteBuffers
#define glVertexAttribPointer      glad_glVertexAttribPointer
#define glEnableVertexAttribArray  glad_glEnableVertexAttribArray

#define glGenFramebuffers          glad_glGenFramebuffers
#define glBindFramebuffer          glad_glBindFramebuffer
#define glDeleteFramebuffers       glad_glDeleteFramebuffers
#define glFramebufferRenderbuffer  glad_glFramebufferRenderbuffer
#define glCheckFramebufferStatus   glad_glCheckFramebufferStatus
#define glGenRenderbuffers         glad_glGenRenderbuffers
#define glBindRenderbuffer         glad_glBindRenderbuffer
#define glRenderbufferStorage      glad_glRenderbufferStorage
#define glDeleteRenderbuffers      glad_glDeleteRenderbuffers

#define glGenQueries               glad_glGenQueries
#define glDeleteQueries            glad_glDeleteQueries
#define glBeginQuery               glad_glBeginQuery
#define glEndQuery                 glad_glEndQuery
#define glGetQueryObjectui64v      glad_glGetQueryObjectui64v

#define glGetUniformLocation       glad_glGetUniformLocation
#define glUniform1i                glad_glUniform1i
#define glUniform1f                glad_glUniform1f

/* ── Loader function ────────────────────────────────────────────── */
typedef void* (*GLADloadproc)(const char *name);
int gladLoadGLLoader(GLADloadproc load);

#ifdef __cplusplus
}
#endif

#endif /* GLAD_GL_H_ */
