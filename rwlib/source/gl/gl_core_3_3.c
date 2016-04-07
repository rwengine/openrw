#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "gl_core_3_3.h"

#if defined(__APPLE__)
#include <mach-o/dyld.h>

static void* AppleGLGetProcAddress (const GLubyte *name)
{
  static const struct mach_header* image = NULL;
  NSSymbol symbol;
  char* symbolName;
  if (NULL == image)
  {
    image = NSAddImage("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
  }
  /* prepend a '_' for the Unix C symbol mangling convention */
  symbolName = malloc(strlen((const char*)name) + 2);
  strcpy(symbolName+1, (const char*)name);
  symbolName[0] = '_';
  symbol = NULL;
  /* if (NSIsSymbolNameDefined(symbolName))
	 symbol = NSLookupAndBindSymbol(symbolName); */
  symbol = image ? NSLookupSymbolInImage(image, symbolName, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR) : NULL;
  free(symbolName);
  return symbol ? NSAddressOfSymbol(symbol) : NULL;
}
#endif /* __APPLE__ */

#if defined(__sgi) || defined (__sun)
#include <dlfcn.h>
#include <stdio.h>

static void* SunGetProcAddress (const GLubyte* name)
{
  static void* h = NULL;
  static void* gpa;

  if (h == NULL)
  {
    if ((h = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL)) == NULL) return NULL;
    gpa = dlsym(h, "glXGetProcAddress");
  }

  if (gpa != NULL)
    return ((void*(*)(const GLubyte*))gpa)(name);
  else
    return dlsym(h, (const char*)name);
}
#endif /* __sgi || __sun */

#if defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(disable: 4055)
#pragma warning(disable: 4054)
#endif

static int TestPointer(const PROC pTest)
{
	ptrdiff_t iTest;
	if(!pTest) return 0;
	iTest = (ptrdiff_t)pTest;
	
	if(iTest == 1 || iTest == 2 || iTest == 3 || iTest == -1) return 0;
	
	return 1;
}

static PROC WinGetProcAddress(const char *name)
{
	HMODULE glMod = NULL;
	PROC pFunc = wglGetProcAddress((LPCSTR)name);
	if(TestPointer(pFunc))
	{
		return pFunc;
	}
	glMod = GetModuleHandleA("OpenGL32.dll");
	return (PROC)GetProcAddress(glMod, (LPCSTR)name);
}
	
#define IntGetProcAddress(name) WinGetProcAddress(name)
#else
	#if defined(__APPLE__)
		#define IntGetProcAddress(name) AppleGLGetProcAddress(name)
	#else
		#if defined(__sgi) || defined(__sun)
			#define IntGetProcAddress(name) SunGetProcAddress(name)
		#else /* GLX */
		    #include <GL/glx.h>

			#define IntGetProcAddress(name) (*glXGetProcAddressARB)((const GLubyte*)name)
		#endif
	#endif
#endif

int ogl_ext_EXT_texture_compression_s3tc = 0;
int ogl_ext_EXT_texture_sRGB = 0;
int ogl_ext_EXT_texture_filter_anisotropic = 0;
int ogl_ext_ARB_compressed_texture_pixel_storage = 0;
int ogl_ext_ARB_conservative_depth = 0;
int ogl_ext_ARB_ES2_compatibility = 0;
int ogl_ext_ARB_get_program_binary = 0;
int ogl_ext_ARB_explicit_uniform_location = 0;
int ogl_ext_ARB_internalformat_query = 0;
int ogl_ext_ARB_internalformat_query2 = 0;
int ogl_ext_ARB_map_buffer_alignment = 0;
int ogl_ext_ARB_program_interface_query = 0;
int ogl_ext_ARB_separate_shader_objects = 0;
int ogl_ext_ARB_shading_language_420pack = 0;
int ogl_ext_ARB_shading_language_packing = 0;
int ogl_ext_ARB_texture_buffer_range = 0;
int ogl_ext_ARB_texture_storage = 0;
int ogl_ext_ARB_texture_view = 0;
int ogl_ext_ARB_vertex_attrib_binding = 0;
int ogl_ext_ARB_viewport_array = 0;
int ogl_ext_ARB_arrays_of_arrays = 0;
int ogl_ext_ARB_clear_buffer_object = 0;
int ogl_ext_ARB_copy_image = 0;
int ogl_ext_ARB_ES3_compatibility = 0;
int ogl_ext_ARB_fragment_layer_viewport = 0;
int ogl_ext_ARB_framebuffer_no_attachments = 0;
int ogl_ext_ARB_invalidate_subdata = 0;
int ogl_ext_ARB_robust_buffer_access_behavior = 0;
int ogl_ext_ARB_stencil_texturing = 0;
int ogl_ext_ARB_texture_query_levels = 0;
int ogl_ext_ARB_texture_storage_multisample = 0;
int ogl_ext_KHR_debug = 0;

// Extension: ARB_ES2_compatibility
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARDEPTHFPROC)(GLfloat);
static void CODEGEN_FUNCPTR Switch_ClearDepthf(GLfloat d);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDEPTHRANGEFPROC)(GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_DepthRangef(GLfloat n, GLfloat f);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSHADERPRECISIONFORMATPROC)(GLenum, GLenum, GLint *, GLint *);
static void CODEGEN_FUNCPTR Switch_GetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLRELEASESHADERCOMPILERPROC)();
static void CODEGEN_FUNCPTR Switch_ReleaseShaderCompiler();
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSHADERBINARYPROC)(GLsizei, const GLuint *, GLenum, const GLvoid *, GLsizei);
static void CODEGEN_FUNCPTR Switch_ShaderBinary(GLsizei count, const GLuint * shaders, GLenum binaryformat, const GLvoid * binary, GLsizei length);

// Extension: ARB_get_program_binary
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMBINARYPROC)(GLuint, GLsizei, GLsizei *, GLenum *, GLvoid *);
static void CODEGEN_FUNCPTR Switch_GetProgramBinary(GLuint program, GLsizei bufSize, GLsizei * length, GLenum * binaryFormat, GLvoid * binary);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMBINARYPROC)(GLuint, GLenum, const GLvoid *, GLsizei);
static void CODEGEN_FUNCPTR Switch_ProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid * binary, GLsizei length);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMPARAMETERIPROC)(GLuint, GLenum, GLint);
static void CODEGEN_FUNCPTR Switch_ProgramParameteri(GLuint program, GLenum pname, GLint value);

// Extension: ARB_internalformat_query
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETINTERNALFORMATIVPROC)(GLenum, GLenum, GLenum, GLsizei, GLint *);
static void CODEGEN_FUNCPTR Switch_GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint * params);

// Extension: ARB_internalformat_query2
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETINTERNALFORMATI64VPROC)(GLenum, GLenum, GLenum, GLsizei, GLint64 *);
static void CODEGEN_FUNCPTR Switch_GetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 * params);

// Extension: ARB_program_interface_query
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMINTERFACEIVPROC)(GLuint, GLenum, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint * params);
typedef GLuint (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMRESOURCEINDEXPROC)(GLuint, GLenum, const GLchar *);
static GLuint CODEGEN_FUNCPTR Switch_GetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar * name);
typedef GLint (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMRESOURCELOCATIONPROC)(GLuint, GLenum, const GLchar *);
static GLint CODEGEN_FUNCPTR Switch_GetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar * name);
typedef GLint (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMRESOURCELOCATIONINDEXPROC)(GLuint, GLenum, const GLchar *);
static GLint CODEGEN_FUNCPTR Switch_GetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMRESOURCENAMEPROC)(GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei * length, GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMRESOURCEIVPROC)(GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei *, GLint *);
static void CODEGEN_FUNCPTR Switch_GetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum * props, GLsizei bufSize, GLsizei * length, GLint * params);

// Extension: ARB_separate_shader_objects
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLACTIVESHADERPROGRAMPROC)(GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_ActiveShaderProgram(GLuint pipeline, GLuint program);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDPROGRAMPIPELINEPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_BindProgramPipeline(GLuint pipeline);
typedef GLuint (CODEGEN_FUNCPTR *PFN_PTRC_GLCREATESHADERPROGRAMVPROC)(GLenum, GLsizei, const GLchar *const*);
static GLuint CODEGEN_FUNCPTR Switch_CreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const* strings);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETEPROGRAMPIPELINESPROC)(GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_DeleteProgramPipelines(GLsizei n, const GLuint * pipelines);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGENPROGRAMPIPELINESPROC)(GLsizei, GLuint *);
static void CODEGEN_FUNCPTR Switch_GenProgramPipelines(GLsizei n, GLuint * pipelines);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMPIPELINEINFOLOGPROC)(GLuint, GLsizei, GLsizei *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMPIPELINEIVPROC)(GLuint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint * params);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISPROGRAMPIPELINEPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsProgramPipeline(GLuint pipeline);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM1DPROC)(GLuint, GLint, GLdouble);
static void CODEGEN_FUNCPTR Switch_ProgramUniform1d(GLuint program, GLint location, GLdouble v0);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM1DVPROC)(GLuint, GLint, GLsizei, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM1FPROC)(GLuint, GLint, GLfloat);
static void CODEGEN_FUNCPTR Switch_ProgramUniform1f(GLuint program, GLint location, GLfloat v0);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM1FVPROC)(GLuint, GLint, GLsizei, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM1IPROC)(GLuint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_ProgramUniform1i(GLuint program, GLint location, GLint v0);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM1IVPROC)(GLuint, GLint, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM1UIPROC)(GLuint, GLint, GLuint);
static void CODEGEN_FUNCPTR Switch_ProgramUniform1ui(GLuint program, GLint location, GLuint v0);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM1UIVPROC)(GLuint, GLint, GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM2DPROC)(GLuint, GLint, GLdouble, GLdouble);
static void CODEGEN_FUNCPTR Switch_ProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM2DVPROC)(GLuint, GLint, GLsizei, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM2FPROC)(GLuint, GLint, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_ProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM2FVPROC)(GLuint, GLint, GLsizei, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM2IPROC)(GLuint, GLint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_ProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM2IVPROC)(GLuint, GLint, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM2UIPROC)(GLuint, GLint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_ProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM2UIVPROC)(GLuint, GLint, GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM3DPROC)(GLuint, GLint, GLdouble, GLdouble, GLdouble);
static void CODEGEN_FUNCPTR Switch_ProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM3DVPROC)(GLuint, GLint, GLsizei, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM3FPROC)(GLuint, GLint, GLfloat, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_ProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM3FVPROC)(GLuint, GLint, GLsizei, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM3IPROC)(GLuint, GLint, GLint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_ProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM3IVPROC)(GLuint, GLint, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM3UIPROC)(GLuint, GLint, GLuint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_ProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM3UIVPROC)(GLuint, GLint, GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM4DPROC)(GLuint, GLint, GLdouble, GLdouble, GLdouble, GLdouble);
static void CODEGEN_FUNCPTR Switch_ProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM4DVPROC)(GLuint, GLint, GLsizei, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM4FPROC)(GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_ProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM4FVPROC)(GLuint, GLint, GLsizei, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM4IPROC)(GLuint, GLint, GLint, GLint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_ProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM4IVPROC)(GLuint, GLint, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM4UIPROC)(GLuint, GLint, GLuint, GLuint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_ProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORM4UIVPROC)(GLuint, GLint, GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_ProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX2DVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX2FVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X3DVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X3FVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X4DVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X4FVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX3DVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX3FVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X2DVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X2FVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X4DVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X4FVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX4DVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX4FVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X2DVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X2FVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X3DVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X3FVPROC)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUSEPROGRAMSTAGESPROC)(GLuint, GLbitfield, GLuint);
static void CODEGEN_FUNCPTR Switch_UseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVALIDATEPROGRAMPIPELINEPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_ValidateProgramPipeline(GLuint pipeline);

// Extension: ARB_texture_buffer_range
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXBUFFERRANGEPROC)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr);
static void CODEGEN_FUNCPTR Switch_TexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);

// Extension: ARB_texture_storage
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXSTORAGE1DPROC)(GLenum, GLsizei, GLenum, GLsizei);
static void CODEGEN_FUNCPTR Switch_TexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXSTORAGE2DPROC)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_TexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXSTORAGE3DPROC)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_TexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

// Extension: ARB_texture_view
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXTUREVIEWPROC)(GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_TextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);

// Extension: ARB_vertex_attrib_binding
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDVERTEXBUFFERPROC)(GLuint, GLuint, GLintptr, GLsizei);
static void CODEGEN_FUNCPTR Switch_BindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBBINDINGPROC)(GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribBinding(GLuint attribindex, GLuint bindingindex);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBFORMATPROC)(GLuint, GLint, GLenum, GLboolean, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBIFORMATPROC)(GLuint, GLint, GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBLFORMATPROC)(GLuint, GLint, GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXBINDINGDIVISORPROC)(GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexBindingDivisor(GLuint bindingindex, GLuint divisor);

// Extension: ARB_viewport_array
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDEPTHRANGEARRAYVPROC)(GLuint, GLsizei, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_DepthRangeArrayv(GLuint first, GLsizei count, const GLdouble * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDEPTHRANGEINDEXEDPROC)(GLuint, GLdouble, GLdouble);
static void CODEGEN_FUNCPTR Switch_DepthRangeIndexed(GLuint index, GLdouble n, GLdouble f);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETDOUBLEI_VPROC)(GLenum, GLuint, GLdouble *);
static void CODEGEN_FUNCPTR Switch_GetDoublei_v(GLenum target, GLuint index, GLdouble * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETFLOATI_VPROC)(GLenum, GLuint, GLfloat *);
static void CODEGEN_FUNCPTR Switch_GetFloati_v(GLenum target, GLuint index, GLfloat * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSCISSORARRAYVPROC)(GLuint, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_ScissorArrayv(GLuint first, GLsizei count, const GLint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSCISSORINDEXEDPROC)(GLuint, GLint, GLint, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_ScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSCISSORINDEXEDVPROC)(GLuint, const GLint *);
static void CODEGEN_FUNCPTR Switch_ScissorIndexedv(GLuint index, const GLint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVIEWPORTARRAYVPROC)(GLuint, GLsizei, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ViewportArrayv(GLuint first, GLsizei count, const GLfloat * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVIEWPORTINDEXEDFPROC)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_ViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVIEWPORTINDEXEDFVPROC)(GLuint, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ViewportIndexedfv(GLuint index, const GLfloat * v);

// Extension: ARB_clear_buffer_object
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARBUFFERDATAPROC)(GLenum, GLenum, GLenum, GLenum, const void *);
static void CODEGEN_FUNCPTR Switch_ClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARBUFFERSUBDATAPROC)(GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *);
static void CODEGEN_FUNCPTR Switch_ClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void * data);

// Extension: ARB_copy_image
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOPYIMAGESUBDATAPROC)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_CopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);

// Extension: ARB_framebuffer_no_attachments
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFRAMEBUFFERPARAMETERIPROC)(GLenum, GLenum, GLint);
static void CODEGEN_FUNCPTR Switch_FramebufferParameteri(GLenum target, GLenum pname, GLint param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETFRAMEBUFFERPARAMETERIVPROC)(GLenum, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetFramebufferParameteriv(GLenum target, GLenum pname, GLint * params);

// Extension: ARB_invalidate_subdata
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLINVALIDATEBUFFERDATAPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_InvalidateBufferData(GLuint buffer);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLINVALIDATEBUFFERSUBDATAPROC)(GLuint, GLintptr, GLsizeiptr);
static void CODEGEN_FUNCPTR Switch_InvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLINVALIDATEFRAMEBUFFERPROC)(GLenum, GLsizei, const GLenum *);
static void CODEGEN_FUNCPTR Switch_InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum * attachments);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLINVALIDATESUBFRAMEBUFFERPROC)(GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_InvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLINVALIDATETEXIMAGEPROC)(GLuint, GLint);
static void CODEGEN_FUNCPTR Switch_InvalidateTexImage(GLuint texture, GLint level);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLINVALIDATETEXSUBIMAGEPROC)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_InvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);

// Extension: ARB_texture_storage_multisample
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXSTORAGE2DMULTISAMPLEPROC)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
static void CODEGEN_FUNCPTR Switch_TexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXSTORAGE3DMULTISAMPLEPROC)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
static void CODEGEN_FUNCPTR Switch_TexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);

// Extension: KHR_debug
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROC, const void *);
static void CODEGEN_FUNCPTR Switch_DebugMessageCallback(GLDEBUGPROC callback, const void * userParam);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDEBUGMESSAGECONTROLPROC)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean);
static void CODEGEN_FUNCPTR Switch_DebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDEBUGMESSAGEINSERTPROC)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *);
static void CODEGEN_FUNCPTR Switch_DebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * buf);
typedef GLuint (CODEGEN_FUNCPTR *PFN_PTRC_GLGETDEBUGMESSAGELOGPROC)(GLuint, GLsizei, GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *);
static GLuint CODEGEN_FUNCPTR Switch_GetDebugMessageLog(GLuint count, GLsizei bufsize, GLenum * sources, GLenum * types, GLuint * ids, GLenum * severities, GLsizei * lengths, GLchar * messageLog);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETOBJECTLABELPROC)(GLenum, GLuint, GLsizei, GLsizei *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei * length, GLchar * label);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETOBJECTPTRLABELPROC)(const void *, GLsizei, GLsizei *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetObjectPtrLabel(const void * ptr, GLsizei bufSize, GLsizei * length, GLchar * label);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPOINTERVPROC)(GLenum, GLvoid **);
static void CODEGEN_FUNCPTR Switch_GetPointerv(GLenum pname, GLvoid ** params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLOBJECTLABELPROC)(GLenum, GLuint, GLsizei, const GLchar *);
static void CODEGEN_FUNCPTR Switch_ObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar * label);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLOBJECTPTRLABELPROC)(const void *, GLsizei, const GLchar *);
static void CODEGEN_FUNCPTR Switch_ObjectPtrLabel(const void * ptr, GLsizei length, const GLchar * label);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPOPDEBUGGROUPPROC)();
static void CODEGEN_FUNCPTR Switch_PopDebugGroup();
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPUSHDEBUGGROUPPROC)(GLenum, GLuint, GLsizei, const GLchar *);
static void CODEGEN_FUNCPTR Switch_PushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar * message);

// Extension: 1.0
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBLENDFUNCPROC)(GLenum, GLenum);
static void CODEGEN_FUNCPTR Switch_BlendFunc(GLenum sfactor, GLenum dfactor);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARPROC)(GLbitfield);
static void CODEGEN_FUNCPTR Switch_Clear(GLbitfield mask);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARCOLORPROC)(GLfloat, GLfloat, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARDEPTHPROC)(GLdouble);
static void CODEGEN_FUNCPTR Switch_ClearDepth(GLdouble depth);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARSTENCILPROC)(GLint);
static void CODEGEN_FUNCPTR Switch_ClearStencil(GLint s);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOLORMASKPROC)(GLboolean, GLboolean, GLboolean, GLboolean);
static void CODEGEN_FUNCPTR Switch_ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCULLFACEPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_CullFace(GLenum mode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDEPTHFUNCPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_DepthFunc(GLenum func);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDEPTHMASKPROC)(GLboolean);
static void CODEGEN_FUNCPTR Switch_DepthMask(GLboolean flag);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDEPTHRANGEPROC)(GLdouble, GLdouble);
static void CODEGEN_FUNCPTR Switch_DepthRange(GLdouble ren_near, GLdouble ren_far);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDISABLEPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_Disable(GLenum cap);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWBUFFERPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_DrawBuffer(GLenum mode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLENABLEPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_Enable(GLenum cap);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFINISHPROC)();
static void CODEGEN_FUNCPTR Switch_Finish();
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFLUSHPROC)();
static void CODEGEN_FUNCPTR Switch_Flush();
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFRONTFACEPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_FrontFace(GLenum mode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETBOOLEANVPROC)(GLenum, GLboolean *);
static void CODEGEN_FUNCPTR Switch_GetBooleanv(GLenum pname, GLboolean * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETDOUBLEVPROC)(GLenum, GLdouble *);
static void CODEGEN_FUNCPTR Switch_GetDoublev(GLenum pname, GLdouble * params);
typedef GLenum (CODEGEN_FUNCPTR *PFN_PTRC_GLGETERRORPROC)();
static GLenum CODEGEN_FUNCPTR Switch_GetError();
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETFLOATVPROC)(GLenum, GLfloat *);
static void CODEGEN_FUNCPTR Switch_GetFloatv(GLenum pname, GLfloat * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETINTEGERVPROC)(GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetIntegerv(GLenum pname, GLint * params);
typedef const GLubyte * (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSTRINGPROC)(GLenum);
static const GLubyte * CODEGEN_FUNCPTR Switch_GetString(GLenum name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETTEXIMAGEPROC)(GLenum, GLint, GLenum, GLenum, GLvoid *);
static void CODEGEN_FUNCPTR Switch_GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETTEXLEVELPARAMETERFVPROC)(GLenum, GLint, GLenum, GLfloat *);
static void CODEGEN_FUNCPTR Switch_GetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETTEXLEVELPARAMETERIVPROC)(GLenum, GLint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETTEXPARAMETERFVPROC)(GLenum, GLenum, GLfloat *);
static void CODEGEN_FUNCPTR Switch_GetTexParameterfv(GLenum target, GLenum pname, GLfloat * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETTEXPARAMETERIVPROC)(GLenum, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetTexParameteriv(GLenum target, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLHINTPROC)(GLenum, GLenum);
static void CODEGEN_FUNCPTR Switch_Hint(GLenum target, GLenum mode);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISENABLEDPROC)(GLenum);
static GLboolean CODEGEN_FUNCPTR Switch_IsEnabled(GLenum cap);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLLINEWIDTHPROC)(GLfloat);
static void CODEGEN_FUNCPTR Switch_LineWidth(GLfloat width);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLLOGICOPPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_LogicOp(GLenum opcode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPIXELSTOREFPROC)(GLenum, GLfloat);
static void CODEGEN_FUNCPTR Switch_PixelStoref(GLenum pname, GLfloat param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPIXELSTOREIPROC)(GLenum, GLint);
static void CODEGEN_FUNCPTR Switch_PixelStorei(GLenum pname, GLint param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPOINTSIZEPROC)(GLfloat);
static void CODEGEN_FUNCPTR Switch_PointSize(GLfloat size);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPOLYGONMODEPROC)(GLenum, GLenum);
static void CODEGEN_FUNCPTR Switch_PolygonMode(GLenum face, GLenum mode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLREADBUFFERPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_ReadBuffer(GLenum mode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLREADPIXELSPROC)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
static void CODEGEN_FUNCPTR Switch_ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSCISSORPROC)(GLint, GLint, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_Scissor(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSTENCILFUNCPROC)(GLenum, GLint, GLuint);
static void CODEGEN_FUNCPTR Switch_StencilFunc(GLenum func, GLint ref, GLuint mask);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSTENCILMASKPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_StencilMask(GLuint mask);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSTENCILOPPROC)(GLenum, GLenum, GLenum);
static void CODEGEN_FUNCPTR Switch_StencilOp(GLenum fail, GLenum zfail, GLenum zpass);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXIMAGE1DPROC)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_TexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXIMAGE2DPROC)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXPARAMETERFPROC)(GLenum, GLenum, GLfloat);
static void CODEGEN_FUNCPTR Switch_TexParameterf(GLenum target, GLenum pname, GLfloat param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXPARAMETERFVPROC)(GLenum, GLenum, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_TexParameterfv(GLenum target, GLenum pname, const GLfloat * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXPARAMETERIPROC)(GLenum, GLenum, GLint);
static void CODEGEN_FUNCPTR Switch_TexParameteri(GLenum target, GLenum pname, GLint param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXPARAMETERIVPROC)(GLenum, GLenum, const GLint *);
static void CODEGEN_FUNCPTR Switch_TexParameteriv(GLenum target, GLenum pname, const GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVIEWPORTPROC)(GLint, GLint, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

// Extension: 1.1
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDTEXTUREPROC)(GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_BindTexture(GLenum target, GLuint texture);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOPYTEXIMAGE1DPROC)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
static void CODEGEN_FUNCPTR Switch_CopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOPYTEXIMAGE2DPROC)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
static void CODEGEN_FUNCPTR Switch_CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOPYTEXSUBIMAGE1DPROC)(GLenum, GLint, GLint, GLint, GLint, GLsizei);
static void CODEGEN_FUNCPTR Switch_CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOPYTEXSUBIMAGE2DPROC)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETETEXTURESPROC)(GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_DeleteTextures(GLsizei n, const GLuint * textures);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWARRAYSPROC)(GLenum, GLint, GLsizei);
static void CODEGEN_FUNCPTR Switch_DrawArrays(GLenum mode, GLint first, GLsizei count);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWELEMENTSPROC)(GLenum, GLsizei, GLenum, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGENTEXTURESPROC)(GLsizei, GLuint *);
static void CODEGEN_FUNCPTR Switch_GenTextures(GLsizei n, GLuint * textures);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISTEXTUREPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsTexture(GLuint texture);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPOLYGONOFFSETPROC)(GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_PolygonOffset(GLfloat factor, GLfloat units);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXSUBIMAGE1DPROC)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_TexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXSUBIMAGE2DPROC)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);

// Extension: 1.2
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBLENDCOLORPROC)(GLfloat, GLfloat, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_BlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBLENDEQUATIONPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_BlendEquation(GLenum mode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOPYTEXSUBIMAGE3DPROC)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWRANGEELEMENTSPROC)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXIMAGE3DPROC)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXSUBIMAGE3DPROC)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels);

// Extension: 1.3
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLACTIVETEXTUREPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_ActiveTexture(GLenum texture);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOMPRESSEDTEXIMAGE1DPROC)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_CompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOMPRESSEDTEXIMAGE2DPROC)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOMPRESSEDTEXIMAGE3DPROC)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOMPRESSEDTEXSUBIMAGE1DPROC)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_CompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOMPRESSEDTEXSUBIMAGE2DPROC)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOMPRESSEDTEXSUBIMAGE3DPROC)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETCOMPRESSEDTEXIMAGEPROC)(GLenum, GLint, GLvoid *);
static void CODEGEN_FUNCPTR Switch_GetCompressedTexImage(GLenum target, GLint level, GLvoid * img);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSAMPLECOVERAGEPROC)(GLfloat, GLboolean);
static void CODEGEN_FUNCPTR Switch_SampleCoverage(GLfloat value, GLboolean invert);

// Extension: 1.4
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBLENDFUNCSEPARATEPROC)(GLenum, GLenum, GLenum, GLenum);
static void CODEGEN_FUNCPTR Switch_BlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLMULTIDRAWARRAYSPROC)(GLenum, const GLint *, const GLsizei *, GLsizei);
static void CODEGEN_FUNCPTR Switch_MultiDrawArrays(GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLMULTIDRAWELEMENTSPROC)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei);
static void CODEGEN_FUNCPTR Switch_MultiDrawElements(GLenum mode, const GLsizei * count, GLenum type, const GLvoid *const* indices, GLsizei drawcount);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPOINTPARAMETERFPROC)(GLenum, GLfloat);
static void CODEGEN_FUNCPTR Switch_PointParameterf(GLenum pname, GLfloat param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPOINTPARAMETERFVPROC)(GLenum, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_PointParameterfv(GLenum pname, const GLfloat * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPOINTPARAMETERIPROC)(GLenum, GLint);
static void CODEGEN_FUNCPTR Switch_PointParameteri(GLenum pname, GLint param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPOINTPARAMETERIVPROC)(GLenum, const GLint *);
static void CODEGEN_FUNCPTR Switch_PointParameteriv(GLenum pname, const GLint * params);

// Extension: 1.5
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBEGINQUERYPROC)(GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_BeginQuery(GLenum target, GLuint id);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDBUFFERPROC)(GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_BindBuffer(GLenum target, GLuint buffer);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBUFFERDATAPROC)(GLenum, GLsizeiptr, const GLvoid *, GLenum);
static void CODEGEN_FUNCPTR Switch_BufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBUFFERSUBDATAPROC)(GLenum, GLintptr, GLsizeiptr, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETEBUFFERSPROC)(GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_DeleteBuffers(GLsizei n, const GLuint * buffers);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETEQUERIESPROC)(GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_DeleteQueries(GLsizei n, const GLuint * ids);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLENDQUERYPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_EndQuery(GLenum target);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGENBUFFERSPROC)(GLsizei, GLuint *);
static void CODEGEN_FUNCPTR Switch_GenBuffers(GLsizei n, GLuint * buffers);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGENQUERIESPROC)(GLsizei, GLuint *);
static void CODEGEN_FUNCPTR Switch_GenQueries(GLsizei n, GLuint * ids);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETBUFFERPARAMETERIVPROC)(GLenum, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetBufferParameteriv(GLenum target, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETBUFFERPOINTERVPROC)(GLenum, GLenum, GLvoid **);
static void CODEGEN_FUNCPTR Switch_GetBufferPointerv(GLenum target, GLenum pname, GLvoid ** params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETBUFFERSUBDATAPROC)(GLenum, GLintptr, GLsizeiptr, GLvoid *);
static void CODEGEN_FUNCPTR Switch_GetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETQUERYOBJECTIVPROC)(GLuint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetQueryObjectiv(GLuint id, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETQUERYOBJECTUIVPROC)(GLuint, GLenum, GLuint *);
static void CODEGEN_FUNCPTR Switch_GetQueryObjectuiv(GLuint id, GLenum pname, GLuint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETQUERYIVPROC)(GLenum, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetQueryiv(GLenum target, GLenum pname, GLint * params);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISBUFFERPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsBuffer(GLuint buffer);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISQUERYPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsQuery(GLuint id);
typedef void * (CODEGEN_FUNCPTR *PFN_PTRC_GLMAPBUFFERPROC)(GLenum, GLenum);
static void * CODEGEN_FUNCPTR Switch_MapBuffer(GLenum target, GLenum access);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLUNMAPBUFFERPROC)(GLenum);
static GLboolean CODEGEN_FUNCPTR Switch_UnmapBuffer(GLenum target);

// Extension: 2.0
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLATTACHSHADERPROC)(GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_AttachShader(GLuint program, GLuint shader);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDATTRIBLOCATIONPROC)(GLuint, GLuint, const GLchar *);
static void CODEGEN_FUNCPTR Switch_BindAttribLocation(GLuint program, GLuint index, const GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBLENDEQUATIONSEPARATEPROC)(GLenum, GLenum);
static void CODEGEN_FUNCPTR Switch_BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOMPILESHADERPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_CompileShader(GLuint shader);
typedef GLuint (CODEGEN_FUNCPTR *PFN_PTRC_GLCREATEPROGRAMPROC)();
static GLuint CODEGEN_FUNCPTR Switch_CreateProgram();
typedef GLuint (CODEGEN_FUNCPTR *PFN_PTRC_GLCREATESHADERPROC)(GLenum);
static GLuint CODEGEN_FUNCPTR Switch_CreateShader(GLenum type);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETEPROGRAMPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_DeleteProgram(GLuint program);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETESHADERPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_DeleteShader(GLuint shader);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDETACHSHADERPROC)(GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_DetachShader(GLuint program, GLuint shader);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDISABLEVERTEXATTRIBARRAYPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_DisableVertexAttribArray(GLuint index);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWBUFFERSPROC)(GLsizei, const GLenum *);
static void CODEGEN_FUNCPTR Switch_DrawBuffers(GLsizei n, const GLenum * bufs);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLENABLEVERTEXATTRIBARRAYPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_EnableVertexAttribArray(GLuint index);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETACTIVEATTRIBPROC)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETACTIVEUNIFORMPROC)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETATTACHEDSHADERSPROC)(GLuint, GLsizei, GLsizei *, GLuint *);
static void CODEGEN_FUNCPTR Switch_GetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * shaders);
typedef GLint (CODEGEN_FUNCPTR *PFN_PTRC_GLGETATTRIBLOCATIONPROC)(GLuint, const GLchar *);
static GLint CODEGEN_FUNCPTR Switch_GetAttribLocation(GLuint program, const GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMINFOLOGPROC)(GLuint, GLsizei, GLsizei *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETPROGRAMIVPROC)(GLuint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetProgramiv(GLuint program, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSHADERINFOLOGPROC)(GLuint, GLsizei, GLsizei *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSHADERSOURCEPROC)(GLuint, GLsizei, GLsizei *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetShaderSource(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSHADERIVPROC)(GLuint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetShaderiv(GLuint shader, GLenum pname, GLint * params);
typedef GLint (CODEGEN_FUNCPTR *PFN_PTRC_GLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar *);
static GLint CODEGEN_FUNCPTR Switch_GetUniformLocation(GLuint program, const GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETUNIFORMFVPROC)(GLuint, GLint, GLfloat *);
static void CODEGEN_FUNCPTR Switch_GetUniformfv(GLuint program, GLint location, GLfloat * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETUNIFORMIVPROC)(GLuint, GLint, GLint *);
static void CODEGEN_FUNCPTR Switch_GetUniformiv(GLuint program, GLint location, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETVERTEXATTRIBPOINTERVPROC)(GLuint, GLenum, GLvoid **);
static void CODEGEN_FUNCPTR Switch_GetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid ** pointer);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETVERTEXATTRIBDVPROC)(GLuint, GLenum, GLdouble *);
static void CODEGEN_FUNCPTR Switch_GetVertexAttribdv(GLuint index, GLenum pname, GLdouble * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETVERTEXATTRIBFVPROC)(GLuint, GLenum, GLfloat *);
static void CODEGEN_FUNCPTR Switch_GetVertexAttribfv(GLuint index, GLenum pname, GLfloat * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETVERTEXATTRIBIVPROC)(GLuint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetVertexAttribiv(GLuint index, GLenum pname, GLint * params);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISPROGRAMPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsProgram(GLuint program);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISSHADERPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsShader(GLuint shader);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLLINKPROGRAMPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_LinkProgram(GLuint program);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSHADERSOURCEPROC)(GLuint, GLsizei, const GLchar *const*, const GLint *);
static void CODEGEN_FUNCPTR Switch_ShaderSource(GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSTENCILFUNCSEPARATEPROC)(GLenum, GLenum, GLint, GLuint);
static void CODEGEN_FUNCPTR Switch_StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSTENCILMASKSEPARATEPROC)(GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_StencilMaskSeparate(GLenum face, GLuint mask);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSTENCILOPSEPARATEPROC)(GLenum, GLenum, GLenum, GLenum);
static void CODEGEN_FUNCPTR Switch_StencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM1FPROC)(GLint, GLfloat);
static void CODEGEN_FUNCPTR Switch_Uniform1f(GLint location, GLfloat v0);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM1FVPROC)(GLint, GLsizei, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_Uniform1fv(GLint location, GLsizei count, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM1IPROC)(GLint, GLint);
static void CODEGEN_FUNCPTR Switch_Uniform1i(GLint location, GLint v0);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM1IVPROC)(GLint, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_Uniform1iv(GLint location, GLsizei count, const GLint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM2FPROC)(GLint, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_Uniform2f(GLint location, GLfloat v0, GLfloat v1);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM2FVPROC)(GLint, GLsizei, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_Uniform2fv(GLint location, GLsizei count, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM2IPROC)(GLint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_Uniform2i(GLint location, GLint v0, GLint v1);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM2IVPROC)(GLint, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_Uniform2iv(GLint location, GLsizei count, const GLint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM3FPROC)(GLint, GLfloat, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM3FVPROC)(GLint, GLsizei, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_Uniform3fv(GLint location, GLsizei count, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM3IPROC)(GLint, GLint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_Uniform3i(GLint location, GLint v0, GLint v1, GLint v2);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM3IVPROC)(GLint, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_Uniform3iv(GLint location, GLsizei count, const GLint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM4FPROC)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM4FVPROC)(GLint, GLsizei, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_Uniform4fv(GLint location, GLsizei count, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM4IPROC)(GLint, GLint, GLint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM4IVPROC)(GLint, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_Uniform4iv(GLint location, GLsizei count, const GLint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMMATRIX2FVPROC)(GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMMATRIX3FVPROC)(GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMMATRIX4FVPROC)(GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUSEPROGRAMPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_UseProgram(GLuint program);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVALIDATEPROGRAMPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_ValidateProgram(GLuint program);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB1DPROC)(GLuint, GLdouble);
static void CODEGEN_FUNCPTR Switch_VertexAttrib1d(GLuint index, GLdouble x);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB1DVPROC)(GLuint, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib1dv(GLuint index, const GLdouble * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB1FPROC)(GLuint, GLfloat);
static void CODEGEN_FUNCPTR Switch_VertexAttrib1f(GLuint index, GLfloat x);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB1FVPROC)(GLuint, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib1fv(GLuint index, const GLfloat * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB1SPROC)(GLuint, GLshort);
static void CODEGEN_FUNCPTR Switch_VertexAttrib1s(GLuint index, GLshort x);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB1SVPROC)(GLuint, const GLshort *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib1sv(GLuint index, const GLshort * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB2DPROC)(GLuint, GLdouble, GLdouble);
static void CODEGEN_FUNCPTR Switch_VertexAttrib2d(GLuint index, GLdouble x, GLdouble y);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB2DVPROC)(GLuint, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib2dv(GLuint index, const GLdouble * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB2FPROC)(GLuint, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_VertexAttrib2f(GLuint index, GLfloat x, GLfloat y);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB2FVPROC)(GLuint, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib2fv(GLuint index, const GLfloat * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB2SPROC)(GLuint, GLshort, GLshort);
static void CODEGEN_FUNCPTR Switch_VertexAttrib2s(GLuint index, GLshort x, GLshort y);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB2SVPROC)(GLuint, const GLshort *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib2sv(GLuint index, const GLshort * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB3DPROC)(GLuint, GLdouble, GLdouble, GLdouble);
static void CODEGEN_FUNCPTR Switch_VertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB3DVPROC)(GLuint, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib3dv(GLuint index, const GLdouble * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB3FPROC)(GLuint, GLfloat, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB3FVPROC)(GLuint, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib3fv(GLuint index, const GLfloat * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB3SPROC)(GLuint, GLshort, GLshort, GLshort);
static void CODEGEN_FUNCPTR Switch_VertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB3SVPROC)(GLuint, const GLshort *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib3sv(GLuint index, const GLshort * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4NBVPROC)(GLuint, const GLbyte *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nbv(GLuint index, const GLbyte * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4NIVPROC)(GLuint, const GLint *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4Niv(GLuint index, const GLint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4NSVPROC)(GLuint, const GLshort *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nsv(GLuint index, const GLshort * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4NUBPROC)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4NUBVPROC)(GLuint, const GLubyte *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nubv(GLuint index, const GLubyte * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4NUIVPROC)(GLuint, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nuiv(GLuint index, const GLuint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4NUSVPROC)(GLuint, const GLushort *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nusv(GLuint index, const GLushort * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4BVPROC)(GLuint, const GLbyte *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4bv(GLuint index, const GLbyte * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4DPROC)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4DVPROC)(GLuint, const GLdouble *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4dv(GLuint index, const GLdouble * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4FPROC)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4FVPROC)(GLuint, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4fv(GLuint index, const GLfloat * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4IVPROC)(GLuint, const GLint *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4iv(GLuint index, const GLint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4SPROC)(GLuint, GLshort, GLshort, GLshort, GLshort);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4SVPROC)(GLuint, const GLshort *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4sv(GLuint index, const GLshort * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4UBVPROC)(GLuint, const GLubyte *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4ubv(GLuint index, const GLubyte * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4UIVPROC)(GLuint, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4uiv(GLuint index, const GLuint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIB4USVPROC)(GLuint, const GLushort *);
static void CODEGEN_FUNCPTR Switch_VertexAttrib4usv(GLuint index, const GLushort * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBPOINTERPROC)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);

// Extension: 2.1
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMMATRIX2X3FVPROC)(GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMMATRIX2X4FVPROC)(GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMMATRIX3X2FVPROC)(GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMMATRIX3X4FVPROC)(GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMMATRIX4X2FVPROC)(GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMMATRIX4X3FVPROC)(GLint, GLsizei, GLboolean, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

// Extension: 3.0
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBEGINCONDITIONALRENDERPROC)(GLuint, GLenum);
static void CODEGEN_FUNCPTR Switch_BeginConditionalRender(GLuint id, GLenum mode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBEGINTRANSFORMFEEDBACKPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_BeginTransformFeedback(GLenum primitiveMode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDBUFFERBASEPROC)(GLenum, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_BindBufferBase(GLenum target, GLuint index, GLuint buffer);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDBUFFERRANGEPROC)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
static void CODEGEN_FUNCPTR Switch_BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDFRAGDATALOCATIONPROC)(GLuint, GLuint, const GLchar *);
static void CODEGEN_FUNCPTR Switch_BindFragDataLocation(GLuint program, GLuint color, const GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDFRAMEBUFFERPROC)(GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_BindFramebuffer(GLenum target, GLuint framebuffer);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDRENDERBUFFERPROC)(GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_BindRenderbuffer(GLenum target, GLuint renderbuffer);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDVERTEXARRAYPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_BindVertexArray(GLuint ren_array);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBLITFRAMEBUFFERPROC)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
static void CODEGEN_FUNCPTR Switch_BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef GLenum (CODEGEN_FUNCPTR *PFN_PTRC_GLCHECKFRAMEBUFFERSTATUSPROC)(GLenum);
static GLenum CODEGEN_FUNCPTR Switch_CheckFramebufferStatus(GLenum target);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLAMPCOLORPROC)(GLenum, GLenum);
static void CODEGEN_FUNCPTR Switch_ClampColor(GLenum target, GLenum clamp);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARBUFFERFIPROC)(GLenum, GLint, GLfloat, GLint);
static void CODEGEN_FUNCPTR Switch_ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARBUFFERFVPROC)(GLenum, GLint, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARBUFFERIVPROC)(GLenum, GLint, const GLint *);
static void CODEGEN_FUNCPTR Switch_ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCLEARBUFFERUIVPROC)(GLenum, GLint, const GLuint *);
static void CODEGEN_FUNCPTR Switch_ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOLORMASKIPROC)(GLuint, GLboolean, GLboolean, GLboolean, GLboolean);
static void CODEGEN_FUNCPTR Switch_ColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETEFRAMEBUFFERSPROC)(GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_DeleteFramebuffers(GLsizei n, const GLuint * framebuffers);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETERENDERBUFFERSPROC)(GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_DeleteRenderbuffers(GLsizei n, const GLuint * renderbuffers);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETEVERTEXARRAYSPROC)(GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_DeleteVertexArrays(GLsizei n, const GLuint * arrays);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDISABLEIPROC)(GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_Disablei(GLenum target, GLuint index);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLENABLEIPROC)(GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_Enablei(GLenum target, GLuint index);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLENDCONDITIONALRENDERPROC)();
static void CODEGEN_FUNCPTR Switch_EndConditionalRender();
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLENDTRANSFORMFEEDBACKPROC)();
static void CODEGEN_FUNCPTR Switch_EndTransformFeedback();
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFLUSHMAPPEDBUFFERRANGEPROC)(GLenum, GLintptr, GLsizeiptr);
static void CODEGEN_FUNCPTR Switch_FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFRAMEBUFFERRENDERBUFFERPROC)(GLenum, GLenum, GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFRAMEBUFFERTEXTURE1DPROC)(GLenum, GLenum, GLenum, GLuint, GLint);
static void CODEGEN_FUNCPTR Switch_FramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFRAMEBUFFERTEXTURE2DPROC)(GLenum, GLenum, GLenum, GLuint, GLint);
static void CODEGEN_FUNCPTR Switch_FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFRAMEBUFFERTEXTURE3DPROC)(GLenum, GLenum, GLenum, GLuint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_FramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFRAMEBUFFERTEXTURELAYERPROC)(GLenum, GLenum, GLuint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGENFRAMEBUFFERSPROC)(GLsizei, GLuint *);
static void CODEGEN_FUNCPTR Switch_GenFramebuffers(GLsizei n, GLuint * framebuffers);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGENRENDERBUFFERSPROC)(GLsizei, GLuint *);
static void CODEGEN_FUNCPTR Switch_GenRenderbuffers(GLsizei n, GLuint * renderbuffers);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGENVERTEXARRAYSPROC)(GLsizei, GLuint *);
static void CODEGEN_FUNCPTR Switch_GenVertexArrays(GLsizei n, GLuint * arrays);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGENERATEMIPMAPPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_GenerateMipmap(GLenum target);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETBOOLEANI_VPROC)(GLenum, GLuint, GLboolean *);
static void CODEGEN_FUNCPTR Switch_GetBooleani_v(GLenum target, GLuint index, GLboolean * data);
typedef GLint (CODEGEN_FUNCPTR *PFN_PTRC_GLGETFRAGDATALOCATIONPROC)(GLuint, const GLchar *);
static GLint CODEGEN_FUNCPTR Switch_GetFragDataLocation(GLuint program, const GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)(GLenum, GLenum, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETINTEGERI_VPROC)(GLenum, GLuint, GLint *);
static void CODEGEN_FUNCPTR Switch_GetIntegeri_v(GLenum target, GLuint index, GLint * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETRENDERBUFFERPARAMETERIVPROC)(GLenum, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint * params);
typedef const GLubyte * (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSTRINGIPROC)(GLenum, GLuint);
static const GLubyte * CODEGEN_FUNCPTR Switch_GetStringi(GLenum name, GLuint index);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETTEXPARAMETERIIVPROC)(GLenum, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetTexParameterIiv(GLenum target, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETTEXPARAMETERIUIVPROC)(GLenum, GLenum, GLuint *);
static void CODEGEN_FUNCPTR Switch_GetTexParameterIuiv(GLenum target, GLenum pname, GLuint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETTRANSFORMFEEDBACKVARYINGPROC)(GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETUNIFORMUIVPROC)(GLuint, GLint, GLuint *);
static void CODEGEN_FUNCPTR Switch_GetUniformuiv(GLuint program, GLint location, GLuint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETVERTEXATTRIBIIVPROC)(GLuint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetVertexAttribIiv(GLuint index, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETVERTEXATTRIBIUIVPROC)(GLuint, GLenum, GLuint *);
static void CODEGEN_FUNCPTR Switch_GetVertexAttribIuiv(GLuint index, GLenum pname, GLuint * params);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISENABLEDIPROC)(GLenum, GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsEnabledi(GLenum target, GLuint index);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISFRAMEBUFFERPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsFramebuffer(GLuint framebuffer);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISRENDERBUFFERPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsRenderbuffer(GLuint renderbuffer);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISVERTEXARRAYPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsVertexArray(GLuint ren_array);
typedef void * (CODEGEN_FUNCPTR *PFN_PTRC_GLMAPBUFFERRANGEPROC)(GLenum, GLintptr, GLsizeiptr, GLbitfield);
static void * CODEGEN_FUNCPTR Switch_MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLRENDERBUFFERSTORAGEPROC)(GLenum, GLenum, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLRENDERBUFFERSTORAGEMULTISAMPLEPROC)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXPARAMETERIIVPROC)(GLenum, GLenum, const GLint *);
static void CODEGEN_FUNCPTR Switch_TexParameterIiv(GLenum target, GLenum pname, const GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXPARAMETERIUIVPROC)(GLenum, GLenum, const GLuint *);
static void CODEGEN_FUNCPTR Switch_TexParameterIuiv(GLenum target, GLenum pname, const GLuint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTRANSFORMFEEDBACKVARYINGSPROC)(GLuint, GLsizei, const GLchar *const*, GLenum);
static void CODEGEN_FUNCPTR Switch_TransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM1UIPROC)(GLint, GLuint);
static void CODEGEN_FUNCPTR Switch_Uniform1ui(GLint location, GLuint v0);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM1UIVPROC)(GLint, GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_Uniform1uiv(GLint location, GLsizei count, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM2UIPROC)(GLint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_Uniform2ui(GLint location, GLuint v0, GLuint v1);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM2UIVPROC)(GLint, GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_Uniform2uiv(GLint location, GLsizei count, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM3UIPROC)(GLint, GLuint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM3UIVPROC)(GLint, GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_Uniform3uiv(GLint location, GLsizei count, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM4UIPROC)(GLint, GLuint, GLuint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORM4UIVPROC)(GLint, GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_Uniform4uiv(GLint location, GLsizei count, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI1IPROC)(GLuint, GLint);
static void CODEGEN_FUNCPTR Switch_VertexAttribI1i(GLuint index, GLint x);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI1IVPROC)(GLuint, const GLint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI1iv(GLuint index, const GLint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI1UIPROC)(GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribI1ui(GLuint index, GLuint x);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI1UIVPROC)(GLuint, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI1uiv(GLuint index, const GLuint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI2IPROC)(GLuint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_VertexAttribI2i(GLuint index, GLint x, GLint y);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI2IVPROC)(GLuint, const GLint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI2iv(GLuint index, const GLint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI2UIPROC)(GLuint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribI2ui(GLuint index, GLuint x, GLuint y);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI2UIVPROC)(GLuint, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI2uiv(GLuint index, const GLuint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI3IPROC)(GLuint, GLint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_VertexAttribI3i(GLuint index, GLint x, GLint y, GLint z);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI3IVPROC)(GLuint, const GLint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI3iv(GLuint index, const GLint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI3UIPROC)(GLuint, GLuint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI3UIVPROC)(GLuint, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI3uiv(GLuint index, const GLuint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI4BVPROC)(GLuint, const GLbyte *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI4bv(GLuint index, const GLbyte * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI4IPROC)(GLuint, GLint, GLint, GLint, GLint);
static void CODEGEN_FUNCPTR Switch_VertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI4IVPROC)(GLuint, const GLint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI4iv(GLuint index, const GLint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI4SVPROC)(GLuint, const GLshort *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI4sv(GLuint index, const GLshort * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI4UBVPROC)(GLuint, const GLubyte *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI4ubv(GLuint index, const GLubyte * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI4UIPROC)(GLuint, GLuint, GLuint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI4UIVPROC)(GLuint, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI4uiv(GLuint index, const GLuint * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBI4USVPROC)(GLuint, const GLushort *);
static void CODEGEN_FUNCPTR Switch_VertexAttribI4usv(GLuint index, const GLushort * v);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBIPOINTERPROC)(GLuint, GLint, GLenum, GLsizei, const GLvoid *);
static void CODEGEN_FUNCPTR Switch_VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);

// Extension: 3.1
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLCOPYBUFFERSUBDATAPROC)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
static void CODEGEN_FUNCPTR Switch_CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWARRAYSINSTANCEDPROC)(GLenum, GLint, GLsizei, GLsizei);
static void CODEGEN_FUNCPTR Switch_DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWELEMENTSINSTANCEDPROC)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei);
static void CODEGEN_FUNCPTR Switch_DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei instancecount);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETACTIVEUNIFORMBLOCKNAMEPROC)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETACTIVEUNIFORMBLOCKIVPROC)(GLuint, GLuint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETACTIVEUNIFORMNAMEPROC)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *);
static void CODEGEN_FUNCPTR Switch_GetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETACTIVEUNIFORMSIVPROC)(GLuint, GLsizei, const GLuint *, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params);
typedef GLuint (CODEGEN_FUNCPTR *PFN_PTRC_GLGETUNIFORMBLOCKINDEXPROC)(GLuint, const GLchar *);
static GLuint CODEGEN_FUNCPTR Switch_GetUniformBlockIndex(GLuint program, const GLchar * uniformBlockName);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETUNIFORMINDICESPROC)(GLuint, GLsizei, const GLchar *const*, GLuint *);
static void CODEGEN_FUNCPTR Switch_GetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint * uniformIndices);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPRIMITIVERESTARTINDEXPROC)(GLuint);
static void CODEGEN_FUNCPTR Switch_PrimitiveRestartIndex(GLuint index);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXBUFFERPROC)(GLenum, GLenum, GLuint);
static void CODEGEN_FUNCPTR Switch_TexBuffer(GLenum target, GLenum internalformat, GLuint buffer);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLUNIFORMBLOCKBINDINGPROC)(GLuint, GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);

// Extension: 3.2
typedef GLenum (CODEGEN_FUNCPTR *PFN_PTRC_GLCLIENTWAITSYNCPROC)(GLsync, GLbitfield, GLuint64);
static GLenum CODEGEN_FUNCPTR Switch_ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETESYNCPROC)(GLsync);
static void CODEGEN_FUNCPTR Switch_DeleteSync(GLsync sync);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWELEMENTSBASEVERTEXPROC)(GLenum, GLsizei, GLenum, const GLvoid *, GLint);
static void CODEGEN_FUNCPTR Switch_DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei, GLint);
static void CODEGEN_FUNCPTR Switch_DrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei instancecount, GLint basevertex);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDRAWRANGEELEMENTSBASEVERTEXPROC)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *, GLint);
static void CODEGEN_FUNCPTR Switch_DrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex);
typedef GLsync (CODEGEN_FUNCPTR *PFN_PTRC_GLFENCESYNCPROC)(GLenum, GLbitfield);
static GLsync CODEGEN_FUNCPTR Switch_FenceSync(GLenum condition, GLbitfield flags);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLFRAMEBUFFERTEXTUREPROC)(GLenum, GLenum, GLuint, GLint);
static void CODEGEN_FUNCPTR Switch_FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETBUFFERPARAMETERI64VPROC)(GLenum, GLenum, GLint64 *);
static void CODEGEN_FUNCPTR Switch_GetBufferParameteri64v(GLenum target, GLenum pname, GLint64 * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETINTEGER64I_VPROC)(GLenum, GLuint, GLint64 *);
static void CODEGEN_FUNCPTR Switch_GetInteger64i_v(GLenum target, GLuint index, GLint64 * data);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETINTEGER64VPROC)(GLenum, GLint64 *);
static void CODEGEN_FUNCPTR Switch_GetInteger64v(GLenum pname, GLint64 * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETMULTISAMPLEFVPROC)(GLenum, GLuint, GLfloat *);
static void CODEGEN_FUNCPTR Switch_GetMultisamplefv(GLenum pname, GLuint index, GLfloat * val);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSYNCIVPROC)(GLsync, GLenum, GLsizei, GLsizei *, GLint *);
static void CODEGEN_FUNCPTR Switch_GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISSYNCPROC)(GLsync);
static GLboolean CODEGEN_FUNCPTR Switch_IsSync(GLsync sync);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLMULTIDRAWELEMENTSBASEVERTEXPROC)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei, const GLint *);
static void CODEGEN_FUNCPTR Switch_MultiDrawElementsBaseVertex(GLenum mode, const GLsizei * count, GLenum type, const GLvoid *const* indices, GLsizei drawcount, const GLint * basevertex);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLPROVOKINGVERTEXPROC)(GLenum);
static void CODEGEN_FUNCPTR Switch_ProvokingVertex(GLenum mode);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSAMPLEMASKIPROC)(GLuint, GLbitfield);
static void CODEGEN_FUNCPTR Switch_SampleMaski(GLuint index, GLbitfield mask);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXIMAGE2DMULTISAMPLEPROC)(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLboolean);
static void CODEGEN_FUNCPTR Switch_TexImage2DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLTEXIMAGE3DMULTISAMPLEPROC)(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLsizei, GLboolean);
static void CODEGEN_FUNCPTR Switch_TexImage3DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLWAITSYNCPROC)(GLsync, GLbitfield, GLuint64);
static void CODEGEN_FUNCPTR Switch_WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);

// Extension: 3.3
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDFRAGDATALOCATIONINDEXEDPROC)(GLuint, GLuint, GLuint, const GLchar *);
static void CODEGEN_FUNCPTR Switch_BindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLBINDSAMPLERPROC)(GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_BindSampler(GLuint unit, GLuint sampler);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLDELETESAMPLERSPROC)(GLsizei, const GLuint *);
static void CODEGEN_FUNCPTR Switch_DeleteSamplers(GLsizei count, const GLuint * samplers);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGENSAMPLERSPROC)(GLsizei, GLuint *);
static void CODEGEN_FUNCPTR Switch_GenSamplers(GLsizei count, GLuint * samplers);
typedef GLint (CODEGEN_FUNCPTR *PFN_PTRC_GLGETFRAGDATAINDEXPROC)(GLuint, const GLchar *);
static GLint CODEGEN_FUNCPTR Switch_GetFragDataIndex(GLuint program, const GLchar * name);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETQUERYOBJECTI64VPROC)(GLuint, GLenum, GLint64 *);
static void CODEGEN_FUNCPTR Switch_GetQueryObjecti64v(GLuint id, GLenum pname, GLint64 * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETQUERYOBJECTUI64VPROC)(GLuint, GLenum, GLuint64 *);
static void CODEGEN_FUNCPTR Switch_GetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSAMPLERPARAMETERIIVPROC)(GLuint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSAMPLERPARAMETERIUIVPROC)(GLuint, GLenum, GLuint *);
static void CODEGEN_FUNCPTR Switch_GetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSAMPLERPARAMETERFVPROC)(GLuint, GLenum, GLfloat *);
static void CODEGEN_FUNCPTR Switch_GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat * params);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLGETSAMPLERPARAMETERIVPROC)(GLuint, GLenum, GLint *);
static void CODEGEN_FUNCPTR Switch_GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint * params);
typedef GLboolean (CODEGEN_FUNCPTR *PFN_PTRC_GLISSAMPLERPROC)(GLuint);
static GLboolean CODEGEN_FUNCPTR Switch_IsSampler(GLuint sampler);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLQUERYCOUNTERPROC)(GLuint, GLenum);
static void CODEGEN_FUNCPTR Switch_QueryCounter(GLuint id, GLenum target);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSAMPLERPARAMETERIIVPROC)(GLuint, GLenum, const GLint *);
static void CODEGEN_FUNCPTR Switch_SamplerParameterIiv(GLuint sampler, GLenum pname, const GLint * param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSAMPLERPARAMETERIUIVPROC)(GLuint, GLenum, const GLuint *);
static void CODEGEN_FUNCPTR Switch_SamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint * param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSAMPLERPARAMETERFPROC)(GLuint, GLenum, GLfloat);
static void CODEGEN_FUNCPTR Switch_SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSAMPLERPARAMETERFVPROC)(GLuint, GLenum, const GLfloat *);
static void CODEGEN_FUNCPTR Switch_SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat * param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSAMPLERPARAMETERIPROC)(GLuint, GLenum, GLint);
static void CODEGEN_FUNCPTR Switch_SamplerParameteri(GLuint sampler, GLenum pname, GLint param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLSAMPLERPARAMETERIVPROC)(GLuint, GLenum, const GLint *);
static void CODEGEN_FUNCPTR Switch_SamplerParameteriv(GLuint sampler, GLenum pname, const GLint * param);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBDIVISORPROC)(GLuint, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribDivisor(GLuint index, GLuint divisor);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBP1UIPROC)(GLuint, GLenum, GLboolean, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBP1UIVPROC)(GLuint, GLenum, GLboolean, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBP2UIPROC)(GLuint, GLenum, GLboolean, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBP2UIVPROC)(GLuint, GLenum, GLboolean, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBP3UIPROC)(GLuint, GLenum, GLboolean, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBP3UIVPROC)(GLuint, GLenum, GLboolean, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBP4UIPROC)(GLuint, GLenum, GLboolean, GLuint);
static void CODEGEN_FUNCPTR Switch_VertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void (CODEGEN_FUNCPTR *PFN_PTRC_GLVERTEXATTRIBP4UIVPROC)(GLuint, GLenum, GLboolean, const GLuint *);
static void CODEGEN_FUNCPTR Switch_VertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value);


// Extension: ARB_ES2_compatibility
PFN_PTRC_GLCLEARDEPTHFPROC _ptrc_glClearDepthf = Switch_ClearDepthf;
PFN_PTRC_GLDEPTHRANGEFPROC _ptrc_glDepthRangef = Switch_DepthRangef;
PFN_PTRC_GLGETSHADERPRECISIONFORMATPROC _ptrc_glGetShaderPrecisionFormat = Switch_GetShaderPrecisionFormat;
PFN_PTRC_GLRELEASESHADERCOMPILERPROC _ptrc_glReleaseShaderCompiler = Switch_ReleaseShaderCompiler;
PFN_PTRC_GLSHADERBINARYPROC _ptrc_glShaderBinary = Switch_ShaderBinary;

// Extension: ARB_get_program_binary
PFN_PTRC_GLGETPROGRAMBINARYPROC _ptrc_glGetProgramBinary = Switch_GetProgramBinary;
PFN_PTRC_GLPROGRAMBINARYPROC _ptrc_glProgramBinary = Switch_ProgramBinary;
PFN_PTRC_GLPROGRAMPARAMETERIPROC _ptrc_glProgramParameteri = Switch_ProgramParameteri;

// Extension: ARB_internalformat_query
PFN_PTRC_GLGETINTERNALFORMATIVPROC _ptrc_glGetInternalformativ = Switch_GetInternalformativ;

// Extension: ARB_internalformat_query2
PFN_PTRC_GLGETINTERNALFORMATI64VPROC _ptrc_glGetInternalformati64v = Switch_GetInternalformati64v;

// Extension: ARB_program_interface_query
PFN_PTRC_GLGETPROGRAMINTERFACEIVPROC _ptrc_glGetProgramInterfaceiv = Switch_GetProgramInterfaceiv;
PFN_PTRC_GLGETPROGRAMRESOURCEINDEXPROC _ptrc_glGetProgramResourceIndex = Switch_GetProgramResourceIndex;
PFN_PTRC_GLGETPROGRAMRESOURCELOCATIONPROC _ptrc_glGetProgramResourceLocation = Switch_GetProgramResourceLocation;
PFN_PTRC_GLGETPROGRAMRESOURCELOCATIONINDEXPROC _ptrc_glGetProgramResourceLocationIndex = Switch_GetProgramResourceLocationIndex;
PFN_PTRC_GLGETPROGRAMRESOURCENAMEPROC _ptrc_glGetProgramResourceName = Switch_GetProgramResourceName;
PFN_PTRC_GLGETPROGRAMRESOURCEIVPROC _ptrc_glGetProgramResourceiv = Switch_GetProgramResourceiv;

// Extension: ARB_separate_shader_objects
PFN_PTRC_GLACTIVESHADERPROGRAMPROC _ptrc_glActiveShaderProgram = Switch_ActiveShaderProgram;
PFN_PTRC_GLBINDPROGRAMPIPELINEPROC _ptrc_glBindProgramPipeline = Switch_BindProgramPipeline;
PFN_PTRC_GLCREATESHADERPROGRAMVPROC _ptrc_glCreateShaderProgramv = Switch_CreateShaderProgramv;
PFN_PTRC_GLDELETEPROGRAMPIPELINESPROC _ptrc_glDeleteProgramPipelines = Switch_DeleteProgramPipelines;
PFN_PTRC_GLGENPROGRAMPIPELINESPROC _ptrc_glGenProgramPipelines = Switch_GenProgramPipelines;
PFN_PTRC_GLGETPROGRAMPIPELINEINFOLOGPROC _ptrc_glGetProgramPipelineInfoLog = Switch_GetProgramPipelineInfoLog;
PFN_PTRC_GLGETPROGRAMPIPELINEIVPROC _ptrc_glGetProgramPipelineiv = Switch_GetProgramPipelineiv;
PFN_PTRC_GLISPROGRAMPIPELINEPROC _ptrc_glIsProgramPipeline = Switch_IsProgramPipeline;
PFN_PTRC_GLPROGRAMUNIFORM1DPROC _ptrc_glProgramUniform1d = Switch_ProgramUniform1d;
PFN_PTRC_GLPROGRAMUNIFORM1DVPROC _ptrc_glProgramUniform1dv = Switch_ProgramUniform1dv;
PFN_PTRC_GLPROGRAMUNIFORM1FPROC _ptrc_glProgramUniform1f = Switch_ProgramUniform1f;
PFN_PTRC_GLPROGRAMUNIFORM1FVPROC _ptrc_glProgramUniform1fv = Switch_ProgramUniform1fv;
PFN_PTRC_GLPROGRAMUNIFORM1IPROC _ptrc_glProgramUniform1i = Switch_ProgramUniform1i;
PFN_PTRC_GLPROGRAMUNIFORM1IVPROC _ptrc_glProgramUniform1iv = Switch_ProgramUniform1iv;
PFN_PTRC_GLPROGRAMUNIFORM1UIPROC _ptrc_glProgramUniform1ui = Switch_ProgramUniform1ui;
PFN_PTRC_GLPROGRAMUNIFORM1UIVPROC _ptrc_glProgramUniform1uiv = Switch_ProgramUniform1uiv;
PFN_PTRC_GLPROGRAMUNIFORM2DPROC _ptrc_glProgramUniform2d = Switch_ProgramUniform2d;
PFN_PTRC_GLPROGRAMUNIFORM2DVPROC _ptrc_glProgramUniform2dv = Switch_ProgramUniform2dv;
PFN_PTRC_GLPROGRAMUNIFORM2FPROC _ptrc_glProgramUniform2f = Switch_ProgramUniform2f;
PFN_PTRC_GLPROGRAMUNIFORM2FVPROC _ptrc_glProgramUniform2fv = Switch_ProgramUniform2fv;
PFN_PTRC_GLPROGRAMUNIFORM2IPROC _ptrc_glProgramUniform2i = Switch_ProgramUniform2i;
PFN_PTRC_GLPROGRAMUNIFORM2IVPROC _ptrc_glProgramUniform2iv = Switch_ProgramUniform2iv;
PFN_PTRC_GLPROGRAMUNIFORM2UIPROC _ptrc_glProgramUniform2ui = Switch_ProgramUniform2ui;
PFN_PTRC_GLPROGRAMUNIFORM2UIVPROC _ptrc_glProgramUniform2uiv = Switch_ProgramUniform2uiv;
PFN_PTRC_GLPROGRAMUNIFORM3DPROC _ptrc_glProgramUniform3d = Switch_ProgramUniform3d;
PFN_PTRC_GLPROGRAMUNIFORM3DVPROC _ptrc_glProgramUniform3dv = Switch_ProgramUniform3dv;
PFN_PTRC_GLPROGRAMUNIFORM3FPROC _ptrc_glProgramUniform3f = Switch_ProgramUniform3f;
PFN_PTRC_GLPROGRAMUNIFORM3FVPROC _ptrc_glProgramUniform3fv = Switch_ProgramUniform3fv;
PFN_PTRC_GLPROGRAMUNIFORM3IPROC _ptrc_glProgramUniform3i = Switch_ProgramUniform3i;
PFN_PTRC_GLPROGRAMUNIFORM3IVPROC _ptrc_glProgramUniform3iv = Switch_ProgramUniform3iv;
PFN_PTRC_GLPROGRAMUNIFORM3UIPROC _ptrc_glProgramUniform3ui = Switch_ProgramUniform3ui;
PFN_PTRC_GLPROGRAMUNIFORM3UIVPROC _ptrc_glProgramUniform3uiv = Switch_ProgramUniform3uiv;
PFN_PTRC_GLPROGRAMUNIFORM4DPROC _ptrc_glProgramUniform4d = Switch_ProgramUniform4d;
PFN_PTRC_GLPROGRAMUNIFORM4DVPROC _ptrc_glProgramUniform4dv = Switch_ProgramUniform4dv;
PFN_PTRC_GLPROGRAMUNIFORM4FPROC _ptrc_glProgramUniform4f = Switch_ProgramUniform4f;
PFN_PTRC_GLPROGRAMUNIFORM4FVPROC _ptrc_glProgramUniform4fv = Switch_ProgramUniform4fv;
PFN_PTRC_GLPROGRAMUNIFORM4IPROC _ptrc_glProgramUniform4i = Switch_ProgramUniform4i;
PFN_PTRC_GLPROGRAMUNIFORM4IVPROC _ptrc_glProgramUniform4iv = Switch_ProgramUniform4iv;
PFN_PTRC_GLPROGRAMUNIFORM4UIPROC _ptrc_glProgramUniform4ui = Switch_ProgramUniform4ui;
PFN_PTRC_GLPROGRAMUNIFORM4UIVPROC _ptrc_glProgramUniform4uiv = Switch_ProgramUniform4uiv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX2DVPROC _ptrc_glProgramUniformMatrix2dv = Switch_ProgramUniformMatrix2dv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX2FVPROC _ptrc_glProgramUniformMatrix2fv = Switch_ProgramUniformMatrix2fv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X3DVPROC _ptrc_glProgramUniformMatrix2x3dv = Switch_ProgramUniformMatrix2x3dv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X3FVPROC _ptrc_glProgramUniformMatrix2x3fv = Switch_ProgramUniformMatrix2x3fv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X4DVPROC _ptrc_glProgramUniformMatrix2x4dv = Switch_ProgramUniformMatrix2x4dv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X4FVPROC _ptrc_glProgramUniformMatrix2x4fv = Switch_ProgramUniformMatrix2x4fv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX3DVPROC _ptrc_glProgramUniformMatrix3dv = Switch_ProgramUniformMatrix3dv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX3FVPROC _ptrc_glProgramUniformMatrix3fv = Switch_ProgramUniformMatrix3fv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X2DVPROC _ptrc_glProgramUniformMatrix3x2dv = Switch_ProgramUniformMatrix3x2dv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X2FVPROC _ptrc_glProgramUniformMatrix3x2fv = Switch_ProgramUniformMatrix3x2fv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X4DVPROC _ptrc_glProgramUniformMatrix3x4dv = Switch_ProgramUniformMatrix3x4dv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X4FVPROC _ptrc_glProgramUniformMatrix3x4fv = Switch_ProgramUniformMatrix3x4fv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX4DVPROC _ptrc_glProgramUniformMatrix4dv = Switch_ProgramUniformMatrix4dv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX4FVPROC _ptrc_glProgramUniformMatrix4fv = Switch_ProgramUniformMatrix4fv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X2DVPROC _ptrc_glProgramUniformMatrix4x2dv = Switch_ProgramUniformMatrix4x2dv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X2FVPROC _ptrc_glProgramUniformMatrix4x2fv = Switch_ProgramUniformMatrix4x2fv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X3DVPROC _ptrc_glProgramUniformMatrix4x3dv = Switch_ProgramUniformMatrix4x3dv;
PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X3FVPROC _ptrc_glProgramUniformMatrix4x3fv = Switch_ProgramUniformMatrix4x3fv;
PFN_PTRC_GLUSEPROGRAMSTAGESPROC _ptrc_glUseProgramStages = Switch_UseProgramStages;
PFN_PTRC_GLVALIDATEPROGRAMPIPELINEPROC _ptrc_glValidateProgramPipeline = Switch_ValidateProgramPipeline;

// Extension: ARB_texture_buffer_range
PFN_PTRC_GLTEXBUFFERRANGEPROC _ptrc_glTexBufferRange = Switch_TexBufferRange;

// Extension: ARB_texture_storage
PFN_PTRC_GLTEXSTORAGE1DPROC _ptrc_glTexStorage1D = Switch_TexStorage1D;
PFN_PTRC_GLTEXSTORAGE2DPROC _ptrc_glTexStorage2D = Switch_TexStorage2D;
PFN_PTRC_GLTEXSTORAGE3DPROC _ptrc_glTexStorage3D = Switch_TexStorage3D;

// Extension: ARB_texture_view
PFN_PTRC_GLTEXTUREVIEWPROC _ptrc_glTextureView = Switch_TextureView;

// Extension: ARB_vertex_attrib_binding
PFN_PTRC_GLBINDVERTEXBUFFERPROC _ptrc_glBindVertexBuffer = Switch_BindVertexBuffer;
PFN_PTRC_GLVERTEXATTRIBBINDINGPROC _ptrc_glVertexAttribBinding = Switch_VertexAttribBinding;
PFN_PTRC_GLVERTEXATTRIBFORMATPROC _ptrc_glVertexAttribFormat = Switch_VertexAttribFormat;
PFN_PTRC_GLVERTEXATTRIBIFORMATPROC _ptrc_glVertexAttribIFormat = Switch_VertexAttribIFormat;
PFN_PTRC_GLVERTEXATTRIBLFORMATPROC _ptrc_glVertexAttribLFormat = Switch_VertexAttribLFormat;
PFN_PTRC_GLVERTEXBINDINGDIVISORPROC _ptrc_glVertexBindingDivisor = Switch_VertexBindingDivisor;

// Extension: ARB_viewport_array
PFN_PTRC_GLDEPTHRANGEARRAYVPROC _ptrc_glDepthRangeArrayv = Switch_DepthRangeArrayv;
PFN_PTRC_GLDEPTHRANGEINDEXEDPROC _ptrc_glDepthRangeIndexed = Switch_DepthRangeIndexed;
PFN_PTRC_GLGETDOUBLEI_VPROC _ptrc_glGetDoublei_v = Switch_GetDoublei_v;
PFN_PTRC_GLGETFLOATI_VPROC _ptrc_glGetFloati_v = Switch_GetFloati_v;
PFN_PTRC_GLSCISSORARRAYVPROC _ptrc_glScissorArrayv = Switch_ScissorArrayv;
PFN_PTRC_GLSCISSORINDEXEDPROC _ptrc_glScissorIndexed = Switch_ScissorIndexed;
PFN_PTRC_GLSCISSORINDEXEDVPROC _ptrc_glScissorIndexedv = Switch_ScissorIndexedv;
PFN_PTRC_GLVIEWPORTARRAYVPROC _ptrc_glViewportArrayv = Switch_ViewportArrayv;
PFN_PTRC_GLVIEWPORTINDEXEDFPROC _ptrc_glViewportIndexedf = Switch_ViewportIndexedf;
PFN_PTRC_GLVIEWPORTINDEXEDFVPROC _ptrc_glViewportIndexedfv = Switch_ViewportIndexedfv;

// Extension: ARB_clear_buffer_object
PFN_PTRC_GLCLEARBUFFERDATAPROC _ptrc_glClearBufferData = Switch_ClearBufferData;
PFN_PTRC_GLCLEARBUFFERSUBDATAPROC _ptrc_glClearBufferSubData = Switch_ClearBufferSubData;

// Extension: ARB_copy_image
PFN_PTRC_GLCOPYIMAGESUBDATAPROC _ptrc_glCopyImageSubData = Switch_CopyImageSubData;

// Extension: ARB_framebuffer_no_attachments
PFN_PTRC_GLFRAMEBUFFERPARAMETERIPROC _ptrc_glFramebufferParameteri = Switch_FramebufferParameteri;
PFN_PTRC_GLGETFRAMEBUFFERPARAMETERIVPROC _ptrc_glGetFramebufferParameteriv = Switch_GetFramebufferParameteriv;

// Extension: ARB_invalidate_subdata
PFN_PTRC_GLINVALIDATEBUFFERDATAPROC _ptrc_glInvalidateBufferData = Switch_InvalidateBufferData;
PFN_PTRC_GLINVALIDATEBUFFERSUBDATAPROC _ptrc_glInvalidateBufferSubData = Switch_InvalidateBufferSubData;
PFN_PTRC_GLINVALIDATEFRAMEBUFFERPROC _ptrc_glInvalidateFramebuffer = Switch_InvalidateFramebuffer;
PFN_PTRC_GLINVALIDATESUBFRAMEBUFFERPROC _ptrc_glInvalidateSubFramebuffer = Switch_InvalidateSubFramebuffer;
PFN_PTRC_GLINVALIDATETEXIMAGEPROC _ptrc_glInvalidateTexImage = Switch_InvalidateTexImage;
PFN_PTRC_GLINVALIDATETEXSUBIMAGEPROC _ptrc_glInvalidateTexSubImage = Switch_InvalidateTexSubImage;

// Extension: ARB_texture_storage_multisample
PFN_PTRC_GLTEXSTORAGE2DMULTISAMPLEPROC _ptrc_glTexStorage2DMultisample = Switch_TexStorage2DMultisample;
PFN_PTRC_GLTEXSTORAGE3DMULTISAMPLEPROC _ptrc_glTexStorage3DMultisample = Switch_TexStorage3DMultisample;

// Extension: KHR_debug
PFN_PTRC_GLDEBUGMESSAGECALLBACKPROC _ptrc_glDebugMessageCallback = Switch_DebugMessageCallback;
PFN_PTRC_GLDEBUGMESSAGECONTROLPROC _ptrc_glDebugMessageControl = Switch_DebugMessageControl;
PFN_PTRC_GLDEBUGMESSAGEINSERTPROC _ptrc_glDebugMessageInsert = Switch_DebugMessageInsert;
PFN_PTRC_GLGETDEBUGMESSAGELOGPROC _ptrc_glGetDebugMessageLog = Switch_GetDebugMessageLog;
PFN_PTRC_GLGETOBJECTLABELPROC _ptrc_glGetObjectLabel = Switch_GetObjectLabel;
PFN_PTRC_GLGETOBJECTPTRLABELPROC _ptrc_glGetObjectPtrLabel = Switch_GetObjectPtrLabel;
PFN_PTRC_GLGETPOINTERVPROC _ptrc_glGetPointerv = Switch_GetPointerv;
PFN_PTRC_GLOBJECTLABELPROC _ptrc_glObjectLabel = Switch_ObjectLabel;
PFN_PTRC_GLOBJECTPTRLABELPROC _ptrc_glObjectPtrLabel = Switch_ObjectPtrLabel;
PFN_PTRC_GLPOPDEBUGGROUPPROC _ptrc_glPopDebugGroup = Switch_PopDebugGroup;
PFN_PTRC_GLPUSHDEBUGGROUPPROC _ptrc_glPushDebugGroup = Switch_PushDebugGroup;

// Extension: 1.0
PFN_PTRC_GLBLENDFUNCPROC _ptrc_glBlendFunc = Switch_BlendFunc;
PFN_PTRC_GLCLEARPROC _ptrc_glClear = Switch_Clear;
PFN_PTRC_GLCLEARCOLORPROC _ptrc_glClearColor = Switch_ClearColor;
PFN_PTRC_GLCLEARDEPTHPROC _ptrc_glClearDepth = Switch_ClearDepth;
PFN_PTRC_GLCLEARSTENCILPROC _ptrc_glClearStencil = Switch_ClearStencil;
PFN_PTRC_GLCOLORMASKPROC _ptrc_glColorMask = Switch_ColorMask;
PFN_PTRC_GLCULLFACEPROC _ptrc_glCullFace = Switch_CullFace;
PFN_PTRC_GLDEPTHFUNCPROC _ptrc_glDepthFunc = Switch_DepthFunc;
PFN_PTRC_GLDEPTHMASKPROC _ptrc_glDepthMask = Switch_DepthMask;
PFN_PTRC_GLDEPTHRANGEPROC _ptrc_glDepthRange = Switch_DepthRange;
PFN_PTRC_GLDISABLEPROC _ptrc_glDisable = Switch_Disable;
PFN_PTRC_GLDRAWBUFFERPROC _ptrc_glDrawBuffer = Switch_DrawBuffer;
PFN_PTRC_GLENABLEPROC _ptrc_glEnable = Switch_Enable;
PFN_PTRC_GLFINISHPROC _ptrc_glFinish = Switch_Finish;
PFN_PTRC_GLFLUSHPROC _ptrc_glFlush = Switch_Flush;
PFN_PTRC_GLFRONTFACEPROC _ptrc_glFrontFace = Switch_FrontFace;
PFN_PTRC_GLGETBOOLEANVPROC _ptrc_glGetBooleanv = Switch_GetBooleanv;
PFN_PTRC_GLGETDOUBLEVPROC _ptrc_glGetDoublev = Switch_GetDoublev;
PFN_PTRC_GLGETERRORPROC _ptrc_glGetError = Switch_GetError;
PFN_PTRC_GLGETFLOATVPROC _ptrc_glGetFloatv = Switch_GetFloatv;
PFN_PTRC_GLGETINTEGERVPROC _ptrc_glGetIntegerv = Switch_GetIntegerv;
PFN_PTRC_GLGETSTRINGPROC _ptrc_glGetString = Switch_GetString;
PFN_PTRC_GLGETTEXIMAGEPROC _ptrc_glGetTexImage = Switch_GetTexImage;
PFN_PTRC_GLGETTEXLEVELPARAMETERFVPROC _ptrc_glGetTexLevelParameterfv = Switch_GetTexLevelParameterfv;
PFN_PTRC_GLGETTEXLEVELPARAMETERIVPROC _ptrc_glGetTexLevelParameteriv = Switch_GetTexLevelParameteriv;
PFN_PTRC_GLGETTEXPARAMETERFVPROC _ptrc_glGetTexParameterfv = Switch_GetTexParameterfv;
PFN_PTRC_GLGETTEXPARAMETERIVPROC _ptrc_glGetTexParameteriv = Switch_GetTexParameteriv;
PFN_PTRC_GLHINTPROC _ptrc_glHint = Switch_Hint;
PFN_PTRC_GLISENABLEDPROC _ptrc_glIsEnabled = Switch_IsEnabled;
PFN_PTRC_GLLINEWIDTHPROC _ptrc_glLineWidth = Switch_LineWidth;
PFN_PTRC_GLLOGICOPPROC _ptrc_glLogicOp = Switch_LogicOp;
PFN_PTRC_GLPIXELSTOREFPROC _ptrc_glPixelStoref = Switch_PixelStoref;
PFN_PTRC_GLPIXELSTOREIPROC _ptrc_glPixelStorei = Switch_PixelStorei;
PFN_PTRC_GLPOINTSIZEPROC _ptrc_glPointSize = Switch_PointSize;
PFN_PTRC_GLPOLYGONMODEPROC _ptrc_glPolygonMode = Switch_PolygonMode;
PFN_PTRC_GLREADBUFFERPROC _ptrc_glReadBuffer = Switch_ReadBuffer;
PFN_PTRC_GLREADPIXELSPROC _ptrc_glReadPixels = Switch_ReadPixels;
PFN_PTRC_GLSCISSORPROC _ptrc_glScissor = Switch_Scissor;
PFN_PTRC_GLSTENCILFUNCPROC _ptrc_glStencilFunc = Switch_StencilFunc;
PFN_PTRC_GLSTENCILMASKPROC _ptrc_glStencilMask = Switch_StencilMask;
PFN_PTRC_GLSTENCILOPPROC _ptrc_glStencilOp = Switch_StencilOp;
PFN_PTRC_GLTEXIMAGE1DPROC _ptrc_glTexImage1D = Switch_TexImage1D;
PFN_PTRC_GLTEXIMAGE2DPROC _ptrc_glTexImage2D = Switch_TexImage2D;
PFN_PTRC_GLTEXPARAMETERFPROC _ptrc_glTexParameterf = Switch_TexParameterf;
PFN_PTRC_GLTEXPARAMETERFVPROC _ptrc_glTexParameterfv = Switch_TexParameterfv;
PFN_PTRC_GLTEXPARAMETERIPROC _ptrc_glTexParameteri = Switch_TexParameteri;
PFN_PTRC_GLTEXPARAMETERIVPROC _ptrc_glTexParameteriv = Switch_TexParameteriv;
PFN_PTRC_GLVIEWPORTPROC _ptrc_glViewport = Switch_Viewport;

// Extension: 1.1
PFN_PTRC_GLBINDTEXTUREPROC _ptrc_glBindTexture = Switch_BindTexture;
PFN_PTRC_GLCOPYTEXIMAGE1DPROC _ptrc_glCopyTexImage1D = Switch_CopyTexImage1D;
PFN_PTRC_GLCOPYTEXIMAGE2DPROC _ptrc_glCopyTexImage2D = Switch_CopyTexImage2D;
PFN_PTRC_GLCOPYTEXSUBIMAGE1DPROC _ptrc_glCopyTexSubImage1D = Switch_CopyTexSubImage1D;
PFN_PTRC_GLCOPYTEXSUBIMAGE2DPROC _ptrc_glCopyTexSubImage2D = Switch_CopyTexSubImage2D;
PFN_PTRC_GLDELETETEXTURESPROC _ptrc_glDeleteTextures = Switch_DeleteTextures;
PFN_PTRC_GLDRAWARRAYSPROC _ptrc_glDrawArrays = Switch_DrawArrays;
PFN_PTRC_GLDRAWELEMENTSPROC _ptrc_glDrawElements = Switch_DrawElements;
PFN_PTRC_GLGENTEXTURESPROC _ptrc_glGenTextures = Switch_GenTextures;
PFN_PTRC_GLISTEXTUREPROC _ptrc_glIsTexture = Switch_IsTexture;
PFN_PTRC_GLPOLYGONOFFSETPROC _ptrc_glPolygonOffset = Switch_PolygonOffset;
PFN_PTRC_GLTEXSUBIMAGE1DPROC _ptrc_glTexSubImage1D = Switch_TexSubImage1D;
PFN_PTRC_GLTEXSUBIMAGE2DPROC _ptrc_glTexSubImage2D = Switch_TexSubImage2D;

// Extension: 1.2
PFN_PTRC_GLBLENDCOLORPROC _ptrc_glBlendColor = Switch_BlendColor;
PFN_PTRC_GLBLENDEQUATIONPROC _ptrc_glBlendEquation = Switch_BlendEquation;
PFN_PTRC_GLCOPYTEXSUBIMAGE3DPROC _ptrc_glCopyTexSubImage3D = Switch_CopyTexSubImage3D;
PFN_PTRC_GLDRAWRANGEELEMENTSPROC _ptrc_glDrawRangeElements = Switch_DrawRangeElements;
PFN_PTRC_GLTEXIMAGE3DPROC _ptrc_glTexImage3D = Switch_TexImage3D;
PFN_PTRC_GLTEXSUBIMAGE3DPROC _ptrc_glTexSubImage3D = Switch_TexSubImage3D;

// Extension: 1.3
PFN_PTRC_GLACTIVETEXTUREPROC _ptrc_glActiveTexture = Switch_ActiveTexture;
PFN_PTRC_GLCOMPRESSEDTEXIMAGE1DPROC _ptrc_glCompressedTexImage1D = Switch_CompressedTexImage1D;
PFN_PTRC_GLCOMPRESSEDTEXIMAGE2DPROC _ptrc_glCompressedTexImage2D = Switch_CompressedTexImage2D;
PFN_PTRC_GLCOMPRESSEDTEXIMAGE3DPROC _ptrc_glCompressedTexImage3D = Switch_CompressedTexImage3D;
PFN_PTRC_GLCOMPRESSEDTEXSUBIMAGE1DPROC _ptrc_glCompressedTexSubImage1D = Switch_CompressedTexSubImage1D;
PFN_PTRC_GLCOMPRESSEDTEXSUBIMAGE2DPROC _ptrc_glCompressedTexSubImage2D = Switch_CompressedTexSubImage2D;
PFN_PTRC_GLCOMPRESSEDTEXSUBIMAGE3DPROC _ptrc_glCompressedTexSubImage3D = Switch_CompressedTexSubImage3D;
PFN_PTRC_GLGETCOMPRESSEDTEXIMAGEPROC _ptrc_glGetCompressedTexImage = Switch_GetCompressedTexImage;
PFN_PTRC_GLSAMPLECOVERAGEPROC _ptrc_glSampleCoverage = Switch_SampleCoverage;

// Extension: 1.4
PFN_PTRC_GLBLENDFUNCSEPARATEPROC _ptrc_glBlendFuncSeparate = Switch_BlendFuncSeparate;
PFN_PTRC_GLMULTIDRAWARRAYSPROC _ptrc_glMultiDrawArrays = Switch_MultiDrawArrays;
PFN_PTRC_GLMULTIDRAWELEMENTSPROC _ptrc_glMultiDrawElements = Switch_MultiDrawElements;
PFN_PTRC_GLPOINTPARAMETERFPROC _ptrc_glPointParameterf = Switch_PointParameterf;
PFN_PTRC_GLPOINTPARAMETERFVPROC _ptrc_glPointParameterfv = Switch_PointParameterfv;
PFN_PTRC_GLPOINTPARAMETERIPROC _ptrc_glPointParameteri = Switch_PointParameteri;
PFN_PTRC_GLPOINTPARAMETERIVPROC _ptrc_glPointParameteriv = Switch_PointParameteriv;

// Extension: 1.5
PFN_PTRC_GLBEGINQUERYPROC _ptrc_glBeginQuery = Switch_BeginQuery;
PFN_PTRC_GLBINDBUFFERPROC _ptrc_glBindBuffer = Switch_BindBuffer;
PFN_PTRC_GLBUFFERDATAPROC _ptrc_glBufferData = Switch_BufferData;
PFN_PTRC_GLBUFFERSUBDATAPROC _ptrc_glBufferSubData = Switch_BufferSubData;
PFN_PTRC_GLDELETEBUFFERSPROC _ptrc_glDeleteBuffers = Switch_DeleteBuffers;
PFN_PTRC_GLDELETEQUERIESPROC _ptrc_glDeleteQueries = Switch_DeleteQueries;
PFN_PTRC_GLENDQUERYPROC _ptrc_glEndQuery = Switch_EndQuery;
PFN_PTRC_GLGENBUFFERSPROC _ptrc_glGenBuffers = Switch_GenBuffers;
PFN_PTRC_GLGENQUERIESPROC _ptrc_glGenQueries = Switch_GenQueries;
PFN_PTRC_GLGETBUFFERPARAMETERIVPROC _ptrc_glGetBufferParameteriv = Switch_GetBufferParameteriv;
PFN_PTRC_GLGETBUFFERPOINTERVPROC _ptrc_glGetBufferPointerv = Switch_GetBufferPointerv;
PFN_PTRC_GLGETBUFFERSUBDATAPROC _ptrc_glGetBufferSubData = Switch_GetBufferSubData;
PFN_PTRC_GLGETQUERYOBJECTIVPROC _ptrc_glGetQueryObjectiv = Switch_GetQueryObjectiv;
PFN_PTRC_GLGETQUERYOBJECTUIVPROC _ptrc_glGetQueryObjectuiv = Switch_GetQueryObjectuiv;
PFN_PTRC_GLGETQUERYIVPROC _ptrc_glGetQueryiv = Switch_GetQueryiv;
PFN_PTRC_GLISBUFFERPROC _ptrc_glIsBuffer = Switch_IsBuffer;
PFN_PTRC_GLISQUERYPROC _ptrc_glIsQuery = Switch_IsQuery;
PFN_PTRC_GLMAPBUFFERPROC _ptrc_glMapBuffer = Switch_MapBuffer;
PFN_PTRC_GLUNMAPBUFFERPROC _ptrc_glUnmapBuffer = Switch_UnmapBuffer;

// Extension: 2.0
PFN_PTRC_GLATTACHSHADERPROC _ptrc_glAttachShader = Switch_AttachShader;
PFN_PTRC_GLBINDATTRIBLOCATIONPROC _ptrc_glBindAttribLocation = Switch_BindAttribLocation;
PFN_PTRC_GLBLENDEQUATIONSEPARATEPROC _ptrc_glBlendEquationSeparate = Switch_BlendEquationSeparate;
PFN_PTRC_GLCOMPILESHADERPROC _ptrc_glCompileShader = Switch_CompileShader;
PFN_PTRC_GLCREATEPROGRAMPROC _ptrc_glCreateProgram = Switch_CreateProgram;
PFN_PTRC_GLCREATESHADERPROC _ptrc_glCreateShader = Switch_CreateShader;
PFN_PTRC_GLDELETEPROGRAMPROC _ptrc_glDeleteProgram = Switch_DeleteProgram;
PFN_PTRC_GLDELETESHADERPROC _ptrc_glDeleteShader = Switch_DeleteShader;
PFN_PTRC_GLDETACHSHADERPROC _ptrc_glDetachShader = Switch_DetachShader;
PFN_PTRC_GLDISABLEVERTEXATTRIBARRAYPROC _ptrc_glDisableVertexAttribArray = Switch_DisableVertexAttribArray;
PFN_PTRC_GLDRAWBUFFERSPROC _ptrc_glDrawBuffers = Switch_DrawBuffers;
PFN_PTRC_GLENABLEVERTEXATTRIBARRAYPROC _ptrc_glEnableVertexAttribArray = Switch_EnableVertexAttribArray;
PFN_PTRC_GLGETACTIVEATTRIBPROC _ptrc_glGetActiveAttrib = Switch_GetActiveAttrib;
PFN_PTRC_GLGETACTIVEUNIFORMPROC _ptrc_glGetActiveUniform = Switch_GetActiveUniform;
PFN_PTRC_GLGETATTACHEDSHADERSPROC _ptrc_glGetAttachedShaders = Switch_GetAttachedShaders;
PFN_PTRC_GLGETATTRIBLOCATIONPROC _ptrc_glGetAttribLocation = Switch_GetAttribLocation;
PFN_PTRC_GLGETPROGRAMINFOLOGPROC _ptrc_glGetProgramInfoLog = Switch_GetProgramInfoLog;
PFN_PTRC_GLGETPROGRAMIVPROC _ptrc_glGetProgramiv = Switch_GetProgramiv;
PFN_PTRC_GLGETSHADERINFOLOGPROC _ptrc_glGetShaderInfoLog = Switch_GetShaderInfoLog;
PFN_PTRC_GLGETSHADERSOURCEPROC _ptrc_glGetShaderSource = Switch_GetShaderSource;
PFN_PTRC_GLGETSHADERIVPROC _ptrc_glGetShaderiv = Switch_GetShaderiv;
PFN_PTRC_GLGETUNIFORMLOCATIONPROC _ptrc_glGetUniformLocation = Switch_GetUniformLocation;
PFN_PTRC_GLGETUNIFORMFVPROC _ptrc_glGetUniformfv = Switch_GetUniformfv;
PFN_PTRC_GLGETUNIFORMIVPROC _ptrc_glGetUniformiv = Switch_GetUniformiv;
PFN_PTRC_GLGETVERTEXATTRIBPOINTERVPROC _ptrc_glGetVertexAttribPointerv = Switch_GetVertexAttribPointerv;
PFN_PTRC_GLGETVERTEXATTRIBDVPROC _ptrc_glGetVertexAttribdv = Switch_GetVertexAttribdv;
PFN_PTRC_GLGETVERTEXATTRIBFVPROC _ptrc_glGetVertexAttribfv = Switch_GetVertexAttribfv;
PFN_PTRC_GLGETVERTEXATTRIBIVPROC _ptrc_glGetVertexAttribiv = Switch_GetVertexAttribiv;
PFN_PTRC_GLISPROGRAMPROC _ptrc_glIsProgram = Switch_IsProgram;
PFN_PTRC_GLISSHADERPROC _ptrc_glIsShader = Switch_IsShader;
PFN_PTRC_GLLINKPROGRAMPROC _ptrc_glLinkProgram = Switch_LinkProgram;
PFN_PTRC_GLSHADERSOURCEPROC _ptrc_glShaderSource = Switch_ShaderSource;
PFN_PTRC_GLSTENCILFUNCSEPARATEPROC _ptrc_glStencilFuncSeparate = Switch_StencilFuncSeparate;
PFN_PTRC_GLSTENCILMASKSEPARATEPROC _ptrc_glStencilMaskSeparate = Switch_StencilMaskSeparate;
PFN_PTRC_GLSTENCILOPSEPARATEPROC _ptrc_glStencilOpSeparate = Switch_StencilOpSeparate;
PFN_PTRC_GLUNIFORM1FPROC _ptrc_glUniform1f = Switch_Uniform1f;
PFN_PTRC_GLUNIFORM1FVPROC _ptrc_glUniform1fv = Switch_Uniform1fv;
PFN_PTRC_GLUNIFORM1IPROC _ptrc_glUniform1i = Switch_Uniform1i;
PFN_PTRC_GLUNIFORM1IVPROC _ptrc_glUniform1iv = Switch_Uniform1iv;
PFN_PTRC_GLUNIFORM2FPROC _ptrc_glUniform2f = Switch_Uniform2f;
PFN_PTRC_GLUNIFORM2FVPROC _ptrc_glUniform2fv = Switch_Uniform2fv;
PFN_PTRC_GLUNIFORM2IPROC _ptrc_glUniform2i = Switch_Uniform2i;
PFN_PTRC_GLUNIFORM2IVPROC _ptrc_glUniform2iv = Switch_Uniform2iv;
PFN_PTRC_GLUNIFORM3FPROC _ptrc_glUniform3f = Switch_Uniform3f;
PFN_PTRC_GLUNIFORM3FVPROC _ptrc_glUniform3fv = Switch_Uniform3fv;
PFN_PTRC_GLUNIFORM3IPROC _ptrc_glUniform3i = Switch_Uniform3i;
PFN_PTRC_GLUNIFORM3IVPROC _ptrc_glUniform3iv = Switch_Uniform3iv;
PFN_PTRC_GLUNIFORM4FPROC _ptrc_glUniform4f = Switch_Uniform4f;
PFN_PTRC_GLUNIFORM4FVPROC _ptrc_glUniform4fv = Switch_Uniform4fv;
PFN_PTRC_GLUNIFORM4IPROC _ptrc_glUniform4i = Switch_Uniform4i;
PFN_PTRC_GLUNIFORM4IVPROC _ptrc_glUniform4iv = Switch_Uniform4iv;
PFN_PTRC_GLUNIFORMMATRIX2FVPROC _ptrc_glUniformMatrix2fv = Switch_UniformMatrix2fv;
PFN_PTRC_GLUNIFORMMATRIX3FVPROC _ptrc_glUniformMatrix3fv = Switch_UniformMatrix3fv;
PFN_PTRC_GLUNIFORMMATRIX4FVPROC _ptrc_glUniformMatrix4fv = Switch_UniformMatrix4fv;
PFN_PTRC_GLUSEPROGRAMPROC _ptrc_glUseProgram = Switch_UseProgram;
PFN_PTRC_GLVALIDATEPROGRAMPROC _ptrc_glValidateProgram = Switch_ValidateProgram;
PFN_PTRC_GLVERTEXATTRIB1DPROC _ptrc_glVertexAttrib1d = Switch_VertexAttrib1d;
PFN_PTRC_GLVERTEXATTRIB1DVPROC _ptrc_glVertexAttrib1dv = Switch_VertexAttrib1dv;
PFN_PTRC_GLVERTEXATTRIB1FPROC _ptrc_glVertexAttrib1f = Switch_VertexAttrib1f;
PFN_PTRC_GLVERTEXATTRIB1FVPROC _ptrc_glVertexAttrib1fv = Switch_VertexAttrib1fv;
PFN_PTRC_GLVERTEXATTRIB1SPROC _ptrc_glVertexAttrib1s = Switch_VertexAttrib1s;
PFN_PTRC_GLVERTEXATTRIB1SVPROC _ptrc_glVertexAttrib1sv = Switch_VertexAttrib1sv;
PFN_PTRC_GLVERTEXATTRIB2DPROC _ptrc_glVertexAttrib2d = Switch_VertexAttrib2d;
PFN_PTRC_GLVERTEXATTRIB2DVPROC _ptrc_glVertexAttrib2dv = Switch_VertexAttrib2dv;
PFN_PTRC_GLVERTEXATTRIB2FPROC _ptrc_glVertexAttrib2f = Switch_VertexAttrib2f;
PFN_PTRC_GLVERTEXATTRIB2FVPROC _ptrc_glVertexAttrib2fv = Switch_VertexAttrib2fv;
PFN_PTRC_GLVERTEXATTRIB2SPROC _ptrc_glVertexAttrib2s = Switch_VertexAttrib2s;
PFN_PTRC_GLVERTEXATTRIB2SVPROC _ptrc_glVertexAttrib2sv = Switch_VertexAttrib2sv;
PFN_PTRC_GLVERTEXATTRIB3DPROC _ptrc_glVertexAttrib3d = Switch_VertexAttrib3d;
PFN_PTRC_GLVERTEXATTRIB3DVPROC _ptrc_glVertexAttrib3dv = Switch_VertexAttrib3dv;
PFN_PTRC_GLVERTEXATTRIB3FPROC _ptrc_glVertexAttrib3f = Switch_VertexAttrib3f;
PFN_PTRC_GLVERTEXATTRIB3FVPROC _ptrc_glVertexAttrib3fv = Switch_VertexAttrib3fv;
PFN_PTRC_GLVERTEXATTRIB3SPROC _ptrc_glVertexAttrib3s = Switch_VertexAttrib3s;
PFN_PTRC_GLVERTEXATTRIB3SVPROC _ptrc_glVertexAttrib3sv = Switch_VertexAttrib3sv;
PFN_PTRC_GLVERTEXATTRIB4NBVPROC _ptrc_glVertexAttrib4Nbv = Switch_VertexAttrib4Nbv;
PFN_PTRC_GLVERTEXATTRIB4NIVPROC _ptrc_glVertexAttrib4Niv = Switch_VertexAttrib4Niv;
PFN_PTRC_GLVERTEXATTRIB4NSVPROC _ptrc_glVertexAttrib4Nsv = Switch_VertexAttrib4Nsv;
PFN_PTRC_GLVERTEXATTRIB4NUBPROC _ptrc_glVertexAttrib4Nub = Switch_VertexAttrib4Nub;
PFN_PTRC_GLVERTEXATTRIB4NUBVPROC _ptrc_glVertexAttrib4Nubv = Switch_VertexAttrib4Nubv;
PFN_PTRC_GLVERTEXATTRIB4NUIVPROC _ptrc_glVertexAttrib4Nuiv = Switch_VertexAttrib4Nuiv;
PFN_PTRC_GLVERTEXATTRIB4NUSVPROC _ptrc_glVertexAttrib4Nusv = Switch_VertexAttrib4Nusv;
PFN_PTRC_GLVERTEXATTRIB4BVPROC _ptrc_glVertexAttrib4bv = Switch_VertexAttrib4bv;
PFN_PTRC_GLVERTEXATTRIB4DPROC _ptrc_glVertexAttrib4d = Switch_VertexAttrib4d;
PFN_PTRC_GLVERTEXATTRIB4DVPROC _ptrc_glVertexAttrib4dv = Switch_VertexAttrib4dv;
PFN_PTRC_GLVERTEXATTRIB4FPROC _ptrc_glVertexAttrib4f = Switch_VertexAttrib4f;
PFN_PTRC_GLVERTEXATTRIB4FVPROC _ptrc_glVertexAttrib4fv = Switch_VertexAttrib4fv;
PFN_PTRC_GLVERTEXATTRIB4IVPROC _ptrc_glVertexAttrib4iv = Switch_VertexAttrib4iv;
PFN_PTRC_GLVERTEXATTRIB4SPROC _ptrc_glVertexAttrib4s = Switch_VertexAttrib4s;
PFN_PTRC_GLVERTEXATTRIB4SVPROC _ptrc_glVertexAttrib4sv = Switch_VertexAttrib4sv;
PFN_PTRC_GLVERTEXATTRIB4UBVPROC _ptrc_glVertexAttrib4ubv = Switch_VertexAttrib4ubv;
PFN_PTRC_GLVERTEXATTRIB4UIVPROC _ptrc_glVertexAttrib4uiv = Switch_VertexAttrib4uiv;
PFN_PTRC_GLVERTEXATTRIB4USVPROC _ptrc_glVertexAttrib4usv = Switch_VertexAttrib4usv;
PFN_PTRC_GLVERTEXATTRIBPOINTERPROC _ptrc_glVertexAttribPointer = Switch_VertexAttribPointer;

// Extension: 2.1
PFN_PTRC_GLUNIFORMMATRIX2X3FVPROC _ptrc_glUniformMatrix2x3fv = Switch_UniformMatrix2x3fv;
PFN_PTRC_GLUNIFORMMATRIX2X4FVPROC _ptrc_glUniformMatrix2x4fv = Switch_UniformMatrix2x4fv;
PFN_PTRC_GLUNIFORMMATRIX3X2FVPROC _ptrc_glUniformMatrix3x2fv = Switch_UniformMatrix3x2fv;
PFN_PTRC_GLUNIFORMMATRIX3X4FVPROC _ptrc_glUniformMatrix3x4fv = Switch_UniformMatrix3x4fv;
PFN_PTRC_GLUNIFORMMATRIX4X2FVPROC _ptrc_glUniformMatrix4x2fv = Switch_UniformMatrix4x2fv;
PFN_PTRC_GLUNIFORMMATRIX4X3FVPROC _ptrc_glUniformMatrix4x3fv = Switch_UniformMatrix4x3fv;

// Extension: 3.0
PFN_PTRC_GLBEGINCONDITIONALRENDERPROC _ptrc_glBeginConditionalRender = Switch_BeginConditionalRender;
PFN_PTRC_GLBEGINTRANSFORMFEEDBACKPROC _ptrc_glBeginTransformFeedback = Switch_BeginTransformFeedback;
PFN_PTRC_GLBINDBUFFERBASEPROC _ptrc_glBindBufferBase = Switch_BindBufferBase;
PFN_PTRC_GLBINDBUFFERRANGEPROC _ptrc_glBindBufferRange = Switch_BindBufferRange;
PFN_PTRC_GLBINDFRAGDATALOCATIONPROC _ptrc_glBindFragDataLocation = Switch_BindFragDataLocation;
PFN_PTRC_GLBINDFRAMEBUFFERPROC _ptrc_glBindFramebuffer = Switch_BindFramebuffer;
PFN_PTRC_GLBINDRENDERBUFFERPROC _ptrc_glBindRenderbuffer = Switch_BindRenderbuffer;
PFN_PTRC_GLBINDVERTEXARRAYPROC _ptrc_glBindVertexArray = Switch_BindVertexArray;
PFN_PTRC_GLBLITFRAMEBUFFERPROC _ptrc_glBlitFramebuffer = Switch_BlitFramebuffer;
PFN_PTRC_GLCHECKFRAMEBUFFERSTATUSPROC _ptrc_glCheckFramebufferStatus = Switch_CheckFramebufferStatus;
PFN_PTRC_GLCLAMPCOLORPROC _ptrc_glClampColor = Switch_ClampColor;
PFN_PTRC_GLCLEARBUFFERFIPROC _ptrc_glClearBufferfi = Switch_ClearBufferfi;
PFN_PTRC_GLCLEARBUFFERFVPROC _ptrc_glClearBufferfv = Switch_ClearBufferfv;
PFN_PTRC_GLCLEARBUFFERIVPROC _ptrc_glClearBufferiv = Switch_ClearBufferiv;
PFN_PTRC_GLCLEARBUFFERUIVPROC _ptrc_glClearBufferuiv = Switch_ClearBufferuiv;
PFN_PTRC_GLCOLORMASKIPROC _ptrc_glColorMaski = Switch_ColorMaski;
PFN_PTRC_GLDELETEFRAMEBUFFERSPROC _ptrc_glDeleteFramebuffers = Switch_DeleteFramebuffers;
PFN_PTRC_GLDELETERENDERBUFFERSPROC _ptrc_glDeleteRenderbuffers = Switch_DeleteRenderbuffers;
PFN_PTRC_GLDELETEVERTEXARRAYSPROC _ptrc_glDeleteVertexArrays = Switch_DeleteVertexArrays;
PFN_PTRC_GLDISABLEIPROC _ptrc_glDisablei = Switch_Disablei;
PFN_PTRC_GLENABLEIPROC _ptrc_glEnablei = Switch_Enablei;
PFN_PTRC_GLENDCONDITIONALRENDERPROC _ptrc_glEndConditionalRender = Switch_EndConditionalRender;
PFN_PTRC_GLENDTRANSFORMFEEDBACKPROC _ptrc_glEndTransformFeedback = Switch_EndTransformFeedback;
PFN_PTRC_GLFLUSHMAPPEDBUFFERRANGEPROC _ptrc_glFlushMappedBufferRange = Switch_FlushMappedBufferRange;
PFN_PTRC_GLFRAMEBUFFERRENDERBUFFERPROC _ptrc_glFramebufferRenderbuffer = Switch_FramebufferRenderbuffer;
PFN_PTRC_GLFRAMEBUFFERTEXTURE1DPROC _ptrc_glFramebufferTexture1D = Switch_FramebufferTexture1D;
PFN_PTRC_GLFRAMEBUFFERTEXTURE2DPROC _ptrc_glFramebufferTexture2D = Switch_FramebufferTexture2D;
PFN_PTRC_GLFRAMEBUFFERTEXTURE3DPROC _ptrc_glFramebufferTexture3D = Switch_FramebufferTexture3D;
PFN_PTRC_GLFRAMEBUFFERTEXTURELAYERPROC _ptrc_glFramebufferTextureLayer = Switch_FramebufferTextureLayer;
PFN_PTRC_GLGENFRAMEBUFFERSPROC _ptrc_glGenFramebuffers = Switch_GenFramebuffers;
PFN_PTRC_GLGENRENDERBUFFERSPROC _ptrc_glGenRenderbuffers = Switch_GenRenderbuffers;
PFN_PTRC_GLGENVERTEXARRAYSPROC _ptrc_glGenVertexArrays = Switch_GenVertexArrays;
PFN_PTRC_GLGENERATEMIPMAPPROC _ptrc_glGenerateMipmap = Switch_GenerateMipmap;
PFN_PTRC_GLGETBOOLEANI_VPROC _ptrc_glGetBooleani_v = Switch_GetBooleani_v;
PFN_PTRC_GLGETFRAGDATALOCATIONPROC _ptrc_glGetFragDataLocation = Switch_GetFragDataLocation;
PFN_PTRC_GLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC _ptrc_glGetFramebufferAttachmentParameteriv = Switch_GetFramebufferAttachmentParameteriv;
PFN_PTRC_GLGETINTEGERI_VPROC _ptrc_glGetIntegeri_v = Switch_GetIntegeri_v;
PFN_PTRC_GLGETRENDERBUFFERPARAMETERIVPROC _ptrc_glGetRenderbufferParameteriv = Switch_GetRenderbufferParameteriv;
PFN_PTRC_GLGETSTRINGIPROC _ptrc_glGetStringi = Switch_GetStringi;
PFN_PTRC_GLGETTEXPARAMETERIIVPROC _ptrc_glGetTexParameterIiv = Switch_GetTexParameterIiv;
PFN_PTRC_GLGETTEXPARAMETERIUIVPROC _ptrc_glGetTexParameterIuiv = Switch_GetTexParameterIuiv;
PFN_PTRC_GLGETTRANSFORMFEEDBACKVARYINGPROC _ptrc_glGetTransformFeedbackVarying = Switch_GetTransformFeedbackVarying;
PFN_PTRC_GLGETUNIFORMUIVPROC _ptrc_glGetUniformuiv = Switch_GetUniformuiv;
PFN_PTRC_GLGETVERTEXATTRIBIIVPROC _ptrc_glGetVertexAttribIiv = Switch_GetVertexAttribIiv;
PFN_PTRC_GLGETVERTEXATTRIBIUIVPROC _ptrc_glGetVertexAttribIuiv = Switch_GetVertexAttribIuiv;
PFN_PTRC_GLISENABLEDIPROC _ptrc_glIsEnabledi = Switch_IsEnabledi;
PFN_PTRC_GLISFRAMEBUFFERPROC _ptrc_glIsFramebuffer = Switch_IsFramebuffer;
PFN_PTRC_GLISRENDERBUFFERPROC _ptrc_glIsRenderbuffer = Switch_IsRenderbuffer;
PFN_PTRC_GLISVERTEXARRAYPROC _ptrc_glIsVertexArray = Switch_IsVertexArray;
PFN_PTRC_GLMAPBUFFERRANGEPROC _ptrc_glMapBufferRange = Switch_MapBufferRange;
PFN_PTRC_GLRENDERBUFFERSTORAGEPROC _ptrc_glRenderbufferStorage = Switch_RenderbufferStorage;
PFN_PTRC_GLRENDERBUFFERSTORAGEMULTISAMPLEPROC _ptrc_glRenderbufferStorageMultisample = Switch_RenderbufferStorageMultisample;
PFN_PTRC_GLTEXPARAMETERIIVPROC _ptrc_glTexParameterIiv = Switch_TexParameterIiv;
PFN_PTRC_GLTEXPARAMETERIUIVPROC _ptrc_glTexParameterIuiv = Switch_TexParameterIuiv;
PFN_PTRC_GLTRANSFORMFEEDBACKVARYINGSPROC _ptrc_glTransformFeedbackVaryings = Switch_TransformFeedbackVaryings;
PFN_PTRC_GLUNIFORM1UIPROC _ptrc_glUniform1ui = Switch_Uniform1ui;
PFN_PTRC_GLUNIFORM1UIVPROC _ptrc_glUniform1uiv = Switch_Uniform1uiv;
PFN_PTRC_GLUNIFORM2UIPROC _ptrc_glUniform2ui = Switch_Uniform2ui;
PFN_PTRC_GLUNIFORM2UIVPROC _ptrc_glUniform2uiv = Switch_Uniform2uiv;
PFN_PTRC_GLUNIFORM3UIPROC _ptrc_glUniform3ui = Switch_Uniform3ui;
PFN_PTRC_GLUNIFORM3UIVPROC _ptrc_glUniform3uiv = Switch_Uniform3uiv;
PFN_PTRC_GLUNIFORM4UIPROC _ptrc_glUniform4ui = Switch_Uniform4ui;
PFN_PTRC_GLUNIFORM4UIVPROC _ptrc_glUniform4uiv = Switch_Uniform4uiv;
PFN_PTRC_GLVERTEXATTRIBI1IPROC _ptrc_glVertexAttribI1i = Switch_VertexAttribI1i;
PFN_PTRC_GLVERTEXATTRIBI1IVPROC _ptrc_glVertexAttribI1iv = Switch_VertexAttribI1iv;
PFN_PTRC_GLVERTEXATTRIBI1UIPROC _ptrc_glVertexAttribI1ui = Switch_VertexAttribI1ui;
PFN_PTRC_GLVERTEXATTRIBI1UIVPROC _ptrc_glVertexAttribI1uiv = Switch_VertexAttribI1uiv;
PFN_PTRC_GLVERTEXATTRIBI2IPROC _ptrc_glVertexAttribI2i = Switch_VertexAttribI2i;
PFN_PTRC_GLVERTEXATTRIBI2IVPROC _ptrc_glVertexAttribI2iv = Switch_VertexAttribI2iv;
PFN_PTRC_GLVERTEXATTRIBI2UIPROC _ptrc_glVertexAttribI2ui = Switch_VertexAttribI2ui;
PFN_PTRC_GLVERTEXATTRIBI2UIVPROC _ptrc_glVertexAttribI2uiv = Switch_VertexAttribI2uiv;
PFN_PTRC_GLVERTEXATTRIBI3IPROC _ptrc_glVertexAttribI3i = Switch_VertexAttribI3i;
PFN_PTRC_GLVERTEXATTRIBI3IVPROC _ptrc_glVertexAttribI3iv = Switch_VertexAttribI3iv;
PFN_PTRC_GLVERTEXATTRIBI3UIPROC _ptrc_glVertexAttribI3ui = Switch_VertexAttribI3ui;
PFN_PTRC_GLVERTEXATTRIBI3UIVPROC _ptrc_glVertexAttribI3uiv = Switch_VertexAttribI3uiv;
PFN_PTRC_GLVERTEXATTRIBI4BVPROC _ptrc_glVertexAttribI4bv = Switch_VertexAttribI4bv;
PFN_PTRC_GLVERTEXATTRIBI4IPROC _ptrc_glVertexAttribI4i = Switch_VertexAttribI4i;
PFN_PTRC_GLVERTEXATTRIBI4IVPROC _ptrc_glVertexAttribI4iv = Switch_VertexAttribI4iv;
PFN_PTRC_GLVERTEXATTRIBI4SVPROC _ptrc_glVertexAttribI4sv = Switch_VertexAttribI4sv;
PFN_PTRC_GLVERTEXATTRIBI4UBVPROC _ptrc_glVertexAttribI4ubv = Switch_VertexAttribI4ubv;
PFN_PTRC_GLVERTEXATTRIBI4UIPROC _ptrc_glVertexAttribI4ui = Switch_VertexAttribI4ui;
PFN_PTRC_GLVERTEXATTRIBI4UIVPROC _ptrc_glVertexAttribI4uiv = Switch_VertexAttribI4uiv;
PFN_PTRC_GLVERTEXATTRIBI4USVPROC _ptrc_glVertexAttribI4usv = Switch_VertexAttribI4usv;
PFN_PTRC_GLVERTEXATTRIBIPOINTERPROC _ptrc_glVertexAttribIPointer = Switch_VertexAttribIPointer;

// Extension: 3.1
PFN_PTRC_GLCOPYBUFFERSUBDATAPROC _ptrc_glCopyBufferSubData = Switch_CopyBufferSubData;
PFN_PTRC_GLDRAWARRAYSINSTANCEDPROC _ptrc_glDrawArraysInstanced = Switch_DrawArraysInstanced;
PFN_PTRC_GLDRAWELEMENTSINSTANCEDPROC _ptrc_glDrawElementsInstanced = Switch_DrawElementsInstanced;
PFN_PTRC_GLGETACTIVEUNIFORMBLOCKNAMEPROC _ptrc_glGetActiveUniformBlockName = Switch_GetActiveUniformBlockName;
PFN_PTRC_GLGETACTIVEUNIFORMBLOCKIVPROC _ptrc_glGetActiveUniformBlockiv = Switch_GetActiveUniformBlockiv;
PFN_PTRC_GLGETACTIVEUNIFORMNAMEPROC _ptrc_glGetActiveUniformName = Switch_GetActiveUniformName;
PFN_PTRC_GLGETACTIVEUNIFORMSIVPROC _ptrc_glGetActiveUniformsiv = Switch_GetActiveUniformsiv;
PFN_PTRC_GLGETUNIFORMBLOCKINDEXPROC _ptrc_glGetUniformBlockIndex = Switch_GetUniformBlockIndex;
PFN_PTRC_GLGETUNIFORMINDICESPROC _ptrc_glGetUniformIndices = Switch_GetUniformIndices;
PFN_PTRC_GLPRIMITIVERESTARTINDEXPROC _ptrc_glPrimitiveRestartIndex = Switch_PrimitiveRestartIndex;
PFN_PTRC_GLTEXBUFFERPROC _ptrc_glTexBuffer = Switch_TexBuffer;
PFN_PTRC_GLUNIFORMBLOCKBINDINGPROC _ptrc_glUniformBlockBinding = Switch_UniformBlockBinding;

// Extension: 3.2
PFN_PTRC_GLCLIENTWAITSYNCPROC _ptrc_glClientWaitSync = Switch_ClientWaitSync;
PFN_PTRC_GLDELETESYNCPROC _ptrc_glDeleteSync = Switch_DeleteSync;
PFN_PTRC_GLDRAWELEMENTSBASEVERTEXPROC _ptrc_glDrawElementsBaseVertex = Switch_DrawElementsBaseVertex;
PFN_PTRC_GLDRAWELEMENTSINSTANCEDBASEVERTEXPROC _ptrc_glDrawElementsInstancedBaseVertex = Switch_DrawElementsInstancedBaseVertex;
PFN_PTRC_GLDRAWRANGEELEMENTSBASEVERTEXPROC _ptrc_glDrawRangeElementsBaseVertex = Switch_DrawRangeElementsBaseVertex;
PFN_PTRC_GLFENCESYNCPROC _ptrc_glFenceSync = Switch_FenceSync;
PFN_PTRC_GLFRAMEBUFFERTEXTUREPROC _ptrc_glFramebufferTexture = Switch_FramebufferTexture;
PFN_PTRC_GLGETBUFFERPARAMETERI64VPROC _ptrc_glGetBufferParameteri64v = Switch_GetBufferParameteri64v;
PFN_PTRC_GLGETINTEGER64I_VPROC _ptrc_glGetInteger64i_v = Switch_GetInteger64i_v;
PFN_PTRC_GLGETINTEGER64VPROC _ptrc_glGetInteger64v = Switch_GetInteger64v;
PFN_PTRC_GLGETMULTISAMPLEFVPROC _ptrc_glGetMultisamplefv = Switch_GetMultisamplefv;
PFN_PTRC_GLGETSYNCIVPROC _ptrc_glGetSynciv = Switch_GetSynciv;
PFN_PTRC_GLISSYNCPROC _ptrc_glIsSync = Switch_IsSync;
PFN_PTRC_GLMULTIDRAWELEMENTSBASEVERTEXPROC _ptrc_glMultiDrawElementsBaseVertex = Switch_MultiDrawElementsBaseVertex;
PFN_PTRC_GLPROVOKINGVERTEXPROC _ptrc_glProvokingVertex = Switch_ProvokingVertex;
PFN_PTRC_GLSAMPLEMASKIPROC _ptrc_glSampleMaski = Switch_SampleMaski;
PFN_PTRC_GLTEXIMAGE2DMULTISAMPLEPROC _ptrc_glTexImage2DMultisample = Switch_TexImage2DMultisample;
PFN_PTRC_GLTEXIMAGE3DMULTISAMPLEPROC _ptrc_glTexImage3DMultisample = Switch_TexImage3DMultisample;
PFN_PTRC_GLWAITSYNCPROC _ptrc_glWaitSync = Switch_WaitSync;

// Extension: 3.3
PFN_PTRC_GLBINDFRAGDATALOCATIONINDEXEDPROC _ptrc_glBindFragDataLocationIndexed = Switch_BindFragDataLocationIndexed;
PFN_PTRC_GLBINDSAMPLERPROC _ptrc_glBindSampler = Switch_BindSampler;
PFN_PTRC_GLDELETESAMPLERSPROC _ptrc_glDeleteSamplers = Switch_DeleteSamplers;
PFN_PTRC_GLGENSAMPLERSPROC _ptrc_glGenSamplers = Switch_GenSamplers;
PFN_PTRC_GLGETFRAGDATAINDEXPROC _ptrc_glGetFragDataIndex = Switch_GetFragDataIndex;
PFN_PTRC_GLGETQUERYOBJECTI64VPROC _ptrc_glGetQueryObjecti64v = Switch_GetQueryObjecti64v;
PFN_PTRC_GLGETQUERYOBJECTUI64VPROC _ptrc_glGetQueryObjectui64v = Switch_GetQueryObjectui64v;
PFN_PTRC_GLGETSAMPLERPARAMETERIIVPROC _ptrc_glGetSamplerParameterIiv = Switch_GetSamplerParameterIiv;
PFN_PTRC_GLGETSAMPLERPARAMETERIUIVPROC _ptrc_glGetSamplerParameterIuiv = Switch_GetSamplerParameterIuiv;
PFN_PTRC_GLGETSAMPLERPARAMETERFVPROC _ptrc_glGetSamplerParameterfv = Switch_GetSamplerParameterfv;
PFN_PTRC_GLGETSAMPLERPARAMETERIVPROC _ptrc_glGetSamplerParameteriv = Switch_GetSamplerParameteriv;
PFN_PTRC_GLISSAMPLERPROC _ptrc_glIsSampler = Switch_IsSampler;
PFN_PTRC_GLQUERYCOUNTERPROC _ptrc_glQueryCounter = Switch_QueryCounter;
PFN_PTRC_GLSAMPLERPARAMETERIIVPROC _ptrc_glSamplerParameterIiv = Switch_SamplerParameterIiv;
PFN_PTRC_GLSAMPLERPARAMETERIUIVPROC _ptrc_glSamplerParameterIuiv = Switch_SamplerParameterIuiv;
PFN_PTRC_GLSAMPLERPARAMETERFPROC _ptrc_glSamplerParameterf = Switch_SamplerParameterf;
PFN_PTRC_GLSAMPLERPARAMETERFVPROC _ptrc_glSamplerParameterfv = Switch_SamplerParameterfv;
PFN_PTRC_GLSAMPLERPARAMETERIPROC _ptrc_glSamplerParameteri = Switch_SamplerParameteri;
PFN_PTRC_GLSAMPLERPARAMETERIVPROC _ptrc_glSamplerParameteriv = Switch_SamplerParameteriv;
PFN_PTRC_GLVERTEXATTRIBDIVISORPROC _ptrc_glVertexAttribDivisor = Switch_VertexAttribDivisor;
PFN_PTRC_GLVERTEXATTRIBP1UIPROC _ptrc_glVertexAttribP1ui = Switch_VertexAttribP1ui;
PFN_PTRC_GLVERTEXATTRIBP1UIVPROC _ptrc_glVertexAttribP1uiv = Switch_VertexAttribP1uiv;
PFN_PTRC_GLVERTEXATTRIBP2UIPROC _ptrc_glVertexAttribP2ui = Switch_VertexAttribP2ui;
PFN_PTRC_GLVERTEXATTRIBP2UIVPROC _ptrc_glVertexAttribP2uiv = Switch_VertexAttribP2uiv;
PFN_PTRC_GLVERTEXATTRIBP3UIPROC _ptrc_glVertexAttribP3ui = Switch_VertexAttribP3ui;
PFN_PTRC_GLVERTEXATTRIBP3UIVPROC _ptrc_glVertexAttribP3uiv = Switch_VertexAttribP3uiv;
PFN_PTRC_GLVERTEXATTRIBP4UIPROC _ptrc_glVertexAttribP4ui = Switch_VertexAttribP4ui;
PFN_PTRC_GLVERTEXATTRIBP4UIVPROC _ptrc_glVertexAttribP4uiv = Switch_VertexAttribP4uiv;


// Extension: ARB_ES2_compatibility
static void CODEGEN_FUNCPTR Switch_ClearDepthf(GLfloat d)
{
	_ptrc_glClearDepthf = (PFN_PTRC_GLCLEARDEPTHFPROC)IntGetProcAddress("glClearDepthf");
	_ptrc_glClearDepthf(d);
}

static void CODEGEN_FUNCPTR Switch_DepthRangef(GLfloat n, GLfloat f)
{
	_ptrc_glDepthRangef = (PFN_PTRC_GLDEPTHRANGEFPROC)IntGetProcAddress("glDepthRangef");
	_ptrc_glDepthRangef(n, f);
}

static void CODEGEN_FUNCPTR Switch_GetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision)
{
	_ptrc_glGetShaderPrecisionFormat = (PFN_PTRC_GLGETSHADERPRECISIONFORMATPROC)IntGetProcAddress("glGetShaderPrecisionFormat");
	_ptrc_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

static void CODEGEN_FUNCPTR Switch_ReleaseShaderCompiler()
{
	_ptrc_glReleaseShaderCompiler = (PFN_PTRC_GLRELEASESHADERCOMPILERPROC)IntGetProcAddress("glReleaseShaderCompiler");
	_ptrc_glReleaseShaderCompiler();
}

static void CODEGEN_FUNCPTR Switch_ShaderBinary(GLsizei count, const GLuint * shaders, GLenum binaryformat, const GLvoid * binary, GLsizei length)
{
	_ptrc_glShaderBinary = (PFN_PTRC_GLSHADERBINARYPROC)IntGetProcAddress("glShaderBinary");
	_ptrc_glShaderBinary(count, shaders, binaryformat, binary, length);
}


// Extension: ARB_get_program_binary
static void CODEGEN_FUNCPTR Switch_GetProgramBinary(GLuint program, GLsizei bufSize, GLsizei * length, GLenum * binaryFormat, GLvoid * binary)
{
	_ptrc_glGetProgramBinary = (PFN_PTRC_GLGETPROGRAMBINARYPROC)IntGetProcAddress("glGetProgramBinary");
	_ptrc_glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

static void CODEGEN_FUNCPTR Switch_ProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid * binary, GLsizei length)
{
	_ptrc_glProgramBinary = (PFN_PTRC_GLPROGRAMBINARYPROC)IntGetProcAddress("glProgramBinary");
	_ptrc_glProgramBinary(program, binaryFormat, binary, length);
}

static void CODEGEN_FUNCPTR Switch_ProgramParameteri(GLuint program, GLenum pname, GLint value)
{
	_ptrc_glProgramParameteri = (PFN_PTRC_GLPROGRAMPARAMETERIPROC)IntGetProcAddress("glProgramParameteri");
	_ptrc_glProgramParameteri(program, pname, value);
}


// Extension: ARB_internalformat_query
static void CODEGEN_FUNCPTR Switch_GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint * params)
{
	_ptrc_glGetInternalformativ = (PFN_PTRC_GLGETINTERNALFORMATIVPROC)IntGetProcAddress("glGetInternalformativ");
	_ptrc_glGetInternalformativ(target, internalformat, pname, bufSize, params);
}


// Extension: ARB_internalformat_query2
static void CODEGEN_FUNCPTR Switch_GetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 * params)
{
	_ptrc_glGetInternalformati64v = (PFN_PTRC_GLGETINTERNALFORMATI64VPROC)IntGetProcAddress("glGetInternalformati64v");
	_ptrc_glGetInternalformati64v(target, internalformat, pname, bufSize, params);
}


// Extension: ARB_program_interface_query
static void CODEGEN_FUNCPTR Switch_GetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint * params)
{
	_ptrc_glGetProgramInterfaceiv = (PFN_PTRC_GLGETPROGRAMINTERFACEIVPROC)IntGetProcAddress("glGetProgramInterfaceiv");
	_ptrc_glGetProgramInterfaceiv(program, programInterface, pname, params);
}

static GLuint CODEGEN_FUNCPTR Switch_GetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar * name)
{
	_ptrc_glGetProgramResourceIndex = (PFN_PTRC_GLGETPROGRAMRESOURCEINDEXPROC)IntGetProcAddress("glGetProgramResourceIndex");
	return _ptrc_glGetProgramResourceIndex(program, programInterface, name);
}

static GLint CODEGEN_FUNCPTR Switch_GetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar * name)
{
	_ptrc_glGetProgramResourceLocation = (PFN_PTRC_GLGETPROGRAMRESOURCELOCATIONPROC)IntGetProcAddress("glGetProgramResourceLocation");
	return _ptrc_glGetProgramResourceLocation(program, programInterface, name);
}

static GLint CODEGEN_FUNCPTR Switch_GetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar * name)
{
	_ptrc_glGetProgramResourceLocationIndex = (PFN_PTRC_GLGETPROGRAMRESOURCELOCATIONINDEXPROC)IntGetProcAddress("glGetProgramResourceLocationIndex");
	return _ptrc_glGetProgramResourceLocationIndex(program, programInterface, name);
}

static void CODEGEN_FUNCPTR Switch_GetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei * length, GLchar * name)
{
	_ptrc_glGetProgramResourceName = (PFN_PTRC_GLGETPROGRAMRESOURCENAMEPROC)IntGetProcAddress("glGetProgramResourceName");
	_ptrc_glGetProgramResourceName(program, programInterface, index, bufSize, length, name);
}

static void CODEGEN_FUNCPTR Switch_GetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum * props, GLsizei bufSize, GLsizei * length, GLint * params)
{
	_ptrc_glGetProgramResourceiv = (PFN_PTRC_GLGETPROGRAMRESOURCEIVPROC)IntGetProcAddress("glGetProgramResourceiv");
	_ptrc_glGetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length, params);
}


// Extension: ARB_separate_shader_objects
static void CODEGEN_FUNCPTR Switch_ActiveShaderProgram(GLuint pipeline, GLuint program)
{
	_ptrc_glActiveShaderProgram = (PFN_PTRC_GLACTIVESHADERPROGRAMPROC)IntGetProcAddress("glActiveShaderProgram");
	_ptrc_glActiveShaderProgram(pipeline, program);
}

static void CODEGEN_FUNCPTR Switch_BindProgramPipeline(GLuint pipeline)
{
	_ptrc_glBindProgramPipeline = (PFN_PTRC_GLBINDPROGRAMPIPELINEPROC)IntGetProcAddress("glBindProgramPipeline");
	_ptrc_glBindProgramPipeline(pipeline);
}

static GLuint CODEGEN_FUNCPTR Switch_CreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const* strings)
{
	_ptrc_glCreateShaderProgramv = (PFN_PTRC_GLCREATESHADERPROGRAMVPROC)IntGetProcAddress("glCreateShaderProgramv");
	return _ptrc_glCreateShaderProgramv(type, count, strings);
}

static void CODEGEN_FUNCPTR Switch_DeleteProgramPipelines(GLsizei n, const GLuint * pipelines)
{
	_ptrc_glDeleteProgramPipelines = (PFN_PTRC_GLDELETEPROGRAMPIPELINESPROC)IntGetProcAddress("glDeleteProgramPipelines");
	_ptrc_glDeleteProgramPipelines(n, pipelines);
}

static void CODEGEN_FUNCPTR Switch_GenProgramPipelines(GLsizei n, GLuint * pipelines)
{
	_ptrc_glGenProgramPipelines = (PFN_PTRC_GLGENPROGRAMPIPELINESPROC)IntGetProcAddress("glGenProgramPipelines");
	_ptrc_glGenProgramPipelines(n, pipelines);
}

static void CODEGEN_FUNCPTR Switch_GetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
	_ptrc_glGetProgramPipelineInfoLog = (PFN_PTRC_GLGETPROGRAMPIPELINEINFOLOGPROC)IntGetProcAddress("glGetProgramPipelineInfoLog");
	_ptrc_glGetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
}

static void CODEGEN_FUNCPTR Switch_GetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint * params)
{
	_ptrc_glGetProgramPipelineiv = (PFN_PTRC_GLGETPROGRAMPIPELINEIVPROC)IntGetProcAddress("glGetProgramPipelineiv");
	_ptrc_glGetProgramPipelineiv(pipeline, pname, params);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsProgramPipeline(GLuint pipeline)
{
	_ptrc_glIsProgramPipeline = (PFN_PTRC_GLISPROGRAMPIPELINEPROC)IntGetProcAddress("glIsProgramPipeline");
	return _ptrc_glIsProgramPipeline(pipeline);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform1d(GLuint program, GLint location, GLdouble v0)
{
	_ptrc_glProgramUniform1d = (PFN_PTRC_GLPROGRAMUNIFORM1DPROC)IntGetProcAddress("glProgramUniform1d");
	_ptrc_glProgramUniform1d(program, location, v0);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble * value)
{
	_ptrc_glProgramUniform1dv = (PFN_PTRC_GLPROGRAMUNIFORM1DVPROC)IntGetProcAddress("glProgramUniform1dv");
	_ptrc_glProgramUniform1dv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
	_ptrc_glProgramUniform1f = (PFN_PTRC_GLPROGRAMUNIFORM1FPROC)IntGetProcAddress("glProgramUniform1f");
	_ptrc_glProgramUniform1f(program, location, v0);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
	_ptrc_glProgramUniform1fv = (PFN_PTRC_GLPROGRAMUNIFORM1FVPROC)IntGetProcAddress("glProgramUniform1fv");
	_ptrc_glProgramUniform1fv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform1i(GLuint program, GLint location, GLint v0)
{
	_ptrc_glProgramUniform1i = (PFN_PTRC_GLPROGRAMUNIFORM1IPROC)IntGetProcAddress("glProgramUniform1i");
	_ptrc_glProgramUniform1i(program, location, v0);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint * value)
{
	_ptrc_glProgramUniform1iv = (PFN_PTRC_GLPROGRAMUNIFORM1IVPROC)IntGetProcAddress("glProgramUniform1iv");
	_ptrc_glProgramUniform1iv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
	_ptrc_glProgramUniform1ui = (PFN_PTRC_GLPROGRAMUNIFORM1UIPROC)IntGetProcAddress("glProgramUniform1ui");
	_ptrc_glProgramUniform1ui(program, location, v0);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
	_ptrc_glProgramUniform1uiv = (PFN_PTRC_GLPROGRAMUNIFORM1UIVPROC)IntGetProcAddress("glProgramUniform1uiv");
	_ptrc_glProgramUniform1uiv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
	_ptrc_glProgramUniform2d = (PFN_PTRC_GLPROGRAMUNIFORM2DPROC)IntGetProcAddress("glProgramUniform2d");
	_ptrc_glProgramUniform2d(program, location, v0, v1);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble * value)
{
	_ptrc_glProgramUniform2dv = (PFN_PTRC_GLPROGRAMUNIFORM2DVPROC)IntGetProcAddress("glProgramUniform2dv");
	_ptrc_glProgramUniform2dv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
	_ptrc_glProgramUniform2f = (PFN_PTRC_GLPROGRAMUNIFORM2FPROC)IntGetProcAddress("glProgramUniform2f");
	_ptrc_glProgramUniform2f(program, location, v0, v1);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
	_ptrc_glProgramUniform2fv = (PFN_PTRC_GLPROGRAMUNIFORM2FVPROC)IntGetProcAddress("glProgramUniform2fv");
	_ptrc_glProgramUniform2fv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
	_ptrc_glProgramUniform2i = (PFN_PTRC_GLPROGRAMUNIFORM2IPROC)IntGetProcAddress("glProgramUniform2i");
	_ptrc_glProgramUniform2i(program, location, v0, v1);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint * value)
{
	_ptrc_glProgramUniform2iv = (PFN_PTRC_GLPROGRAMUNIFORM2IVPROC)IntGetProcAddress("glProgramUniform2iv");
	_ptrc_glProgramUniform2iv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
	_ptrc_glProgramUniform2ui = (PFN_PTRC_GLPROGRAMUNIFORM2UIPROC)IntGetProcAddress("glProgramUniform2ui");
	_ptrc_glProgramUniform2ui(program, location, v0, v1);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
	_ptrc_glProgramUniform2uiv = (PFN_PTRC_GLPROGRAMUNIFORM2UIVPROC)IntGetProcAddress("glProgramUniform2uiv");
	_ptrc_glProgramUniform2uiv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
	_ptrc_glProgramUniform3d = (PFN_PTRC_GLPROGRAMUNIFORM3DPROC)IntGetProcAddress("glProgramUniform3d");
	_ptrc_glProgramUniform3d(program, location, v0, v1, v2);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble * value)
{
	_ptrc_glProgramUniform3dv = (PFN_PTRC_GLPROGRAMUNIFORM3DVPROC)IntGetProcAddress("glProgramUniform3dv");
	_ptrc_glProgramUniform3dv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	_ptrc_glProgramUniform3f = (PFN_PTRC_GLPROGRAMUNIFORM3FPROC)IntGetProcAddress("glProgramUniform3f");
	_ptrc_glProgramUniform3f(program, location, v0, v1, v2);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
	_ptrc_glProgramUniform3fv = (PFN_PTRC_GLPROGRAMUNIFORM3FVPROC)IntGetProcAddress("glProgramUniform3fv");
	_ptrc_glProgramUniform3fv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
	_ptrc_glProgramUniform3i = (PFN_PTRC_GLPROGRAMUNIFORM3IPROC)IntGetProcAddress("glProgramUniform3i");
	_ptrc_glProgramUniform3i(program, location, v0, v1, v2);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint * value)
{
	_ptrc_glProgramUniform3iv = (PFN_PTRC_GLPROGRAMUNIFORM3IVPROC)IntGetProcAddress("glProgramUniform3iv");
	_ptrc_glProgramUniform3iv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	_ptrc_glProgramUniform3ui = (PFN_PTRC_GLPROGRAMUNIFORM3UIPROC)IntGetProcAddress("glProgramUniform3ui");
	_ptrc_glProgramUniform3ui(program, location, v0, v1, v2);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
	_ptrc_glProgramUniform3uiv = (PFN_PTRC_GLPROGRAMUNIFORM3UIVPROC)IntGetProcAddress("glProgramUniform3uiv");
	_ptrc_glProgramUniform3uiv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
	_ptrc_glProgramUniform4d = (PFN_PTRC_GLPROGRAMUNIFORM4DPROC)IntGetProcAddress("glProgramUniform4d");
	_ptrc_glProgramUniform4d(program, location, v0, v1, v2, v3);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble * value)
{
	_ptrc_glProgramUniform4dv = (PFN_PTRC_GLPROGRAMUNIFORM4DVPROC)IntGetProcAddress("glProgramUniform4dv");
	_ptrc_glProgramUniform4dv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	_ptrc_glProgramUniform4f = (PFN_PTRC_GLPROGRAMUNIFORM4FPROC)IntGetProcAddress("glProgramUniform4f");
	_ptrc_glProgramUniform4f(program, location, v0, v1, v2, v3);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
	_ptrc_glProgramUniform4fv = (PFN_PTRC_GLPROGRAMUNIFORM4FVPROC)IntGetProcAddress("glProgramUniform4fv");
	_ptrc_glProgramUniform4fv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	_ptrc_glProgramUniform4i = (PFN_PTRC_GLPROGRAMUNIFORM4IPROC)IntGetProcAddress("glProgramUniform4i");
	_ptrc_glProgramUniform4i(program, location, v0, v1, v2, v3);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint * value)
{
	_ptrc_glProgramUniform4iv = (PFN_PTRC_GLPROGRAMUNIFORM4IVPROC)IntGetProcAddress("glProgramUniform4iv");
	_ptrc_glProgramUniform4iv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	_ptrc_glProgramUniform4ui = (PFN_PTRC_GLPROGRAMUNIFORM4UIPROC)IntGetProcAddress("glProgramUniform4ui");
	_ptrc_glProgramUniform4ui(program, location, v0, v1, v2, v3);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
	_ptrc_glProgramUniform4uiv = (PFN_PTRC_GLPROGRAMUNIFORM4UIVPROC)IntGetProcAddress("glProgramUniform4uiv");
	_ptrc_glProgramUniform4uiv(program, location, count, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
	_ptrc_glProgramUniformMatrix2dv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX2DVPROC)IntGetProcAddress("glProgramUniformMatrix2dv");
	_ptrc_glProgramUniformMatrix2dv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glProgramUniformMatrix2fv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX2FVPROC)IntGetProcAddress("glProgramUniformMatrix2fv");
	_ptrc_glProgramUniformMatrix2fv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
	_ptrc_glProgramUniformMatrix2x3dv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X3DVPROC)IntGetProcAddress("glProgramUniformMatrix2x3dv");
	_ptrc_glProgramUniformMatrix2x3dv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glProgramUniformMatrix2x3fv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X3FVPROC)IntGetProcAddress("glProgramUniformMatrix2x3fv");
	_ptrc_glProgramUniformMatrix2x3fv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
	_ptrc_glProgramUniformMatrix2x4dv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X4DVPROC)IntGetProcAddress("glProgramUniformMatrix2x4dv");
	_ptrc_glProgramUniformMatrix2x4dv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glProgramUniformMatrix2x4fv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX2X4FVPROC)IntGetProcAddress("glProgramUniformMatrix2x4fv");
	_ptrc_glProgramUniformMatrix2x4fv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
	_ptrc_glProgramUniformMatrix3dv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX3DVPROC)IntGetProcAddress("glProgramUniformMatrix3dv");
	_ptrc_glProgramUniformMatrix3dv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glProgramUniformMatrix3fv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX3FVPROC)IntGetProcAddress("glProgramUniformMatrix3fv");
	_ptrc_glProgramUniformMatrix3fv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
	_ptrc_glProgramUniformMatrix3x2dv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X2DVPROC)IntGetProcAddress("glProgramUniformMatrix3x2dv");
	_ptrc_glProgramUniformMatrix3x2dv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glProgramUniformMatrix3x2fv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X2FVPROC)IntGetProcAddress("glProgramUniformMatrix3x2fv");
	_ptrc_glProgramUniformMatrix3x2fv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
	_ptrc_glProgramUniformMatrix3x4dv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X4DVPROC)IntGetProcAddress("glProgramUniformMatrix3x4dv");
	_ptrc_glProgramUniformMatrix3x4dv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glProgramUniformMatrix3x4fv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX3X4FVPROC)IntGetProcAddress("glProgramUniformMatrix3x4fv");
	_ptrc_glProgramUniformMatrix3x4fv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
	_ptrc_glProgramUniformMatrix4dv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX4DVPROC)IntGetProcAddress("glProgramUniformMatrix4dv");
	_ptrc_glProgramUniformMatrix4dv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glProgramUniformMatrix4fv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX4FVPROC)IntGetProcAddress("glProgramUniformMatrix4fv");
	_ptrc_glProgramUniformMatrix4fv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
	_ptrc_glProgramUniformMatrix4x2dv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X2DVPROC)IntGetProcAddress("glProgramUniformMatrix4x2dv");
	_ptrc_glProgramUniformMatrix4x2dv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glProgramUniformMatrix4x2fv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X2FVPROC)IntGetProcAddress("glProgramUniformMatrix4x2fv");
	_ptrc_glProgramUniformMatrix4x2fv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
	_ptrc_glProgramUniformMatrix4x3dv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X3DVPROC)IntGetProcAddress("glProgramUniformMatrix4x3dv");
	_ptrc_glProgramUniformMatrix4x3dv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_ProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glProgramUniformMatrix4x3fv = (PFN_PTRC_GLPROGRAMUNIFORMMATRIX4X3FVPROC)IntGetProcAddress("glProgramUniformMatrix4x3fv");
	_ptrc_glProgramUniformMatrix4x3fv(program, location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_UseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
	_ptrc_glUseProgramStages = (PFN_PTRC_GLUSEPROGRAMSTAGESPROC)IntGetProcAddress("glUseProgramStages");
	_ptrc_glUseProgramStages(pipeline, stages, program);
}

static void CODEGEN_FUNCPTR Switch_ValidateProgramPipeline(GLuint pipeline)
{
	_ptrc_glValidateProgramPipeline = (PFN_PTRC_GLVALIDATEPROGRAMPIPELINEPROC)IntGetProcAddress("glValidateProgramPipeline");
	_ptrc_glValidateProgramPipeline(pipeline);
}


// Extension: ARB_texture_buffer_range
static void CODEGEN_FUNCPTR Switch_TexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	_ptrc_glTexBufferRange = (PFN_PTRC_GLTEXBUFFERRANGEPROC)IntGetProcAddress("glTexBufferRange");
	_ptrc_glTexBufferRange(target, internalformat, buffer, offset, size);
}


// Extension: ARB_texture_storage
static void CODEGEN_FUNCPTR Switch_TexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
	_ptrc_glTexStorage1D = (PFN_PTRC_GLTEXSTORAGE1DPROC)IntGetProcAddress("glTexStorage1D");
	_ptrc_glTexStorage1D(target, levels, internalformat, width);
}

static void CODEGEN_FUNCPTR Switch_TexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	_ptrc_glTexStorage2D = (PFN_PTRC_GLTEXSTORAGE2DPROC)IntGetProcAddress("glTexStorage2D");
	_ptrc_glTexStorage2D(target, levels, internalformat, width, height);
}

static void CODEGEN_FUNCPTR Switch_TexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	_ptrc_glTexStorage3D = (PFN_PTRC_GLTEXSTORAGE3DPROC)IntGetProcAddress("glTexStorage3D");
	_ptrc_glTexStorage3D(target, levels, internalformat, width, height, depth);
}


// Extension: ARB_texture_view
static void CODEGEN_FUNCPTR Switch_TextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)
{
	_ptrc_glTextureView = (PFN_PTRC_GLTEXTUREVIEWPROC)IntGetProcAddress("glTextureView");
	_ptrc_glTextureView(texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers);
}


// Extension: ARB_vertex_attrib_binding
static void CODEGEN_FUNCPTR Switch_BindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
	_ptrc_glBindVertexBuffer = (PFN_PTRC_GLBINDVERTEXBUFFERPROC)IntGetProcAddress("glBindVertexBuffer");
	_ptrc_glBindVertexBuffer(bindingindex, buffer, offset, stride);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
	_ptrc_glVertexAttribBinding = (PFN_PTRC_GLVERTEXATTRIBBINDINGPROC)IntGetProcAddress("glVertexAttribBinding");
	_ptrc_glVertexAttribBinding(attribindex, bindingindex);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
	_ptrc_glVertexAttribFormat = (PFN_PTRC_GLVERTEXATTRIBFORMATPROC)IntGetProcAddress("glVertexAttribFormat");
	_ptrc_glVertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
	_ptrc_glVertexAttribIFormat = (PFN_PTRC_GLVERTEXATTRIBIFORMATPROC)IntGetProcAddress("glVertexAttribIFormat");
	_ptrc_glVertexAttribIFormat(attribindex, size, type, relativeoffset);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
	_ptrc_glVertexAttribLFormat = (PFN_PTRC_GLVERTEXATTRIBLFORMATPROC)IntGetProcAddress("glVertexAttribLFormat");
	_ptrc_glVertexAttribLFormat(attribindex, size, type, relativeoffset);
}

static void CODEGEN_FUNCPTR Switch_VertexBindingDivisor(GLuint bindingindex, GLuint divisor)
{
	_ptrc_glVertexBindingDivisor = (PFN_PTRC_GLVERTEXBINDINGDIVISORPROC)IntGetProcAddress("glVertexBindingDivisor");
	_ptrc_glVertexBindingDivisor(bindingindex, divisor);
}


// Extension: ARB_viewport_array
static void CODEGEN_FUNCPTR Switch_DepthRangeArrayv(GLuint first, GLsizei count, const GLdouble * v)
{
	_ptrc_glDepthRangeArrayv = (PFN_PTRC_GLDEPTHRANGEARRAYVPROC)IntGetProcAddress("glDepthRangeArrayv");
	_ptrc_glDepthRangeArrayv(first, count, v);
}

static void CODEGEN_FUNCPTR Switch_DepthRangeIndexed(GLuint index, GLdouble n, GLdouble f)
{
	_ptrc_glDepthRangeIndexed = (PFN_PTRC_GLDEPTHRANGEINDEXEDPROC)IntGetProcAddress("glDepthRangeIndexed");
	_ptrc_glDepthRangeIndexed(index, n, f);
}

static void CODEGEN_FUNCPTR Switch_GetDoublei_v(GLenum target, GLuint index, GLdouble * data)
{
	_ptrc_glGetDoublei_v = (PFN_PTRC_GLGETDOUBLEI_VPROC)IntGetProcAddress("glGetDoublei_v");
	_ptrc_glGetDoublei_v(target, index, data);
}

static void CODEGEN_FUNCPTR Switch_GetFloati_v(GLenum target, GLuint index, GLfloat * data)
{
	_ptrc_glGetFloati_v = (PFN_PTRC_GLGETFLOATI_VPROC)IntGetProcAddress("glGetFloati_v");
	_ptrc_glGetFloati_v(target, index, data);
}

static void CODEGEN_FUNCPTR Switch_ScissorArrayv(GLuint first, GLsizei count, const GLint * v)
{
	_ptrc_glScissorArrayv = (PFN_PTRC_GLSCISSORARRAYVPROC)IntGetProcAddress("glScissorArrayv");
	_ptrc_glScissorArrayv(first, count, v);
}

static void CODEGEN_FUNCPTR Switch_ScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
	_ptrc_glScissorIndexed = (PFN_PTRC_GLSCISSORINDEXEDPROC)IntGetProcAddress("glScissorIndexed");
	_ptrc_glScissorIndexed(index, left, bottom, width, height);
}

static void CODEGEN_FUNCPTR Switch_ScissorIndexedv(GLuint index, const GLint * v)
{
	_ptrc_glScissorIndexedv = (PFN_PTRC_GLSCISSORINDEXEDVPROC)IntGetProcAddress("glScissorIndexedv");
	_ptrc_glScissorIndexedv(index, v);
}

static void CODEGEN_FUNCPTR Switch_ViewportArrayv(GLuint first, GLsizei count, const GLfloat * v)
{
	_ptrc_glViewportArrayv = (PFN_PTRC_GLVIEWPORTARRAYVPROC)IntGetProcAddress("glViewportArrayv");
	_ptrc_glViewportArrayv(first, count, v);
}

static void CODEGEN_FUNCPTR Switch_ViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	_ptrc_glViewportIndexedf = (PFN_PTRC_GLVIEWPORTINDEXEDFPROC)IntGetProcAddress("glViewportIndexedf");
	_ptrc_glViewportIndexedf(index, x, y, w, h);
}

static void CODEGEN_FUNCPTR Switch_ViewportIndexedfv(GLuint index, const GLfloat * v)
{
	_ptrc_glViewportIndexedfv = (PFN_PTRC_GLVIEWPORTINDEXEDFVPROC)IntGetProcAddress("glViewportIndexedfv");
	_ptrc_glViewportIndexedfv(index, v);
}


// Extension: ARB_clear_buffer_object
static void CODEGEN_FUNCPTR Switch_ClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void * data)
{
	_ptrc_glClearBufferData = (PFN_PTRC_GLCLEARBUFFERDATAPROC)IntGetProcAddress("glClearBufferData");
	_ptrc_glClearBufferData(target, internalformat, format, type, data);
}

static void CODEGEN_FUNCPTR Switch_ClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void * data)
{
	_ptrc_glClearBufferSubData = (PFN_PTRC_GLCLEARBUFFERSUBDATAPROC)IntGetProcAddress("glClearBufferSubData");
	_ptrc_glClearBufferSubData(target, internalformat, offset, size, format, type, data);
}


// Extension: ARB_copy_image
static void CODEGEN_FUNCPTR Switch_CopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
	_ptrc_glCopyImageSubData = (PFN_PTRC_GLCOPYIMAGESUBDATAPROC)IntGetProcAddress("glCopyImageSubData");
	_ptrc_glCopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
}


// Extension: ARB_framebuffer_no_attachments
static void CODEGEN_FUNCPTR Switch_FramebufferParameteri(GLenum target, GLenum pname, GLint param)
{
	_ptrc_glFramebufferParameteri = (PFN_PTRC_GLFRAMEBUFFERPARAMETERIPROC)IntGetProcAddress("glFramebufferParameteri");
	_ptrc_glFramebufferParameteri(target, pname, param);
}

static void CODEGEN_FUNCPTR Switch_GetFramebufferParameteriv(GLenum target, GLenum pname, GLint * params)
{
	_ptrc_glGetFramebufferParameteriv = (PFN_PTRC_GLGETFRAMEBUFFERPARAMETERIVPROC)IntGetProcAddress("glGetFramebufferParameteriv");
	_ptrc_glGetFramebufferParameteriv(target, pname, params);
}


// Extension: ARB_invalidate_subdata
static void CODEGEN_FUNCPTR Switch_InvalidateBufferData(GLuint buffer)
{
	_ptrc_glInvalidateBufferData = (PFN_PTRC_GLINVALIDATEBUFFERDATAPROC)IntGetProcAddress("glInvalidateBufferData");
	_ptrc_glInvalidateBufferData(buffer);
}

static void CODEGEN_FUNCPTR Switch_InvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
	_ptrc_glInvalidateBufferSubData = (PFN_PTRC_GLINVALIDATEBUFFERSUBDATAPROC)IntGetProcAddress("glInvalidateBufferSubData");
	_ptrc_glInvalidateBufferSubData(buffer, offset, length);
}

static void CODEGEN_FUNCPTR Switch_InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum * attachments)
{
	_ptrc_glInvalidateFramebuffer = (PFN_PTRC_GLINVALIDATEFRAMEBUFFERPROC)IntGetProcAddress("glInvalidateFramebuffer");
	_ptrc_glInvalidateFramebuffer(target, numAttachments, attachments);
}

static void CODEGEN_FUNCPTR Switch_InvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
	_ptrc_glInvalidateSubFramebuffer = (PFN_PTRC_GLINVALIDATESUBFRAMEBUFFERPROC)IntGetProcAddress("glInvalidateSubFramebuffer");
	_ptrc_glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
}

static void CODEGEN_FUNCPTR Switch_InvalidateTexImage(GLuint texture, GLint level)
{
	_ptrc_glInvalidateTexImage = (PFN_PTRC_GLINVALIDATETEXIMAGEPROC)IntGetProcAddress("glInvalidateTexImage");
	_ptrc_glInvalidateTexImage(texture, level);
}

static void CODEGEN_FUNCPTR Switch_InvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)
{
	_ptrc_glInvalidateTexSubImage = (PFN_PTRC_GLINVALIDATETEXSUBIMAGEPROC)IntGetProcAddress("glInvalidateTexSubImage");
	_ptrc_glInvalidateTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth);
}


// Extension: ARB_texture_storage_multisample
static void CODEGEN_FUNCPTR Switch_TexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	_ptrc_glTexStorage2DMultisample = (PFN_PTRC_GLTEXSTORAGE2DMULTISAMPLEPROC)IntGetProcAddress("glTexStorage2DMultisample");
	_ptrc_glTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

static void CODEGEN_FUNCPTR Switch_TexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	_ptrc_glTexStorage3DMultisample = (PFN_PTRC_GLTEXSTORAGE3DMULTISAMPLEPROC)IntGetProcAddress("glTexStorage3DMultisample");
	_ptrc_glTexStorage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}


// Extension: KHR_debug
static void CODEGEN_FUNCPTR Switch_DebugMessageCallback(GLDEBUGPROC callback, const void * userParam)
{
	_ptrc_glDebugMessageCallback = (PFN_PTRC_GLDEBUGMESSAGECALLBACKPROC)IntGetProcAddress("glDebugMessageCallback");
	_ptrc_glDebugMessageCallback(callback, userParam);
}

static void CODEGEN_FUNCPTR Switch_DebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled)
{
	_ptrc_glDebugMessageControl = (PFN_PTRC_GLDEBUGMESSAGECONTROLPROC)IntGetProcAddress("glDebugMessageControl");
	_ptrc_glDebugMessageControl(source, type, severity, count, ids, enabled);
}

static void CODEGEN_FUNCPTR Switch_DebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * buf)
{
	_ptrc_glDebugMessageInsert = (PFN_PTRC_GLDEBUGMESSAGEINSERTPROC)IntGetProcAddress("glDebugMessageInsert");
	_ptrc_glDebugMessageInsert(source, type, id, severity, length, buf);
}

static GLuint CODEGEN_FUNCPTR Switch_GetDebugMessageLog(GLuint count, GLsizei bufsize, GLenum * sources, GLenum * types, GLuint * ids, GLenum * severities, GLsizei * lengths, GLchar * messageLog)
{
	_ptrc_glGetDebugMessageLog = (PFN_PTRC_GLGETDEBUGMESSAGELOGPROC)IntGetProcAddress("glGetDebugMessageLog");
	return _ptrc_glGetDebugMessageLog(count, bufsize, sources, types, ids, severities, lengths, messageLog);
}

static void CODEGEN_FUNCPTR Switch_GetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei * length, GLchar * label)
{
	_ptrc_glGetObjectLabel = (PFN_PTRC_GLGETOBJECTLABELPROC)IntGetProcAddress("glGetObjectLabel");
	_ptrc_glGetObjectLabel(identifier, name, bufSize, length, label);
}

static void CODEGEN_FUNCPTR Switch_GetObjectPtrLabel(const void * ptr, GLsizei bufSize, GLsizei * length, GLchar * label)
{
	_ptrc_glGetObjectPtrLabel = (PFN_PTRC_GLGETOBJECTPTRLABELPROC)IntGetProcAddress("glGetObjectPtrLabel");
	_ptrc_glGetObjectPtrLabel(ptr, bufSize, length, label);
}

static void CODEGEN_FUNCPTR Switch_GetPointerv(GLenum pname, GLvoid ** params)
{
	_ptrc_glGetPointerv = (PFN_PTRC_GLGETPOINTERVPROC)IntGetProcAddress("glGetPointerv");
	_ptrc_glGetPointerv(pname, params);
}

static void CODEGEN_FUNCPTR Switch_ObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar * label)
{
	_ptrc_glObjectLabel = (PFN_PTRC_GLOBJECTLABELPROC)IntGetProcAddress("glObjectLabel");
	_ptrc_glObjectLabel(identifier, name, length, label);
}

static void CODEGEN_FUNCPTR Switch_ObjectPtrLabel(const void * ptr, GLsizei length, const GLchar * label)
{
	_ptrc_glObjectPtrLabel = (PFN_PTRC_GLOBJECTPTRLABELPROC)IntGetProcAddress("glObjectPtrLabel");
	_ptrc_glObjectPtrLabel(ptr, length, label);
}

static void CODEGEN_FUNCPTR Switch_PopDebugGroup()
{
	_ptrc_glPopDebugGroup = (PFN_PTRC_GLPOPDEBUGGROUPPROC)IntGetProcAddress("glPopDebugGroup");
	_ptrc_glPopDebugGroup();
}

static void CODEGEN_FUNCPTR Switch_PushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
	_ptrc_glPushDebugGroup = (PFN_PTRC_GLPUSHDEBUGGROUPPROC)IntGetProcAddress("glPushDebugGroup");
	_ptrc_glPushDebugGroup(source, id, length, message);
}


// Extension: 1.0
static void CODEGEN_FUNCPTR Switch_BlendFunc(GLenum sfactor, GLenum dfactor)
{
	_ptrc_glBlendFunc = (PFN_PTRC_GLBLENDFUNCPROC)IntGetProcAddress("glBlendFunc");
	_ptrc_glBlendFunc(sfactor, dfactor);
}

static void CODEGEN_FUNCPTR Switch_Clear(GLbitfield mask)
{
	_ptrc_glClear = (PFN_PTRC_GLCLEARPROC)IntGetProcAddress("glClear");
	_ptrc_glClear(mask);
}

static void CODEGEN_FUNCPTR Switch_ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	_ptrc_glClearColor = (PFN_PTRC_GLCLEARCOLORPROC)IntGetProcAddress("glClearColor");
	_ptrc_glClearColor(red, green, blue, alpha);
}

static void CODEGEN_FUNCPTR Switch_ClearDepth(GLdouble depth)
{
	_ptrc_glClearDepth = (PFN_PTRC_GLCLEARDEPTHPROC)IntGetProcAddress("glClearDepth");
	_ptrc_glClearDepth(depth);
}

static void CODEGEN_FUNCPTR Switch_ClearStencil(GLint s)
{
	_ptrc_glClearStencil = (PFN_PTRC_GLCLEARSTENCILPROC)IntGetProcAddress("glClearStencil");
	_ptrc_glClearStencil(s);
}

static void CODEGEN_FUNCPTR Switch_ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	_ptrc_glColorMask = (PFN_PTRC_GLCOLORMASKPROC)IntGetProcAddress("glColorMask");
	_ptrc_glColorMask(red, green, blue, alpha);
}

static void CODEGEN_FUNCPTR Switch_CullFace(GLenum mode)
{
	_ptrc_glCullFace = (PFN_PTRC_GLCULLFACEPROC)IntGetProcAddress("glCullFace");
	_ptrc_glCullFace(mode);
}

static void CODEGEN_FUNCPTR Switch_DepthFunc(GLenum func)
{
	_ptrc_glDepthFunc = (PFN_PTRC_GLDEPTHFUNCPROC)IntGetProcAddress("glDepthFunc");
	_ptrc_glDepthFunc(func);
}

static void CODEGEN_FUNCPTR Switch_DepthMask(GLboolean flag)
{
	_ptrc_glDepthMask = (PFN_PTRC_GLDEPTHMASKPROC)IntGetProcAddress("glDepthMask");
	_ptrc_glDepthMask(flag);
}

static void CODEGEN_FUNCPTR Switch_DepthRange(GLdouble ren_near, GLdouble ren_far)
{
	_ptrc_glDepthRange = (PFN_PTRC_GLDEPTHRANGEPROC)IntGetProcAddress("glDepthRange");
	_ptrc_glDepthRange(ren_near, ren_far);
}

static void CODEGEN_FUNCPTR Switch_Disable(GLenum cap)
{
	_ptrc_glDisable = (PFN_PTRC_GLDISABLEPROC)IntGetProcAddress("glDisable");
	_ptrc_glDisable(cap);
}

static void CODEGEN_FUNCPTR Switch_DrawBuffer(GLenum mode)
{
	_ptrc_glDrawBuffer = (PFN_PTRC_GLDRAWBUFFERPROC)IntGetProcAddress("glDrawBuffer");
	_ptrc_glDrawBuffer(mode);
}

static void CODEGEN_FUNCPTR Switch_Enable(GLenum cap)
{
	_ptrc_glEnable = (PFN_PTRC_GLENABLEPROC)IntGetProcAddress("glEnable");
	_ptrc_glEnable(cap);
}

static void CODEGEN_FUNCPTR Switch_Finish()
{
	_ptrc_glFinish = (PFN_PTRC_GLFINISHPROC)IntGetProcAddress("glFinish");
	_ptrc_glFinish();
}

static void CODEGEN_FUNCPTR Switch_Flush()
{
	_ptrc_glFlush = (PFN_PTRC_GLFLUSHPROC)IntGetProcAddress("glFlush");
	_ptrc_glFlush();
}

static void CODEGEN_FUNCPTR Switch_FrontFace(GLenum mode)
{
	_ptrc_glFrontFace = (PFN_PTRC_GLFRONTFACEPROC)IntGetProcAddress("glFrontFace");
	_ptrc_glFrontFace(mode);
}

static void CODEGEN_FUNCPTR Switch_GetBooleanv(GLenum pname, GLboolean * params)
{
	_ptrc_glGetBooleanv = (PFN_PTRC_GLGETBOOLEANVPROC)IntGetProcAddress("glGetBooleanv");
	_ptrc_glGetBooleanv(pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetDoublev(GLenum pname, GLdouble * params)
{
	_ptrc_glGetDoublev = (PFN_PTRC_GLGETDOUBLEVPROC)IntGetProcAddress("glGetDoublev");
	_ptrc_glGetDoublev(pname, params);
}

static GLenum CODEGEN_FUNCPTR Switch_GetError()
{
	_ptrc_glGetError = (PFN_PTRC_GLGETERRORPROC)IntGetProcAddress("glGetError");
	return _ptrc_glGetError();
}

static void CODEGEN_FUNCPTR Switch_GetFloatv(GLenum pname, GLfloat * params)
{
	_ptrc_glGetFloatv = (PFN_PTRC_GLGETFLOATVPROC)IntGetProcAddress("glGetFloatv");
	_ptrc_glGetFloatv(pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetIntegerv(GLenum pname, GLint * params)
{
	_ptrc_glGetIntegerv = (PFN_PTRC_GLGETINTEGERVPROC)IntGetProcAddress("glGetIntegerv");
	_ptrc_glGetIntegerv(pname, params);
}

static const GLubyte * CODEGEN_FUNCPTR Switch_GetString(GLenum name)
{
	_ptrc_glGetString = (PFN_PTRC_GLGETSTRINGPROC)IntGetProcAddress("glGetString");
	return _ptrc_glGetString(name);
}

static void CODEGEN_FUNCPTR Switch_GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels)
{
	_ptrc_glGetTexImage = (PFN_PTRC_GLGETTEXIMAGEPROC)IntGetProcAddress("glGetTexImage");
	_ptrc_glGetTexImage(target, level, format, type, pixels);
}

static void CODEGEN_FUNCPTR Switch_GetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat * params)
{
	_ptrc_glGetTexLevelParameterfv = (PFN_PTRC_GLGETTEXLEVELPARAMETERFVPROC)IntGetProcAddress("glGetTexLevelParameterfv");
	_ptrc_glGetTexLevelParameterfv(target, level, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint * params)
{
	_ptrc_glGetTexLevelParameteriv = (PFN_PTRC_GLGETTEXLEVELPARAMETERIVPROC)IntGetProcAddress("glGetTexLevelParameteriv");
	_ptrc_glGetTexLevelParameteriv(target, level, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetTexParameterfv(GLenum target, GLenum pname, GLfloat * params)
{
	_ptrc_glGetTexParameterfv = (PFN_PTRC_GLGETTEXPARAMETERFVPROC)IntGetProcAddress("glGetTexParameterfv");
	_ptrc_glGetTexParameterfv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetTexParameteriv(GLenum target, GLenum pname, GLint * params)
{
	_ptrc_glGetTexParameteriv = (PFN_PTRC_GLGETTEXPARAMETERIVPROC)IntGetProcAddress("glGetTexParameteriv");
	_ptrc_glGetTexParameteriv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_Hint(GLenum target, GLenum mode)
{
	_ptrc_glHint = (PFN_PTRC_GLHINTPROC)IntGetProcAddress("glHint");
	_ptrc_glHint(target, mode);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsEnabled(GLenum cap)
{
	_ptrc_glIsEnabled = (PFN_PTRC_GLISENABLEDPROC)IntGetProcAddress("glIsEnabled");
	return _ptrc_glIsEnabled(cap);
}

static void CODEGEN_FUNCPTR Switch_LineWidth(GLfloat width)
{
	_ptrc_glLineWidth = (PFN_PTRC_GLLINEWIDTHPROC)IntGetProcAddress("glLineWidth");
	_ptrc_glLineWidth(width);
}

static void CODEGEN_FUNCPTR Switch_LogicOp(GLenum opcode)
{
	_ptrc_glLogicOp = (PFN_PTRC_GLLOGICOPPROC)IntGetProcAddress("glLogicOp");
	_ptrc_glLogicOp(opcode);
}

static void CODEGEN_FUNCPTR Switch_PixelStoref(GLenum pname, GLfloat param)
{
	_ptrc_glPixelStoref = (PFN_PTRC_GLPIXELSTOREFPROC)IntGetProcAddress("glPixelStoref");
	_ptrc_glPixelStoref(pname, param);
}

static void CODEGEN_FUNCPTR Switch_PixelStorei(GLenum pname, GLint param)
{
	_ptrc_glPixelStorei = (PFN_PTRC_GLPIXELSTOREIPROC)IntGetProcAddress("glPixelStorei");
	_ptrc_glPixelStorei(pname, param);
}

static void CODEGEN_FUNCPTR Switch_PointSize(GLfloat size)
{
	_ptrc_glPointSize = (PFN_PTRC_GLPOINTSIZEPROC)IntGetProcAddress("glPointSize");
	_ptrc_glPointSize(size);
}

static void CODEGEN_FUNCPTR Switch_PolygonMode(GLenum face, GLenum mode)
{
	_ptrc_glPolygonMode = (PFN_PTRC_GLPOLYGONMODEPROC)IntGetProcAddress("glPolygonMode");
	_ptrc_glPolygonMode(face, mode);
}

static void CODEGEN_FUNCPTR Switch_ReadBuffer(GLenum mode)
{
	_ptrc_glReadBuffer = (PFN_PTRC_GLREADBUFFERPROC)IntGetProcAddress("glReadBuffer");
	_ptrc_glReadBuffer(mode);
}

static void CODEGEN_FUNCPTR Switch_ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels)
{
	_ptrc_glReadPixels = (PFN_PTRC_GLREADPIXELSPROC)IntGetProcAddress("glReadPixels");
	_ptrc_glReadPixels(x, y, width, height, format, type, pixels);
}

static void CODEGEN_FUNCPTR Switch_Scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	_ptrc_glScissor = (PFN_PTRC_GLSCISSORPROC)IntGetProcAddress("glScissor");
	_ptrc_glScissor(x, y, width, height);
}

static void CODEGEN_FUNCPTR Switch_StencilFunc(GLenum func, GLint ref, GLuint mask)
{
	_ptrc_glStencilFunc = (PFN_PTRC_GLSTENCILFUNCPROC)IntGetProcAddress("glStencilFunc");
	_ptrc_glStencilFunc(func, ref, mask);
}

static void CODEGEN_FUNCPTR Switch_StencilMask(GLuint mask)
{
	_ptrc_glStencilMask = (PFN_PTRC_GLSTENCILMASKPROC)IntGetProcAddress("glStencilMask");
	_ptrc_glStencilMask(mask);
}

static void CODEGEN_FUNCPTR Switch_StencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	_ptrc_glStencilOp = (PFN_PTRC_GLSTENCILOPPROC)IntGetProcAddress("glStencilOp");
	_ptrc_glStencilOp(fail, zfail, zpass);
}

static void CODEGEN_FUNCPTR Switch_TexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels)
{
	_ptrc_glTexImage1D = (PFN_PTRC_GLTEXIMAGE1DPROC)IntGetProcAddress("glTexImage1D");
	_ptrc_glTexImage1D(target, level, internalformat, width, border, format, type, pixels);
}

static void CODEGEN_FUNCPTR Switch_TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels)
{
	_ptrc_glTexImage2D = (PFN_PTRC_GLTEXIMAGE2DPROC)IntGetProcAddress("glTexImage2D");
	_ptrc_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

static void CODEGEN_FUNCPTR Switch_TexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	_ptrc_glTexParameterf = (PFN_PTRC_GLTEXPARAMETERFPROC)IntGetProcAddress("glTexParameterf");
	_ptrc_glTexParameterf(target, pname, param);
}

static void CODEGEN_FUNCPTR Switch_TexParameterfv(GLenum target, GLenum pname, const GLfloat * params)
{
	_ptrc_glTexParameterfv = (PFN_PTRC_GLTEXPARAMETERFVPROC)IntGetProcAddress("glTexParameterfv");
	_ptrc_glTexParameterfv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_TexParameteri(GLenum target, GLenum pname, GLint param)
{
	_ptrc_glTexParameteri = (PFN_PTRC_GLTEXPARAMETERIPROC)IntGetProcAddress("glTexParameteri");
	_ptrc_glTexParameteri(target, pname, param);
}

static void CODEGEN_FUNCPTR Switch_TexParameteriv(GLenum target, GLenum pname, const GLint * params)
{
	_ptrc_glTexParameteriv = (PFN_PTRC_GLTEXPARAMETERIVPROC)IntGetProcAddress("glTexParameteriv");
	_ptrc_glTexParameteriv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	_ptrc_glViewport = (PFN_PTRC_GLVIEWPORTPROC)IntGetProcAddress("glViewport");
	_ptrc_glViewport(x, y, width, height);
}


// Extension: 1.1
static void CODEGEN_FUNCPTR Switch_BindTexture(GLenum target, GLuint texture)
{
	_ptrc_glBindTexture = (PFN_PTRC_GLBINDTEXTUREPROC)IntGetProcAddress("glBindTexture");
	_ptrc_glBindTexture(target, texture);
}

static void CODEGEN_FUNCPTR Switch_CopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	_ptrc_glCopyTexImage1D = (PFN_PTRC_GLCOPYTEXIMAGE1DPROC)IntGetProcAddress("glCopyTexImage1D");
	_ptrc_glCopyTexImage1D(target, level, internalformat, x, y, width, border);
}

static void CODEGEN_FUNCPTR Switch_CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	_ptrc_glCopyTexImage2D = (PFN_PTRC_GLCOPYTEXIMAGE2DPROC)IntGetProcAddress("glCopyTexImage2D");
	_ptrc_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

static void CODEGEN_FUNCPTR Switch_CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	_ptrc_glCopyTexSubImage1D = (PFN_PTRC_GLCOPYTEXSUBIMAGE1DPROC)IntGetProcAddress("glCopyTexSubImage1D");
	_ptrc_glCopyTexSubImage1D(target, level, xoffset, x, y, width);
}

static void CODEGEN_FUNCPTR Switch_CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	_ptrc_glCopyTexSubImage2D = (PFN_PTRC_GLCOPYTEXSUBIMAGE2DPROC)IntGetProcAddress("glCopyTexSubImage2D");
	_ptrc_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

static void CODEGEN_FUNCPTR Switch_DeleteTextures(GLsizei n, const GLuint * textures)
{
	_ptrc_glDeleteTextures = (PFN_PTRC_GLDELETETEXTURESPROC)IntGetProcAddress("glDeleteTextures");
	_ptrc_glDeleteTextures(n, textures);
}

static void CODEGEN_FUNCPTR Switch_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
	_ptrc_glDrawArrays = (PFN_PTRC_GLDRAWARRAYSPROC)IntGetProcAddress("glDrawArrays");
	_ptrc_glDrawArrays(mode, first, count);
}

static void CODEGEN_FUNCPTR Switch_DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
	_ptrc_glDrawElements = (PFN_PTRC_GLDRAWELEMENTSPROC)IntGetProcAddress("glDrawElements");
	_ptrc_glDrawElements(mode, count, type, indices);
}

static void CODEGEN_FUNCPTR Switch_GenTextures(GLsizei n, GLuint * textures)
{
	_ptrc_glGenTextures = (PFN_PTRC_GLGENTEXTURESPROC)IntGetProcAddress("glGenTextures");
	_ptrc_glGenTextures(n, textures);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsTexture(GLuint texture)
{
	_ptrc_glIsTexture = (PFN_PTRC_GLISTEXTUREPROC)IntGetProcAddress("glIsTexture");
	return _ptrc_glIsTexture(texture);
}

static void CODEGEN_FUNCPTR Switch_PolygonOffset(GLfloat factor, GLfloat units)
{
	_ptrc_glPolygonOffset = (PFN_PTRC_GLPOLYGONOFFSETPROC)IntGetProcAddress("glPolygonOffset");
	_ptrc_glPolygonOffset(factor, units);
}

static void CODEGEN_FUNCPTR Switch_TexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels)
{
	_ptrc_glTexSubImage1D = (PFN_PTRC_GLTEXSUBIMAGE1DPROC)IntGetProcAddress("glTexSubImage1D");
	_ptrc_glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
}

static void CODEGEN_FUNCPTR Switch_TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels)
{
	_ptrc_glTexSubImage2D = (PFN_PTRC_GLTEXSUBIMAGE2DPROC)IntGetProcAddress("glTexSubImage2D");
	_ptrc_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}


// Extension: 1.2
static void CODEGEN_FUNCPTR Switch_BlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	_ptrc_glBlendColor = (PFN_PTRC_GLBLENDCOLORPROC)IntGetProcAddress("glBlendColor");
	_ptrc_glBlendColor(red, green, blue, alpha);
}

static void CODEGEN_FUNCPTR Switch_BlendEquation(GLenum mode)
{
	_ptrc_glBlendEquation = (PFN_PTRC_GLBLENDEQUATIONPROC)IntGetProcAddress("glBlendEquation");
	_ptrc_glBlendEquation(mode);
}

static void CODEGEN_FUNCPTR Switch_CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	_ptrc_glCopyTexSubImage3D = (PFN_PTRC_GLCOPYTEXSUBIMAGE3DPROC)IntGetProcAddress("glCopyTexSubImage3D");
	_ptrc_glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

static void CODEGEN_FUNCPTR Switch_DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices)
{
	_ptrc_glDrawRangeElements = (PFN_PTRC_GLDRAWRANGEELEMENTSPROC)IntGetProcAddress("glDrawRangeElements");
	_ptrc_glDrawRangeElements(mode, start, end, count, type, indices);
}

static void CODEGEN_FUNCPTR Switch_TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels)
{
	_ptrc_glTexImage3D = (PFN_PTRC_GLTEXIMAGE3DPROC)IntGetProcAddress("glTexImage3D");
	_ptrc_glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

static void CODEGEN_FUNCPTR Switch_TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels)
{
	_ptrc_glTexSubImage3D = (PFN_PTRC_GLTEXSUBIMAGE3DPROC)IntGetProcAddress("glTexSubImage3D");
	_ptrc_glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}


// Extension: 1.3
static void CODEGEN_FUNCPTR Switch_ActiveTexture(GLenum texture)
{
	_ptrc_glActiveTexture = (PFN_PTRC_GLACTIVETEXTUREPROC)IntGetProcAddress("glActiveTexture");
	_ptrc_glActiveTexture(texture);
}

static void CODEGEN_FUNCPTR Switch_CompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data)
{
	_ptrc_glCompressedTexImage1D = (PFN_PTRC_GLCOMPRESSEDTEXIMAGE1DPROC)IntGetProcAddress("glCompressedTexImage1D");
	_ptrc_glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
}

static void CODEGEN_FUNCPTR Switch_CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data)
{
	_ptrc_glCompressedTexImage2D = (PFN_PTRC_GLCOMPRESSEDTEXIMAGE2DPROC)IntGetProcAddress("glCompressedTexImage2D");
	_ptrc_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

static void CODEGEN_FUNCPTR Switch_CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data)
{
	_ptrc_glCompressedTexImage3D = (PFN_PTRC_GLCOMPRESSEDTEXIMAGE3DPROC)IntGetProcAddress("glCompressedTexImage3D");
	_ptrc_glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

static void CODEGEN_FUNCPTR Switch_CompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data)
{
	_ptrc_glCompressedTexSubImage1D = (PFN_PTRC_GLCOMPRESSEDTEXSUBIMAGE1DPROC)IntGetProcAddress("glCompressedTexSubImage1D");
	_ptrc_glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
}

static void CODEGEN_FUNCPTR Switch_CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data)
{
	_ptrc_glCompressedTexSubImage2D = (PFN_PTRC_GLCOMPRESSEDTEXSUBIMAGE2DPROC)IntGetProcAddress("glCompressedTexSubImage2D");
	_ptrc_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

static void CODEGEN_FUNCPTR Switch_CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data)
{
	_ptrc_glCompressedTexSubImage3D = (PFN_PTRC_GLCOMPRESSEDTEXSUBIMAGE3DPROC)IntGetProcAddress("glCompressedTexSubImage3D");
	_ptrc_glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

static void CODEGEN_FUNCPTR Switch_GetCompressedTexImage(GLenum target, GLint level, GLvoid * img)
{
	_ptrc_glGetCompressedTexImage = (PFN_PTRC_GLGETCOMPRESSEDTEXIMAGEPROC)IntGetProcAddress("glGetCompressedTexImage");
	_ptrc_glGetCompressedTexImage(target, level, img);
}

static void CODEGEN_FUNCPTR Switch_SampleCoverage(GLfloat value, GLboolean invert)
{
	_ptrc_glSampleCoverage = (PFN_PTRC_GLSAMPLECOVERAGEPROC)IntGetProcAddress("glSampleCoverage");
	_ptrc_glSampleCoverage(value, invert);
}


// Extension: 1.4
static void CODEGEN_FUNCPTR Switch_BlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	_ptrc_glBlendFuncSeparate = (PFN_PTRC_GLBLENDFUNCSEPARATEPROC)IntGetProcAddress("glBlendFuncSeparate");
	_ptrc_glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

static void CODEGEN_FUNCPTR Switch_MultiDrawArrays(GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount)
{
	_ptrc_glMultiDrawArrays = (PFN_PTRC_GLMULTIDRAWARRAYSPROC)IntGetProcAddress("glMultiDrawArrays");
	_ptrc_glMultiDrawArrays(mode, first, count, drawcount);
}

static void CODEGEN_FUNCPTR Switch_MultiDrawElements(GLenum mode, const GLsizei * count, GLenum type, const GLvoid *const* indices, GLsizei drawcount)
{
	_ptrc_glMultiDrawElements = (PFN_PTRC_GLMULTIDRAWELEMENTSPROC)IntGetProcAddress("glMultiDrawElements");
	_ptrc_glMultiDrawElements(mode, count, type, indices, drawcount);
}

static void CODEGEN_FUNCPTR Switch_PointParameterf(GLenum pname, GLfloat param)
{
	_ptrc_glPointParameterf = (PFN_PTRC_GLPOINTPARAMETERFPROC)IntGetProcAddress("glPointParameterf");
	_ptrc_glPointParameterf(pname, param);
}

static void CODEGEN_FUNCPTR Switch_PointParameterfv(GLenum pname, const GLfloat * params)
{
	_ptrc_glPointParameterfv = (PFN_PTRC_GLPOINTPARAMETERFVPROC)IntGetProcAddress("glPointParameterfv");
	_ptrc_glPointParameterfv(pname, params);
}

static void CODEGEN_FUNCPTR Switch_PointParameteri(GLenum pname, GLint param)
{
	_ptrc_glPointParameteri = (PFN_PTRC_GLPOINTPARAMETERIPROC)IntGetProcAddress("glPointParameteri");
	_ptrc_glPointParameteri(pname, param);
}

static void CODEGEN_FUNCPTR Switch_PointParameteriv(GLenum pname, const GLint * params)
{
	_ptrc_glPointParameteriv = (PFN_PTRC_GLPOINTPARAMETERIVPROC)IntGetProcAddress("glPointParameteriv");
	_ptrc_glPointParameteriv(pname, params);
}


// Extension: 1.5
static void CODEGEN_FUNCPTR Switch_BeginQuery(GLenum target, GLuint id)
{
	_ptrc_glBeginQuery = (PFN_PTRC_GLBEGINQUERYPROC)IntGetProcAddress("glBeginQuery");
	_ptrc_glBeginQuery(target, id);
}

static void CODEGEN_FUNCPTR Switch_BindBuffer(GLenum target, GLuint buffer)
{
	_ptrc_glBindBuffer = (PFN_PTRC_GLBINDBUFFERPROC)IntGetProcAddress("glBindBuffer");
	_ptrc_glBindBuffer(target, buffer);
}

static void CODEGEN_FUNCPTR Switch_BufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage)
{
	_ptrc_glBufferData = (PFN_PTRC_GLBUFFERDATAPROC)IntGetProcAddress("glBufferData");
	_ptrc_glBufferData(target, size, data, usage);
}

static void CODEGEN_FUNCPTR Switch_BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data)
{
	_ptrc_glBufferSubData = (PFN_PTRC_GLBUFFERSUBDATAPROC)IntGetProcAddress("glBufferSubData");
	_ptrc_glBufferSubData(target, offset, size, data);
}

static void CODEGEN_FUNCPTR Switch_DeleteBuffers(GLsizei n, const GLuint * buffers)
{
	_ptrc_glDeleteBuffers = (PFN_PTRC_GLDELETEBUFFERSPROC)IntGetProcAddress("glDeleteBuffers");
	_ptrc_glDeleteBuffers(n, buffers);
}

static void CODEGEN_FUNCPTR Switch_DeleteQueries(GLsizei n, const GLuint * ids)
{
	_ptrc_glDeleteQueries = (PFN_PTRC_GLDELETEQUERIESPROC)IntGetProcAddress("glDeleteQueries");
	_ptrc_glDeleteQueries(n, ids);
}

static void CODEGEN_FUNCPTR Switch_EndQuery(GLenum target)
{
	_ptrc_glEndQuery = (PFN_PTRC_GLENDQUERYPROC)IntGetProcAddress("glEndQuery");
	_ptrc_glEndQuery(target);
}

static void CODEGEN_FUNCPTR Switch_GenBuffers(GLsizei n, GLuint * buffers)
{
	_ptrc_glGenBuffers = (PFN_PTRC_GLGENBUFFERSPROC)IntGetProcAddress("glGenBuffers");
	_ptrc_glGenBuffers(n, buffers);
}

static void CODEGEN_FUNCPTR Switch_GenQueries(GLsizei n, GLuint * ids)
{
	_ptrc_glGenQueries = (PFN_PTRC_GLGENQUERIESPROC)IntGetProcAddress("glGenQueries");
	_ptrc_glGenQueries(n, ids);
}

static void CODEGEN_FUNCPTR Switch_GetBufferParameteriv(GLenum target, GLenum pname, GLint * params)
{
	_ptrc_glGetBufferParameteriv = (PFN_PTRC_GLGETBUFFERPARAMETERIVPROC)IntGetProcAddress("glGetBufferParameteriv");
	_ptrc_glGetBufferParameteriv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetBufferPointerv(GLenum target, GLenum pname, GLvoid ** params)
{
	_ptrc_glGetBufferPointerv = (PFN_PTRC_GLGETBUFFERPOINTERVPROC)IntGetProcAddress("glGetBufferPointerv");
	_ptrc_glGetBufferPointerv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid * data)
{
	_ptrc_glGetBufferSubData = (PFN_PTRC_GLGETBUFFERSUBDATAPROC)IntGetProcAddress("glGetBufferSubData");
	_ptrc_glGetBufferSubData(target, offset, size, data);
}

static void CODEGEN_FUNCPTR Switch_GetQueryObjectiv(GLuint id, GLenum pname, GLint * params)
{
	_ptrc_glGetQueryObjectiv = (PFN_PTRC_GLGETQUERYOBJECTIVPROC)IntGetProcAddress("glGetQueryObjectiv");
	_ptrc_glGetQueryObjectiv(id, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetQueryObjectuiv(GLuint id, GLenum pname, GLuint * params)
{
	_ptrc_glGetQueryObjectuiv = (PFN_PTRC_GLGETQUERYOBJECTUIVPROC)IntGetProcAddress("glGetQueryObjectuiv");
	_ptrc_glGetQueryObjectuiv(id, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetQueryiv(GLenum target, GLenum pname, GLint * params)
{
	_ptrc_glGetQueryiv = (PFN_PTRC_GLGETQUERYIVPROC)IntGetProcAddress("glGetQueryiv");
	_ptrc_glGetQueryiv(target, pname, params);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsBuffer(GLuint buffer)
{
	_ptrc_glIsBuffer = (PFN_PTRC_GLISBUFFERPROC)IntGetProcAddress("glIsBuffer");
	return _ptrc_glIsBuffer(buffer);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsQuery(GLuint id)
{
	_ptrc_glIsQuery = (PFN_PTRC_GLISQUERYPROC)IntGetProcAddress("glIsQuery");
	return _ptrc_glIsQuery(id);
}

static void * CODEGEN_FUNCPTR Switch_MapBuffer(GLenum target, GLenum access)
{
	_ptrc_glMapBuffer = (PFN_PTRC_GLMAPBUFFERPROC)IntGetProcAddress("glMapBuffer");
	return _ptrc_glMapBuffer(target, access);
}

static GLboolean CODEGEN_FUNCPTR Switch_UnmapBuffer(GLenum target)
{
	_ptrc_glUnmapBuffer = (PFN_PTRC_GLUNMAPBUFFERPROC)IntGetProcAddress("glUnmapBuffer");
	return _ptrc_glUnmapBuffer(target);
}


// Extension: 2.0
static void CODEGEN_FUNCPTR Switch_AttachShader(GLuint program, GLuint shader)
{
	_ptrc_glAttachShader = (PFN_PTRC_GLATTACHSHADERPROC)IntGetProcAddress("glAttachShader");
	_ptrc_glAttachShader(program, shader);
}

static void CODEGEN_FUNCPTR Switch_BindAttribLocation(GLuint program, GLuint index, const GLchar * name)
{
	_ptrc_glBindAttribLocation = (PFN_PTRC_GLBINDATTRIBLOCATIONPROC)IntGetProcAddress("glBindAttribLocation");
	_ptrc_glBindAttribLocation(program, index, name);
}

static void CODEGEN_FUNCPTR Switch_BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
	_ptrc_glBlendEquationSeparate = (PFN_PTRC_GLBLENDEQUATIONSEPARATEPROC)IntGetProcAddress("glBlendEquationSeparate");
	_ptrc_glBlendEquationSeparate(modeRGB, modeAlpha);
}

static void CODEGEN_FUNCPTR Switch_CompileShader(GLuint shader)
{
	_ptrc_glCompileShader = (PFN_PTRC_GLCOMPILESHADERPROC)IntGetProcAddress("glCompileShader");
	_ptrc_glCompileShader(shader);
}

static GLuint CODEGEN_FUNCPTR Switch_CreateProgram()
{
	_ptrc_glCreateProgram = (PFN_PTRC_GLCREATEPROGRAMPROC)IntGetProcAddress("glCreateProgram");
	return _ptrc_glCreateProgram();
}

static GLuint CODEGEN_FUNCPTR Switch_CreateShader(GLenum type)
{
	_ptrc_glCreateShader = (PFN_PTRC_GLCREATESHADERPROC)IntGetProcAddress("glCreateShader");
	return _ptrc_glCreateShader(type);
}

static void CODEGEN_FUNCPTR Switch_DeleteProgram(GLuint program)
{
	_ptrc_glDeleteProgram = (PFN_PTRC_GLDELETEPROGRAMPROC)IntGetProcAddress("glDeleteProgram");
	_ptrc_glDeleteProgram(program);
}

static void CODEGEN_FUNCPTR Switch_DeleteShader(GLuint shader)
{
	_ptrc_glDeleteShader = (PFN_PTRC_GLDELETESHADERPROC)IntGetProcAddress("glDeleteShader");
	_ptrc_glDeleteShader(shader);
}

static void CODEGEN_FUNCPTR Switch_DetachShader(GLuint program, GLuint shader)
{
	_ptrc_glDetachShader = (PFN_PTRC_GLDETACHSHADERPROC)IntGetProcAddress("glDetachShader");
	_ptrc_glDetachShader(program, shader);
}

static void CODEGEN_FUNCPTR Switch_DisableVertexAttribArray(GLuint index)
{
	_ptrc_glDisableVertexAttribArray = (PFN_PTRC_GLDISABLEVERTEXATTRIBARRAYPROC)IntGetProcAddress("glDisableVertexAttribArray");
	_ptrc_glDisableVertexAttribArray(index);
}

static void CODEGEN_FUNCPTR Switch_DrawBuffers(GLsizei n, const GLenum * bufs)
{
	_ptrc_glDrawBuffers = (PFN_PTRC_GLDRAWBUFFERSPROC)IntGetProcAddress("glDrawBuffers");
	_ptrc_glDrawBuffers(n, bufs);
}

static void CODEGEN_FUNCPTR Switch_EnableVertexAttribArray(GLuint index)
{
	_ptrc_glEnableVertexAttribArray = (PFN_PTRC_GLENABLEVERTEXATTRIBARRAYPROC)IntGetProcAddress("glEnableVertexAttribArray");
	_ptrc_glEnableVertexAttribArray(index);
}

static void CODEGEN_FUNCPTR Switch_GetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name)
{
	_ptrc_glGetActiveAttrib = (PFN_PTRC_GLGETACTIVEATTRIBPROC)IntGetProcAddress("glGetActiveAttrib");
	_ptrc_glGetActiveAttrib(program, index, bufSize, length, size, type, name);
}

static void CODEGEN_FUNCPTR Switch_GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name)
{
	_ptrc_glGetActiveUniform = (PFN_PTRC_GLGETACTIVEUNIFORMPROC)IntGetProcAddress("glGetActiveUniform");
	_ptrc_glGetActiveUniform(program, index, bufSize, length, size, type, name);
}

static void CODEGEN_FUNCPTR Switch_GetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * shaders)
{
	_ptrc_glGetAttachedShaders = (PFN_PTRC_GLGETATTACHEDSHADERSPROC)IntGetProcAddress("glGetAttachedShaders");
	_ptrc_glGetAttachedShaders(program, maxCount, count, shaders);
}

static GLint CODEGEN_FUNCPTR Switch_GetAttribLocation(GLuint program, const GLchar * name)
{
	_ptrc_glGetAttribLocation = (PFN_PTRC_GLGETATTRIBLOCATIONPROC)IntGetProcAddress("glGetAttribLocation");
	return _ptrc_glGetAttribLocation(program, name);
}

static void CODEGEN_FUNCPTR Switch_GetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
	_ptrc_glGetProgramInfoLog = (PFN_PTRC_GLGETPROGRAMINFOLOGPROC)IntGetProcAddress("glGetProgramInfoLog");
	_ptrc_glGetProgramInfoLog(program, bufSize, length, infoLog);
}

static void CODEGEN_FUNCPTR Switch_GetProgramiv(GLuint program, GLenum pname, GLint * params)
{
	_ptrc_glGetProgramiv = (PFN_PTRC_GLGETPROGRAMIVPROC)IntGetProcAddress("glGetProgramiv");
	_ptrc_glGetProgramiv(program, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
	_ptrc_glGetShaderInfoLog = (PFN_PTRC_GLGETSHADERINFOLOGPROC)IntGetProcAddress("glGetShaderInfoLog");
	_ptrc_glGetShaderInfoLog(shader, bufSize, length, infoLog);
}

static void CODEGEN_FUNCPTR Switch_GetShaderSource(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source)
{
	_ptrc_glGetShaderSource = (PFN_PTRC_GLGETSHADERSOURCEPROC)IntGetProcAddress("glGetShaderSource");
	_ptrc_glGetShaderSource(shader, bufSize, length, source);
}

static void CODEGEN_FUNCPTR Switch_GetShaderiv(GLuint shader, GLenum pname, GLint * params)
{
	_ptrc_glGetShaderiv = (PFN_PTRC_GLGETSHADERIVPROC)IntGetProcAddress("glGetShaderiv");
	_ptrc_glGetShaderiv(shader, pname, params);
}

static GLint CODEGEN_FUNCPTR Switch_GetUniformLocation(GLuint program, const GLchar * name)
{
	_ptrc_glGetUniformLocation = (PFN_PTRC_GLGETUNIFORMLOCATIONPROC)IntGetProcAddress("glGetUniformLocation");
	return _ptrc_glGetUniformLocation(program, name);
}

static void CODEGEN_FUNCPTR Switch_GetUniformfv(GLuint program, GLint location, GLfloat * params)
{
	_ptrc_glGetUniformfv = (PFN_PTRC_GLGETUNIFORMFVPROC)IntGetProcAddress("glGetUniformfv");
	_ptrc_glGetUniformfv(program, location, params);
}

static void CODEGEN_FUNCPTR Switch_GetUniformiv(GLuint program, GLint location, GLint * params)
{
	_ptrc_glGetUniformiv = (PFN_PTRC_GLGETUNIFORMIVPROC)IntGetProcAddress("glGetUniformiv");
	_ptrc_glGetUniformiv(program, location, params);
}

static void CODEGEN_FUNCPTR Switch_GetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid ** pointer)
{
	_ptrc_glGetVertexAttribPointerv = (PFN_PTRC_GLGETVERTEXATTRIBPOINTERVPROC)IntGetProcAddress("glGetVertexAttribPointerv");
	_ptrc_glGetVertexAttribPointerv(index, pname, pointer);
}

static void CODEGEN_FUNCPTR Switch_GetVertexAttribdv(GLuint index, GLenum pname, GLdouble * params)
{
	_ptrc_glGetVertexAttribdv = (PFN_PTRC_GLGETVERTEXATTRIBDVPROC)IntGetProcAddress("glGetVertexAttribdv");
	_ptrc_glGetVertexAttribdv(index, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetVertexAttribfv(GLuint index, GLenum pname, GLfloat * params)
{
	_ptrc_glGetVertexAttribfv = (PFN_PTRC_GLGETVERTEXATTRIBFVPROC)IntGetProcAddress("glGetVertexAttribfv");
	_ptrc_glGetVertexAttribfv(index, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetVertexAttribiv(GLuint index, GLenum pname, GLint * params)
{
	_ptrc_glGetVertexAttribiv = (PFN_PTRC_GLGETVERTEXATTRIBIVPROC)IntGetProcAddress("glGetVertexAttribiv");
	_ptrc_glGetVertexAttribiv(index, pname, params);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsProgram(GLuint program)
{
	_ptrc_glIsProgram = (PFN_PTRC_GLISPROGRAMPROC)IntGetProcAddress("glIsProgram");
	return _ptrc_glIsProgram(program);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsShader(GLuint shader)
{
	_ptrc_glIsShader = (PFN_PTRC_GLISSHADERPROC)IntGetProcAddress("glIsShader");
	return _ptrc_glIsShader(shader);
}

static void CODEGEN_FUNCPTR Switch_LinkProgram(GLuint program)
{
	_ptrc_glLinkProgram = (PFN_PTRC_GLLINKPROGRAMPROC)IntGetProcAddress("glLinkProgram");
	_ptrc_glLinkProgram(program);
}

static void CODEGEN_FUNCPTR Switch_ShaderSource(GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length)
{
	_ptrc_glShaderSource = (PFN_PTRC_GLSHADERSOURCEPROC)IntGetProcAddress("glShaderSource");
	_ptrc_glShaderSource(shader, count, string, length);
}

static void CODEGEN_FUNCPTR Switch_StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
	_ptrc_glStencilFuncSeparate = (PFN_PTRC_GLSTENCILFUNCSEPARATEPROC)IntGetProcAddress("glStencilFuncSeparate");
	_ptrc_glStencilFuncSeparate(face, func, ref, mask);
}

static void CODEGEN_FUNCPTR Switch_StencilMaskSeparate(GLenum face, GLuint mask)
{
	_ptrc_glStencilMaskSeparate = (PFN_PTRC_GLSTENCILMASKSEPARATEPROC)IntGetProcAddress("glStencilMaskSeparate");
	_ptrc_glStencilMaskSeparate(face, mask);
}

static void CODEGEN_FUNCPTR Switch_StencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	_ptrc_glStencilOpSeparate = (PFN_PTRC_GLSTENCILOPSEPARATEPROC)IntGetProcAddress("glStencilOpSeparate");
	_ptrc_glStencilOpSeparate(face, sfail, dpfail, dppass);
}

static void CODEGEN_FUNCPTR Switch_Uniform1f(GLint location, GLfloat v0)
{
	_ptrc_glUniform1f = (PFN_PTRC_GLUNIFORM1FPROC)IntGetProcAddress("glUniform1f");
	_ptrc_glUniform1f(location, v0);
}

static void CODEGEN_FUNCPTR Switch_Uniform1fv(GLint location, GLsizei count, const GLfloat * value)
{
	_ptrc_glUniform1fv = (PFN_PTRC_GLUNIFORM1FVPROC)IntGetProcAddress("glUniform1fv");
	_ptrc_glUniform1fv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform1i(GLint location, GLint v0)
{
	_ptrc_glUniform1i = (PFN_PTRC_GLUNIFORM1IPROC)IntGetProcAddress("glUniform1i");
	_ptrc_glUniform1i(location, v0);
}

static void CODEGEN_FUNCPTR Switch_Uniform1iv(GLint location, GLsizei count, const GLint * value)
{
	_ptrc_glUniform1iv = (PFN_PTRC_GLUNIFORM1IVPROC)IntGetProcAddress("glUniform1iv");
	_ptrc_glUniform1iv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform2f(GLint location, GLfloat v0, GLfloat v1)
{
	_ptrc_glUniform2f = (PFN_PTRC_GLUNIFORM2FPROC)IntGetProcAddress("glUniform2f");
	_ptrc_glUniform2f(location, v0, v1);
}

static void CODEGEN_FUNCPTR Switch_Uniform2fv(GLint location, GLsizei count, const GLfloat * value)
{
	_ptrc_glUniform2fv = (PFN_PTRC_GLUNIFORM2FVPROC)IntGetProcAddress("glUniform2fv");
	_ptrc_glUniform2fv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform2i(GLint location, GLint v0, GLint v1)
{
	_ptrc_glUniform2i = (PFN_PTRC_GLUNIFORM2IPROC)IntGetProcAddress("glUniform2i");
	_ptrc_glUniform2i(location, v0, v1);
}

static void CODEGEN_FUNCPTR Switch_Uniform2iv(GLint location, GLsizei count, const GLint * value)
{
	_ptrc_glUniform2iv = (PFN_PTRC_GLUNIFORM2IVPROC)IntGetProcAddress("glUniform2iv");
	_ptrc_glUniform2iv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	_ptrc_glUniform3f = (PFN_PTRC_GLUNIFORM3FPROC)IntGetProcAddress("glUniform3f");
	_ptrc_glUniform3f(location, v0, v1, v2);
}

static void CODEGEN_FUNCPTR Switch_Uniform3fv(GLint location, GLsizei count, const GLfloat * value)
{
	_ptrc_glUniform3fv = (PFN_PTRC_GLUNIFORM3FVPROC)IntGetProcAddress("glUniform3fv");
	_ptrc_glUniform3fv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
	_ptrc_glUniform3i = (PFN_PTRC_GLUNIFORM3IPROC)IntGetProcAddress("glUniform3i");
	_ptrc_glUniform3i(location, v0, v1, v2);
}

static void CODEGEN_FUNCPTR Switch_Uniform3iv(GLint location, GLsizei count, const GLint * value)
{
	_ptrc_glUniform3iv = (PFN_PTRC_GLUNIFORM3IVPROC)IntGetProcAddress("glUniform3iv");
	_ptrc_glUniform3iv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	_ptrc_glUniform4f = (PFN_PTRC_GLUNIFORM4FPROC)IntGetProcAddress("glUniform4f");
	_ptrc_glUniform4f(location, v0, v1, v2, v3);
}

static void CODEGEN_FUNCPTR Switch_Uniform4fv(GLint location, GLsizei count, const GLfloat * value)
{
	_ptrc_glUniform4fv = (PFN_PTRC_GLUNIFORM4FVPROC)IntGetProcAddress("glUniform4fv");
	_ptrc_glUniform4fv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	_ptrc_glUniform4i = (PFN_PTRC_GLUNIFORM4IPROC)IntGetProcAddress("glUniform4i");
	_ptrc_glUniform4i(location, v0, v1, v2, v3);
}

static void CODEGEN_FUNCPTR Switch_Uniform4iv(GLint location, GLsizei count, const GLint * value)
{
	_ptrc_glUniform4iv = (PFN_PTRC_GLUNIFORM4IVPROC)IntGetProcAddress("glUniform4iv");
	_ptrc_glUniform4iv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glUniformMatrix2fv = (PFN_PTRC_GLUNIFORMMATRIX2FVPROC)IntGetProcAddress("glUniformMatrix2fv");
	_ptrc_glUniformMatrix2fv(location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glUniformMatrix3fv = (PFN_PTRC_GLUNIFORMMATRIX3FVPROC)IntGetProcAddress("glUniformMatrix3fv");
	_ptrc_glUniformMatrix3fv(location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glUniformMatrix4fv = (PFN_PTRC_GLUNIFORMMATRIX4FVPROC)IntGetProcAddress("glUniformMatrix4fv");
	_ptrc_glUniformMatrix4fv(location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_UseProgram(GLuint program)
{
	_ptrc_glUseProgram = (PFN_PTRC_GLUSEPROGRAMPROC)IntGetProcAddress("glUseProgram");
	_ptrc_glUseProgram(program);
}

static void CODEGEN_FUNCPTR Switch_ValidateProgram(GLuint program)
{
	_ptrc_glValidateProgram = (PFN_PTRC_GLVALIDATEPROGRAMPROC)IntGetProcAddress("glValidateProgram");
	_ptrc_glValidateProgram(program);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib1d(GLuint index, GLdouble x)
{
	_ptrc_glVertexAttrib1d = (PFN_PTRC_GLVERTEXATTRIB1DPROC)IntGetProcAddress("glVertexAttrib1d");
	_ptrc_glVertexAttrib1d(index, x);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib1dv(GLuint index, const GLdouble * v)
{
	_ptrc_glVertexAttrib1dv = (PFN_PTRC_GLVERTEXATTRIB1DVPROC)IntGetProcAddress("glVertexAttrib1dv");
	_ptrc_glVertexAttrib1dv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib1f(GLuint index, GLfloat x)
{
	_ptrc_glVertexAttrib1f = (PFN_PTRC_GLVERTEXATTRIB1FPROC)IntGetProcAddress("glVertexAttrib1f");
	_ptrc_glVertexAttrib1f(index, x);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib1fv(GLuint index, const GLfloat * v)
{
	_ptrc_glVertexAttrib1fv = (PFN_PTRC_GLVERTEXATTRIB1FVPROC)IntGetProcAddress("glVertexAttrib1fv");
	_ptrc_glVertexAttrib1fv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib1s(GLuint index, GLshort x)
{
	_ptrc_glVertexAttrib1s = (PFN_PTRC_GLVERTEXATTRIB1SPROC)IntGetProcAddress("glVertexAttrib1s");
	_ptrc_glVertexAttrib1s(index, x);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib1sv(GLuint index, const GLshort * v)
{
	_ptrc_glVertexAttrib1sv = (PFN_PTRC_GLVERTEXATTRIB1SVPROC)IntGetProcAddress("glVertexAttrib1sv");
	_ptrc_glVertexAttrib1sv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
	_ptrc_glVertexAttrib2d = (PFN_PTRC_GLVERTEXATTRIB2DPROC)IntGetProcAddress("glVertexAttrib2d");
	_ptrc_glVertexAttrib2d(index, x, y);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib2dv(GLuint index, const GLdouble * v)
{
	_ptrc_glVertexAttrib2dv = (PFN_PTRC_GLVERTEXATTRIB2DVPROC)IntGetProcAddress("glVertexAttrib2dv");
	_ptrc_glVertexAttrib2dv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
	_ptrc_glVertexAttrib2f = (PFN_PTRC_GLVERTEXATTRIB2FPROC)IntGetProcAddress("glVertexAttrib2f");
	_ptrc_glVertexAttrib2f(index, x, y);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib2fv(GLuint index, const GLfloat * v)
{
	_ptrc_glVertexAttrib2fv = (PFN_PTRC_GLVERTEXATTRIB2FVPROC)IntGetProcAddress("glVertexAttrib2fv");
	_ptrc_glVertexAttrib2fv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
	_ptrc_glVertexAttrib2s = (PFN_PTRC_GLVERTEXATTRIB2SPROC)IntGetProcAddress("glVertexAttrib2s");
	_ptrc_glVertexAttrib2s(index, x, y);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib2sv(GLuint index, const GLshort * v)
{
	_ptrc_glVertexAttrib2sv = (PFN_PTRC_GLVERTEXATTRIB2SVPROC)IntGetProcAddress("glVertexAttrib2sv");
	_ptrc_glVertexAttrib2sv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	_ptrc_glVertexAttrib3d = (PFN_PTRC_GLVERTEXATTRIB3DPROC)IntGetProcAddress("glVertexAttrib3d");
	_ptrc_glVertexAttrib3d(index, x, y, z);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib3dv(GLuint index, const GLdouble * v)
{
	_ptrc_glVertexAttrib3dv = (PFN_PTRC_GLVERTEXATTRIB3DVPROC)IntGetProcAddress("glVertexAttrib3dv");
	_ptrc_glVertexAttrib3dv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	_ptrc_glVertexAttrib3f = (PFN_PTRC_GLVERTEXATTRIB3FPROC)IntGetProcAddress("glVertexAttrib3f");
	_ptrc_glVertexAttrib3f(index, x, y, z);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib3fv(GLuint index, const GLfloat * v)
{
	_ptrc_glVertexAttrib3fv = (PFN_PTRC_GLVERTEXATTRIB3FVPROC)IntGetProcAddress("glVertexAttrib3fv");
	_ptrc_glVertexAttrib3fv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
	_ptrc_glVertexAttrib3s = (PFN_PTRC_GLVERTEXATTRIB3SPROC)IntGetProcAddress("glVertexAttrib3s");
	_ptrc_glVertexAttrib3s(index, x, y, z);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib3sv(GLuint index, const GLshort * v)
{
	_ptrc_glVertexAttrib3sv = (PFN_PTRC_GLVERTEXATTRIB3SVPROC)IntGetProcAddress("glVertexAttrib3sv");
	_ptrc_glVertexAttrib3sv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nbv(GLuint index, const GLbyte * v)
{
	_ptrc_glVertexAttrib4Nbv = (PFN_PTRC_GLVERTEXATTRIB4NBVPROC)IntGetProcAddress("glVertexAttrib4Nbv");
	_ptrc_glVertexAttrib4Nbv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4Niv(GLuint index, const GLint * v)
{
	_ptrc_glVertexAttrib4Niv = (PFN_PTRC_GLVERTEXATTRIB4NIVPROC)IntGetProcAddress("glVertexAttrib4Niv");
	_ptrc_glVertexAttrib4Niv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nsv(GLuint index, const GLshort * v)
{
	_ptrc_glVertexAttrib4Nsv = (PFN_PTRC_GLVERTEXATTRIB4NSVPROC)IntGetProcAddress("glVertexAttrib4Nsv");
	_ptrc_glVertexAttrib4Nsv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
	_ptrc_glVertexAttrib4Nub = (PFN_PTRC_GLVERTEXATTRIB4NUBPROC)IntGetProcAddress("glVertexAttrib4Nub");
	_ptrc_glVertexAttrib4Nub(index, x, y, z, w);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nubv(GLuint index, const GLubyte * v)
{
	_ptrc_glVertexAttrib4Nubv = (PFN_PTRC_GLVERTEXATTRIB4NUBVPROC)IntGetProcAddress("glVertexAttrib4Nubv");
	_ptrc_glVertexAttrib4Nubv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nuiv(GLuint index, const GLuint * v)
{
	_ptrc_glVertexAttrib4Nuiv = (PFN_PTRC_GLVERTEXATTRIB4NUIVPROC)IntGetProcAddress("glVertexAttrib4Nuiv");
	_ptrc_glVertexAttrib4Nuiv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4Nusv(GLuint index, const GLushort * v)
{
	_ptrc_glVertexAttrib4Nusv = (PFN_PTRC_GLVERTEXATTRIB4NUSVPROC)IntGetProcAddress("glVertexAttrib4Nusv");
	_ptrc_glVertexAttrib4Nusv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4bv(GLuint index, const GLbyte * v)
{
	_ptrc_glVertexAttrib4bv = (PFN_PTRC_GLVERTEXATTRIB4BVPROC)IntGetProcAddress("glVertexAttrib4bv");
	_ptrc_glVertexAttrib4bv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	_ptrc_glVertexAttrib4d = (PFN_PTRC_GLVERTEXATTRIB4DPROC)IntGetProcAddress("glVertexAttrib4d");
	_ptrc_glVertexAttrib4d(index, x, y, z, w);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4dv(GLuint index, const GLdouble * v)
{
	_ptrc_glVertexAttrib4dv = (PFN_PTRC_GLVERTEXATTRIB4DVPROC)IntGetProcAddress("glVertexAttrib4dv");
	_ptrc_glVertexAttrib4dv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	_ptrc_glVertexAttrib4f = (PFN_PTRC_GLVERTEXATTRIB4FPROC)IntGetProcAddress("glVertexAttrib4f");
	_ptrc_glVertexAttrib4f(index, x, y, z, w);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4fv(GLuint index, const GLfloat * v)
{
	_ptrc_glVertexAttrib4fv = (PFN_PTRC_GLVERTEXATTRIB4FVPROC)IntGetProcAddress("glVertexAttrib4fv");
	_ptrc_glVertexAttrib4fv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4iv(GLuint index, const GLint * v)
{
	_ptrc_glVertexAttrib4iv = (PFN_PTRC_GLVERTEXATTRIB4IVPROC)IntGetProcAddress("glVertexAttrib4iv");
	_ptrc_glVertexAttrib4iv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
	_ptrc_glVertexAttrib4s = (PFN_PTRC_GLVERTEXATTRIB4SPROC)IntGetProcAddress("glVertexAttrib4s");
	_ptrc_glVertexAttrib4s(index, x, y, z, w);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4sv(GLuint index, const GLshort * v)
{
	_ptrc_glVertexAttrib4sv = (PFN_PTRC_GLVERTEXATTRIB4SVPROC)IntGetProcAddress("glVertexAttrib4sv");
	_ptrc_glVertexAttrib4sv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4ubv(GLuint index, const GLubyte * v)
{
	_ptrc_glVertexAttrib4ubv = (PFN_PTRC_GLVERTEXATTRIB4UBVPROC)IntGetProcAddress("glVertexAttrib4ubv");
	_ptrc_glVertexAttrib4ubv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4uiv(GLuint index, const GLuint * v)
{
	_ptrc_glVertexAttrib4uiv = (PFN_PTRC_GLVERTEXATTRIB4UIVPROC)IntGetProcAddress("glVertexAttrib4uiv");
	_ptrc_glVertexAttrib4uiv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttrib4usv(GLuint index, const GLushort * v)
{
	_ptrc_glVertexAttrib4usv = (PFN_PTRC_GLVERTEXATTRIB4USVPROC)IntGetProcAddress("glVertexAttrib4usv");
	_ptrc_glVertexAttrib4usv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
{
	_ptrc_glVertexAttribPointer = (PFN_PTRC_GLVERTEXATTRIBPOINTERPROC)IntGetProcAddress("glVertexAttribPointer");
	_ptrc_glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}


// Extension: 2.1
static void CODEGEN_FUNCPTR Switch_UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glUniformMatrix2x3fv = (PFN_PTRC_GLUNIFORMMATRIX2X3FVPROC)IntGetProcAddress("glUniformMatrix2x3fv");
	_ptrc_glUniformMatrix2x3fv(location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glUniformMatrix2x4fv = (PFN_PTRC_GLUNIFORMMATRIX2X4FVPROC)IntGetProcAddress("glUniformMatrix2x4fv");
	_ptrc_glUniformMatrix2x4fv(location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glUniformMatrix3x2fv = (PFN_PTRC_GLUNIFORMMATRIX3X2FVPROC)IntGetProcAddress("glUniformMatrix3x2fv");
	_ptrc_glUniformMatrix3x2fv(location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glUniformMatrix3x4fv = (PFN_PTRC_GLUNIFORMMATRIX3X4FVPROC)IntGetProcAddress("glUniformMatrix3x4fv");
	_ptrc_glUniformMatrix3x4fv(location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glUniformMatrix4x2fv = (PFN_PTRC_GLUNIFORMMATRIX4X2FVPROC)IntGetProcAddress("glUniformMatrix4x2fv");
	_ptrc_glUniformMatrix4x2fv(location, count, transpose, value);
}

static void CODEGEN_FUNCPTR Switch_UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	_ptrc_glUniformMatrix4x3fv = (PFN_PTRC_GLUNIFORMMATRIX4X3FVPROC)IntGetProcAddress("glUniformMatrix4x3fv");
	_ptrc_glUniformMatrix4x3fv(location, count, transpose, value);
}


// Extension: 3.0
static void CODEGEN_FUNCPTR Switch_BeginConditionalRender(GLuint id, GLenum mode)
{
	_ptrc_glBeginConditionalRender = (PFN_PTRC_GLBEGINCONDITIONALRENDERPROC)IntGetProcAddress("glBeginConditionalRender");
	_ptrc_glBeginConditionalRender(id, mode);
}

static void CODEGEN_FUNCPTR Switch_BeginTransformFeedback(GLenum primitiveMode)
{
	_ptrc_glBeginTransformFeedback = (PFN_PTRC_GLBEGINTRANSFORMFEEDBACKPROC)IntGetProcAddress("glBeginTransformFeedback");
	_ptrc_glBeginTransformFeedback(primitiveMode);
}

static void CODEGEN_FUNCPTR Switch_BindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	_ptrc_glBindBufferBase = (PFN_PTRC_GLBINDBUFFERBASEPROC)IntGetProcAddress("glBindBufferBase");
	_ptrc_glBindBufferBase(target, index, buffer);
}

static void CODEGEN_FUNCPTR Switch_BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	_ptrc_glBindBufferRange = (PFN_PTRC_GLBINDBUFFERRANGEPROC)IntGetProcAddress("glBindBufferRange");
	_ptrc_glBindBufferRange(target, index, buffer, offset, size);
}

static void CODEGEN_FUNCPTR Switch_BindFragDataLocation(GLuint program, GLuint color, const GLchar * name)
{
	_ptrc_glBindFragDataLocation = (PFN_PTRC_GLBINDFRAGDATALOCATIONPROC)IntGetProcAddress("glBindFragDataLocation");
	_ptrc_glBindFragDataLocation(program, color, name);
}

static void CODEGEN_FUNCPTR Switch_BindFramebuffer(GLenum target, GLuint framebuffer)
{
	_ptrc_glBindFramebuffer = (PFN_PTRC_GLBINDFRAMEBUFFERPROC)IntGetProcAddress("glBindFramebuffer");
	_ptrc_glBindFramebuffer(target, framebuffer);
}

static void CODEGEN_FUNCPTR Switch_BindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	_ptrc_glBindRenderbuffer = (PFN_PTRC_GLBINDRENDERBUFFERPROC)IntGetProcAddress("glBindRenderbuffer");
	_ptrc_glBindRenderbuffer(target, renderbuffer);
}

static void CODEGEN_FUNCPTR Switch_BindVertexArray(GLuint ren_array)
{
	_ptrc_glBindVertexArray = (PFN_PTRC_GLBINDVERTEXARRAYPROC)IntGetProcAddress("glBindVertexArray");
	_ptrc_glBindVertexArray(ren_array);
}

static void CODEGEN_FUNCPTR Switch_BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	_ptrc_glBlitFramebuffer = (PFN_PTRC_GLBLITFRAMEBUFFERPROC)IntGetProcAddress("glBlitFramebuffer");
	_ptrc_glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

static GLenum CODEGEN_FUNCPTR Switch_CheckFramebufferStatus(GLenum target)
{
	_ptrc_glCheckFramebufferStatus = (PFN_PTRC_GLCHECKFRAMEBUFFERSTATUSPROC)IntGetProcAddress("glCheckFramebufferStatus");
	return _ptrc_glCheckFramebufferStatus(target);
}

static void CODEGEN_FUNCPTR Switch_ClampColor(GLenum target, GLenum clamp)
{
	_ptrc_glClampColor = (PFN_PTRC_GLCLAMPCOLORPROC)IntGetProcAddress("glClampColor");
	_ptrc_glClampColor(target, clamp);
}

static void CODEGEN_FUNCPTR Switch_ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	_ptrc_glClearBufferfi = (PFN_PTRC_GLCLEARBUFFERFIPROC)IntGetProcAddress("glClearBufferfi");
	_ptrc_glClearBufferfi(buffer, drawbuffer, depth, stencil);
}

static void CODEGEN_FUNCPTR Switch_ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat * value)
{
	_ptrc_glClearBufferfv = (PFN_PTRC_GLCLEARBUFFERFVPROC)IntGetProcAddress("glClearBufferfv");
	_ptrc_glClearBufferfv(buffer, drawbuffer, value);
}

static void CODEGEN_FUNCPTR Switch_ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint * value)
{
	_ptrc_glClearBufferiv = (PFN_PTRC_GLCLEARBUFFERIVPROC)IntGetProcAddress("glClearBufferiv");
	_ptrc_glClearBufferiv(buffer, drawbuffer, value);
}

static void CODEGEN_FUNCPTR Switch_ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint * value)
{
	_ptrc_glClearBufferuiv = (PFN_PTRC_GLCLEARBUFFERUIVPROC)IntGetProcAddress("glClearBufferuiv");
	_ptrc_glClearBufferuiv(buffer, drawbuffer, value);
}

static void CODEGEN_FUNCPTR Switch_ColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
	_ptrc_glColorMaski = (PFN_PTRC_GLCOLORMASKIPROC)IntGetProcAddress("glColorMaski");
	_ptrc_glColorMaski(index, r, g, b, a);
}

static void CODEGEN_FUNCPTR Switch_DeleteFramebuffers(GLsizei n, const GLuint * framebuffers)
{
	_ptrc_glDeleteFramebuffers = (PFN_PTRC_GLDELETEFRAMEBUFFERSPROC)IntGetProcAddress("glDeleteFramebuffers");
	_ptrc_glDeleteFramebuffers(n, framebuffers);
}

static void CODEGEN_FUNCPTR Switch_DeleteRenderbuffers(GLsizei n, const GLuint * renderbuffers)
{
	_ptrc_glDeleteRenderbuffers = (PFN_PTRC_GLDELETERENDERBUFFERSPROC)IntGetProcAddress("glDeleteRenderbuffers");
	_ptrc_glDeleteRenderbuffers(n, renderbuffers);
}

static void CODEGEN_FUNCPTR Switch_DeleteVertexArrays(GLsizei n, const GLuint * arrays)
{
	_ptrc_glDeleteVertexArrays = (PFN_PTRC_GLDELETEVERTEXARRAYSPROC)IntGetProcAddress("glDeleteVertexArrays");
	_ptrc_glDeleteVertexArrays(n, arrays);
}

static void CODEGEN_FUNCPTR Switch_Disablei(GLenum target, GLuint index)
{
	_ptrc_glDisablei = (PFN_PTRC_GLDISABLEIPROC)IntGetProcAddress("glDisablei");
	_ptrc_glDisablei(target, index);
}

static void CODEGEN_FUNCPTR Switch_Enablei(GLenum target, GLuint index)
{
	_ptrc_glEnablei = (PFN_PTRC_GLENABLEIPROC)IntGetProcAddress("glEnablei");
	_ptrc_glEnablei(target, index);
}

static void CODEGEN_FUNCPTR Switch_EndConditionalRender()
{
	_ptrc_glEndConditionalRender = (PFN_PTRC_GLENDCONDITIONALRENDERPROC)IntGetProcAddress("glEndConditionalRender");
	_ptrc_glEndConditionalRender();
}

static void CODEGEN_FUNCPTR Switch_EndTransformFeedback()
{
	_ptrc_glEndTransformFeedback = (PFN_PTRC_GLENDTRANSFORMFEEDBACKPROC)IntGetProcAddress("glEndTransformFeedback");
	_ptrc_glEndTransformFeedback();
}

static void CODEGEN_FUNCPTR Switch_FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	_ptrc_glFlushMappedBufferRange = (PFN_PTRC_GLFLUSHMAPPEDBUFFERRANGEPROC)IntGetProcAddress("glFlushMappedBufferRange");
	_ptrc_glFlushMappedBufferRange(target, offset, length);
}

static void CODEGEN_FUNCPTR Switch_FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	_ptrc_glFramebufferRenderbuffer = (PFN_PTRC_GLFRAMEBUFFERRENDERBUFFERPROC)IntGetProcAddress("glFramebufferRenderbuffer");
	_ptrc_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

static void CODEGEN_FUNCPTR Switch_FramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	_ptrc_glFramebufferTexture1D = (PFN_PTRC_GLFRAMEBUFFERTEXTURE1DPROC)IntGetProcAddress("glFramebufferTexture1D");
	_ptrc_glFramebufferTexture1D(target, attachment, textarget, texture, level);
}

static void CODEGEN_FUNCPTR Switch_FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	_ptrc_glFramebufferTexture2D = (PFN_PTRC_GLFRAMEBUFFERTEXTURE2DPROC)IntGetProcAddress("glFramebufferTexture2D");
	_ptrc_glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

static void CODEGEN_FUNCPTR Switch_FramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	_ptrc_glFramebufferTexture3D = (PFN_PTRC_GLFRAMEBUFFERTEXTURE3DPROC)IntGetProcAddress("glFramebufferTexture3D");
	_ptrc_glFramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
}

static void CODEGEN_FUNCPTR Switch_FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	_ptrc_glFramebufferTextureLayer = (PFN_PTRC_GLFRAMEBUFFERTEXTURELAYERPROC)IntGetProcAddress("glFramebufferTextureLayer");
	_ptrc_glFramebufferTextureLayer(target, attachment, texture, level, layer);
}

static void CODEGEN_FUNCPTR Switch_GenFramebuffers(GLsizei n, GLuint * framebuffers)
{
	_ptrc_glGenFramebuffers = (PFN_PTRC_GLGENFRAMEBUFFERSPROC)IntGetProcAddress("glGenFramebuffers");
	_ptrc_glGenFramebuffers(n, framebuffers);
}

static void CODEGEN_FUNCPTR Switch_GenRenderbuffers(GLsizei n, GLuint * renderbuffers)
{
	_ptrc_glGenRenderbuffers = (PFN_PTRC_GLGENRENDERBUFFERSPROC)IntGetProcAddress("glGenRenderbuffers");
	_ptrc_glGenRenderbuffers(n, renderbuffers);
}

static void CODEGEN_FUNCPTR Switch_GenVertexArrays(GLsizei n, GLuint * arrays)
{
	_ptrc_glGenVertexArrays = (PFN_PTRC_GLGENVERTEXARRAYSPROC)IntGetProcAddress("glGenVertexArrays");
	_ptrc_glGenVertexArrays(n, arrays);
}

static void CODEGEN_FUNCPTR Switch_GenerateMipmap(GLenum target)
{
	_ptrc_glGenerateMipmap = (PFN_PTRC_GLGENERATEMIPMAPPROC)IntGetProcAddress("glGenerateMipmap");
	_ptrc_glGenerateMipmap(target);
}

static void CODEGEN_FUNCPTR Switch_GetBooleani_v(GLenum target, GLuint index, GLboolean * data)
{
	_ptrc_glGetBooleani_v = (PFN_PTRC_GLGETBOOLEANI_VPROC)IntGetProcAddress("glGetBooleani_v");
	_ptrc_glGetBooleani_v(target, index, data);
}

static GLint CODEGEN_FUNCPTR Switch_GetFragDataLocation(GLuint program, const GLchar * name)
{
	_ptrc_glGetFragDataLocation = (PFN_PTRC_GLGETFRAGDATALOCATIONPROC)IntGetProcAddress("glGetFragDataLocation");
	return _ptrc_glGetFragDataLocation(program, name);
}

static void CODEGEN_FUNCPTR Switch_GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint * params)
{
	_ptrc_glGetFramebufferAttachmentParameteriv = (PFN_PTRC_GLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)IntGetProcAddress("glGetFramebufferAttachmentParameteriv");
	_ptrc_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetIntegeri_v(GLenum target, GLuint index, GLint * data)
{
	_ptrc_glGetIntegeri_v = (PFN_PTRC_GLGETINTEGERI_VPROC)IntGetProcAddress("glGetIntegeri_v");
	_ptrc_glGetIntegeri_v(target, index, data);
}

static void CODEGEN_FUNCPTR Switch_GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint * params)
{
	_ptrc_glGetRenderbufferParameteriv = (PFN_PTRC_GLGETRENDERBUFFERPARAMETERIVPROC)IntGetProcAddress("glGetRenderbufferParameteriv");
	_ptrc_glGetRenderbufferParameteriv(target, pname, params);
}

static const GLubyte * CODEGEN_FUNCPTR Switch_GetStringi(GLenum name, GLuint index)
{
	_ptrc_glGetStringi = (PFN_PTRC_GLGETSTRINGIPROC)IntGetProcAddress("glGetStringi");
	return _ptrc_glGetStringi(name, index);
}

static void CODEGEN_FUNCPTR Switch_GetTexParameterIiv(GLenum target, GLenum pname, GLint * params)
{
	_ptrc_glGetTexParameterIiv = (PFN_PTRC_GLGETTEXPARAMETERIIVPROC)IntGetProcAddress("glGetTexParameterIiv");
	_ptrc_glGetTexParameterIiv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetTexParameterIuiv(GLenum target, GLenum pname, GLuint * params)
{
	_ptrc_glGetTexParameterIuiv = (PFN_PTRC_GLGETTEXPARAMETERIUIVPROC)IntGetProcAddress("glGetTexParameterIuiv");
	_ptrc_glGetTexParameterIuiv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name)
{
	_ptrc_glGetTransformFeedbackVarying = (PFN_PTRC_GLGETTRANSFORMFEEDBACKVARYINGPROC)IntGetProcAddress("glGetTransformFeedbackVarying");
	_ptrc_glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

static void CODEGEN_FUNCPTR Switch_GetUniformuiv(GLuint program, GLint location, GLuint * params)
{
	_ptrc_glGetUniformuiv = (PFN_PTRC_GLGETUNIFORMUIVPROC)IntGetProcAddress("glGetUniformuiv");
	_ptrc_glGetUniformuiv(program, location, params);
}

static void CODEGEN_FUNCPTR Switch_GetVertexAttribIiv(GLuint index, GLenum pname, GLint * params)
{
	_ptrc_glGetVertexAttribIiv = (PFN_PTRC_GLGETVERTEXATTRIBIIVPROC)IntGetProcAddress("glGetVertexAttribIiv");
	_ptrc_glGetVertexAttribIiv(index, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetVertexAttribIuiv(GLuint index, GLenum pname, GLuint * params)
{
	_ptrc_glGetVertexAttribIuiv = (PFN_PTRC_GLGETVERTEXATTRIBIUIVPROC)IntGetProcAddress("glGetVertexAttribIuiv");
	_ptrc_glGetVertexAttribIuiv(index, pname, params);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsEnabledi(GLenum target, GLuint index)
{
	_ptrc_glIsEnabledi = (PFN_PTRC_GLISENABLEDIPROC)IntGetProcAddress("glIsEnabledi");
	return _ptrc_glIsEnabledi(target, index);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsFramebuffer(GLuint framebuffer)
{
	_ptrc_glIsFramebuffer = (PFN_PTRC_GLISFRAMEBUFFERPROC)IntGetProcAddress("glIsFramebuffer");
	return _ptrc_glIsFramebuffer(framebuffer);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsRenderbuffer(GLuint renderbuffer)
{
	_ptrc_glIsRenderbuffer = (PFN_PTRC_GLISRENDERBUFFERPROC)IntGetProcAddress("glIsRenderbuffer");
	return _ptrc_glIsRenderbuffer(renderbuffer);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsVertexArray(GLuint ren_array)
{
	_ptrc_glIsVertexArray = (PFN_PTRC_GLISVERTEXARRAYPROC)IntGetProcAddress("glIsVertexArray");
	return _ptrc_glIsVertexArray(ren_array);
}

static void * CODEGEN_FUNCPTR Switch_MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	_ptrc_glMapBufferRange = (PFN_PTRC_GLMAPBUFFERRANGEPROC)IntGetProcAddress("glMapBufferRange");
	return _ptrc_glMapBufferRange(target, offset, length, access);
}

static void CODEGEN_FUNCPTR Switch_RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	_ptrc_glRenderbufferStorage = (PFN_PTRC_GLRENDERBUFFERSTORAGEPROC)IntGetProcAddress("glRenderbufferStorage");
	_ptrc_glRenderbufferStorage(target, internalformat, width, height);
}

static void CODEGEN_FUNCPTR Switch_RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	_ptrc_glRenderbufferStorageMultisample = (PFN_PTRC_GLRENDERBUFFERSTORAGEMULTISAMPLEPROC)IntGetProcAddress("glRenderbufferStorageMultisample");
	_ptrc_glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

static void CODEGEN_FUNCPTR Switch_TexParameterIiv(GLenum target, GLenum pname, const GLint * params)
{
	_ptrc_glTexParameterIiv = (PFN_PTRC_GLTEXPARAMETERIIVPROC)IntGetProcAddress("glTexParameterIiv");
	_ptrc_glTexParameterIiv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_TexParameterIuiv(GLenum target, GLenum pname, const GLuint * params)
{
	_ptrc_glTexParameterIuiv = (PFN_PTRC_GLTEXPARAMETERIUIVPROC)IntGetProcAddress("glTexParameterIuiv");
	_ptrc_glTexParameterIuiv(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_TransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode)
{
	_ptrc_glTransformFeedbackVaryings = (PFN_PTRC_GLTRANSFORMFEEDBACKVARYINGSPROC)IntGetProcAddress("glTransformFeedbackVaryings");
	_ptrc_glTransformFeedbackVaryings(program, count, varyings, bufferMode);
}

static void CODEGEN_FUNCPTR Switch_Uniform1ui(GLint location, GLuint v0)
{
	_ptrc_glUniform1ui = (PFN_PTRC_GLUNIFORM1UIPROC)IntGetProcAddress("glUniform1ui");
	_ptrc_glUniform1ui(location, v0);
}

static void CODEGEN_FUNCPTR Switch_Uniform1uiv(GLint location, GLsizei count, const GLuint * value)
{
	_ptrc_glUniform1uiv = (PFN_PTRC_GLUNIFORM1UIVPROC)IntGetProcAddress("glUniform1uiv");
	_ptrc_glUniform1uiv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform2ui(GLint location, GLuint v0, GLuint v1)
{
	_ptrc_glUniform2ui = (PFN_PTRC_GLUNIFORM2UIPROC)IntGetProcAddress("glUniform2ui");
	_ptrc_glUniform2ui(location, v0, v1);
}

static void CODEGEN_FUNCPTR Switch_Uniform2uiv(GLint location, GLsizei count, const GLuint * value)
{
	_ptrc_glUniform2uiv = (PFN_PTRC_GLUNIFORM2UIVPROC)IntGetProcAddress("glUniform2uiv");
	_ptrc_glUniform2uiv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	_ptrc_glUniform3ui = (PFN_PTRC_GLUNIFORM3UIPROC)IntGetProcAddress("glUniform3ui");
	_ptrc_glUniform3ui(location, v0, v1, v2);
}

static void CODEGEN_FUNCPTR Switch_Uniform3uiv(GLint location, GLsizei count, const GLuint * value)
{
	_ptrc_glUniform3uiv = (PFN_PTRC_GLUNIFORM3UIVPROC)IntGetProcAddress("glUniform3uiv");
	_ptrc_glUniform3uiv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	_ptrc_glUniform4ui = (PFN_PTRC_GLUNIFORM4UIPROC)IntGetProcAddress("glUniform4ui");
	_ptrc_glUniform4ui(location, v0, v1, v2, v3);
}

static void CODEGEN_FUNCPTR Switch_Uniform4uiv(GLint location, GLsizei count, const GLuint * value)
{
	_ptrc_glUniform4uiv = (PFN_PTRC_GLUNIFORM4UIVPROC)IntGetProcAddress("glUniform4uiv");
	_ptrc_glUniform4uiv(location, count, value);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI1i(GLuint index, GLint x)
{
	_ptrc_glVertexAttribI1i = (PFN_PTRC_GLVERTEXATTRIBI1IPROC)IntGetProcAddress("glVertexAttribI1i");
	_ptrc_glVertexAttribI1i(index, x);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI1iv(GLuint index, const GLint * v)
{
	_ptrc_glVertexAttribI1iv = (PFN_PTRC_GLVERTEXATTRIBI1IVPROC)IntGetProcAddress("glVertexAttribI1iv");
	_ptrc_glVertexAttribI1iv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI1ui(GLuint index, GLuint x)
{
	_ptrc_glVertexAttribI1ui = (PFN_PTRC_GLVERTEXATTRIBI1UIPROC)IntGetProcAddress("glVertexAttribI1ui");
	_ptrc_glVertexAttribI1ui(index, x);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI1uiv(GLuint index, const GLuint * v)
{
	_ptrc_glVertexAttribI1uiv = (PFN_PTRC_GLVERTEXATTRIBI1UIVPROC)IntGetProcAddress("glVertexAttribI1uiv");
	_ptrc_glVertexAttribI1uiv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI2i(GLuint index, GLint x, GLint y)
{
	_ptrc_glVertexAttribI2i = (PFN_PTRC_GLVERTEXATTRIBI2IPROC)IntGetProcAddress("glVertexAttribI2i");
	_ptrc_glVertexAttribI2i(index, x, y);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI2iv(GLuint index, const GLint * v)
{
	_ptrc_glVertexAttribI2iv = (PFN_PTRC_GLVERTEXATTRIBI2IVPROC)IntGetProcAddress("glVertexAttribI2iv");
	_ptrc_glVertexAttribI2iv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI2ui(GLuint index, GLuint x, GLuint y)
{
	_ptrc_glVertexAttribI2ui = (PFN_PTRC_GLVERTEXATTRIBI2UIPROC)IntGetProcAddress("glVertexAttribI2ui");
	_ptrc_glVertexAttribI2ui(index, x, y);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI2uiv(GLuint index, const GLuint * v)
{
	_ptrc_glVertexAttribI2uiv = (PFN_PTRC_GLVERTEXATTRIBI2UIVPROC)IntGetProcAddress("glVertexAttribI2uiv");
	_ptrc_glVertexAttribI2uiv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI3i(GLuint index, GLint x, GLint y, GLint z)
{
	_ptrc_glVertexAttribI3i = (PFN_PTRC_GLVERTEXATTRIBI3IPROC)IntGetProcAddress("glVertexAttribI3i");
	_ptrc_glVertexAttribI3i(index, x, y, z);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI3iv(GLuint index, const GLint * v)
{
	_ptrc_glVertexAttribI3iv = (PFN_PTRC_GLVERTEXATTRIBI3IVPROC)IntGetProcAddress("glVertexAttribI3iv");
	_ptrc_glVertexAttribI3iv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z)
{
	_ptrc_glVertexAttribI3ui = (PFN_PTRC_GLVERTEXATTRIBI3UIPROC)IntGetProcAddress("glVertexAttribI3ui");
	_ptrc_glVertexAttribI3ui(index, x, y, z);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI3uiv(GLuint index, const GLuint * v)
{
	_ptrc_glVertexAttribI3uiv = (PFN_PTRC_GLVERTEXATTRIBI3UIVPROC)IntGetProcAddress("glVertexAttribI3uiv");
	_ptrc_glVertexAttribI3uiv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI4bv(GLuint index, const GLbyte * v)
{
	_ptrc_glVertexAttribI4bv = (PFN_PTRC_GLVERTEXATTRIBI4BVPROC)IntGetProcAddress("glVertexAttribI4bv");
	_ptrc_glVertexAttribI4bv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	_ptrc_glVertexAttribI4i = (PFN_PTRC_GLVERTEXATTRIBI4IPROC)IntGetProcAddress("glVertexAttribI4i");
	_ptrc_glVertexAttribI4i(index, x, y, z, w);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI4iv(GLuint index, const GLint * v)
{
	_ptrc_glVertexAttribI4iv = (PFN_PTRC_GLVERTEXATTRIBI4IVPROC)IntGetProcAddress("glVertexAttribI4iv");
	_ptrc_glVertexAttribI4iv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI4sv(GLuint index, const GLshort * v)
{
	_ptrc_glVertexAttribI4sv = (PFN_PTRC_GLVERTEXATTRIBI4SVPROC)IntGetProcAddress("glVertexAttribI4sv");
	_ptrc_glVertexAttribI4sv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI4ubv(GLuint index, const GLubyte * v)
{
	_ptrc_glVertexAttribI4ubv = (PFN_PTRC_GLVERTEXATTRIBI4UBVPROC)IntGetProcAddress("glVertexAttribI4ubv");
	_ptrc_glVertexAttribI4ubv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	_ptrc_glVertexAttribI4ui = (PFN_PTRC_GLVERTEXATTRIBI4UIPROC)IntGetProcAddress("glVertexAttribI4ui");
	_ptrc_glVertexAttribI4ui(index, x, y, z, w);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI4uiv(GLuint index, const GLuint * v)
{
	_ptrc_glVertexAttribI4uiv = (PFN_PTRC_GLVERTEXATTRIBI4UIVPROC)IntGetProcAddress("glVertexAttribI4uiv");
	_ptrc_glVertexAttribI4uiv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribI4usv(GLuint index, const GLushort * v)
{
	_ptrc_glVertexAttribI4usv = (PFN_PTRC_GLVERTEXATTRIBI4USVPROC)IntGetProcAddress("glVertexAttribI4usv");
	_ptrc_glVertexAttribI4usv(index, v);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
	_ptrc_glVertexAttribIPointer = (PFN_PTRC_GLVERTEXATTRIBIPOINTERPROC)IntGetProcAddress("glVertexAttribIPointer");
	_ptrc_glVertexAttribIPointer(index, size, type, stride, pointer);
}


// Extension: 3.1
static void CODEGEN_FUNCPTR Switch_CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	_ptrc_glCopyBufferSubData = (PFN_PTRC_GLCOPYBUFFERSUBDATAPROC)IntGetProcAddress("glCopyBufferSubData");
	_ptrc_glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

static void CODEGEN_FUNCPTR Switch_DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
	_ptrc_glDrawArraysInstanced = (PFN_PTRC_GLDRAWARRAYSINSTANCEDPROC)IntGetProcAddress("glDrawArraysInstanced");
	_ptrc_glDrawArraysInstanced(mode, first, count, instancecount);
}

static void CODEGEN_FUNCPTR Switch_DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei instancecount)
{
	_ptrc_glDrawElementsInstanced = (PFN_PTRC_GLDRAWELEMENTSINSTANCEDPROC)IntGetProcAddress("glDrawElementsInstanced");
	_ptrc_glDrawElementsInstanced(mode, count, type, indices, instancecount);
}

static void CODEGEN_FUNCPTR Switch_GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName)
{
	_ptrc_glGetActiveUniformBlockName = (PFN_PTRC_GLGETACTIVEUNIFORMBLOCKNAMEPROC)IntGetProcAddress("glGetActiveUniformBlockName");
	_ptrc_glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

static void CODEGEN_FUNCPTR Switch_GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params)
{
	_ptrc_glGetActiveUniformBlockiv = (PFN_PTRC_GLGETACTIVEUNIFORMBLOCKIVPROC)IntGetProcAddress("glGetActiveUniformBlockiv");
	_ptrc_glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName)
{
	_ptrc_glGetActiveUniformName = (PFN_PTRC_GLGETACTIVEUNIFORMNAMEPROC)IntGetProcAddress("glGetActiveUniformName");
	_ptrc_glGetActiveUniformName(program, uniformIndex, bufSize, length, uniformName);
}

static void CODEGEN_FUNCPTR Switch_GetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params)
{
	_ptrc_glGetActiveUniformsiv = (PFN_PTRC_GLGETACTIVEUNIFORMSIVPROC)IntGetProcAddress("glGetActiveUniformsiv");
	_ptrc_glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

static GLuint CODEGEN_FUNCPTR Switch_GetUniformBlockIndex(GLuint program, const GLchar * uniformBlockName)
{
	_ptrc_glGetUniformBlockIndex = (PFN_PTRC_GLGETUNIFORMBLOCKINDEXPROC)IntGetProcAddress("glGetUniformBlockIndex");
	return _ptrc_glGetUniformBlockIndex(program, uniformBlockName);
}

static void CODEGEN_FUNCPTR Switch_GetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint * uniformIndices)
{
	_ptrc_glGetUniformIndices = (PFN_PTRC_GLGETUNIFORMINDICESPROC)IntGetProcAddress("glGetUniformIndices");
	_ptrc_glGetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
}

static void CODEGEN_FUNCPTR Switch_PrimitiveRestartIndex(GLuint index)
{
	_ptrc_glPrimitiveRestartIndex = (PFN_PTRC_GLPRIMITIVERESTARTINDEXPROC)IntGetProcAddress("glPrimitiveRestartIndex");
	_ptrc_glPrimitiveRestartIndex(index);
}

static void CODEGEN_FUNCPTR Switch_TexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
	_ptrc_glTexBuffer = (PFN_PTRC_GLTEXBUFFERPROC)IntGetProcAddress("glTexBuffer");
	_ptrc_glTexBuffer(target, internalformat, buffer);
}

static void CODEGEN_FUNCPTR Switch_UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
	_ptrc_glUniformBlockBinding = (PFN_PTRC_GLUNIFORMBLOCKBINDINGPROC)IntGetProcAddress("glUniformBlockBinding");
	_ptrc_glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}


// Extension: 3.2
static GLenum CODEGEN_FUNCPTR Switch_ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	_ptrc_glClientWaitSync = (PFN_PTRC_GLCLIENTWAITSYNCPROC)IntGetProcAddress("glClientWaitSync");
	return _ptrc_glClientWaitSync(sync, flags, timeout);
}

static void CODEGEN_FUNCPTR Switch_DeleteSync(GLsync sync)
{
	_ptrc_glDeleteSync = (PFN_PTRC_GLDELETESYNCPROC)IntGetProcAddress("glDeleteSync");
	_ptrc_glDeleteSync(sync);
}

static void CODEGEN_FUNCPTR Switch_DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex)
{
	_ptrc_glDrawElementsBaseVertex = (PFN_PTRC_GLDRAWELEMENTSBASEVERTEXPROC)IntGetProcAddress("glDrawElementsBaseVertex");
	_ptrc_glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
}

static void CODEGEN_FUNCPTR Switch_DrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei instancecount, GLint basevertex)
{
	_ptrc_glDrawElementsInstancedBaseVertex = (PFN_PTRC_GLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)IntGetProcAddress("glDrawElementsInstancedBaseVertex");
	_ptrc_glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
}

static void CODEGEN_FUNCPTR Switch_DrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex)
{
	_ptrc_glDrawRangeElementsBaseVertex = (PFN_PTRC_GLDRAWRANGEELEMENTSBASEVERTEXPROC)IntGetProcAddress("glDrawRangeElementsBaseVertex");
	_ptrc_glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
}

static GLsync CODEGEN_FUNCPTR Switch_FenceSync(GLenum condition, GLbitfield flags)
{
	_ptrc_glFenceSync = (PFN_PTRC_GLFENCESYNCPROC)IntGetProcAddress("glFenceSync");
	return _ptrc_glFenceSync(condition, flags);
}

static void CODEGEN_FUNCPTR Switch_FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	_ptrc_glFramebufferTexture = (PFN_PTRC_GLFRAMEBUFFERTEXTUREPROC)IntGetProcAddress("glFramebufferTexture");
	_ptrc_glFramebufferTexture(target, attachment, texture, level);
}

static void CODEGEN_FUNCPTR Switch_GetBufferParameteri64v(GLenum target, GLenum pname, GLint64 * params)
{
	_ptrc_glGetBufferParameteri64v = (PFN_PTRC_GLGETBUFFERPARAMETERI64VPROC)IntGetProcAddress("glGetBufferParameteri64v");
	_ptrc_glGetBufferParameteri64v(target, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetInteger64i_v(GLenum target, GLuint index, GLint64 * data)
{
	_ptrc_glGetInteger64i_v = (PFN_PTRC_GLGETINTEGER64I_VPROC)IntGetProcAddress("glGetInteger64i_v");
	_ptrc_glGetInteger64i_v(target, index, data);
}

static void CODEGEN_FUNCPTR Switch_GetInteger64v(GLenum pname, GLint64 * params)
{
	_ptrc_glGetInteger64v = (PFN_PTRC_GLGETINTEGER64VPROC)IntGetProcAddress("glGetInteger64v");
	_ptrc_glGetInteger64v(pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetMultisamplefv(GLenum pname, GLuint index, GLfloat * val)
{
	_ptrc_glGetMultisamplefv = (PFN_PTRC_GLGETMULTISAMPLEFVPROC)IntGetProcAddress("glGetMultisamplefv");
	_ptrc_glGetMultisamplefv(pname, index, val);
}

static void CODEGEN_FUNCPTR Switch_GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values)
{
	_ptrc_glGetSynciv = (PFN_PTRC_GLGETSYNCIVPROC)IntGetProcAddress("glGetSynciv");
	_ptrc_glGetSynciv(sync, pname, bufSize, length, values);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsSync(GLsync sync)
{
	_ptrc_glIsSync = (PFN_PTRC_GLISSYNCPROC)IntGetProcAddress("glIsSync");
	return _ptrc_glIsSync(sync);
}

static void CODEGEN_FUNCPTR Switch_MultiDrawElementsBaseVertex(GLenum mode, const GLsizei * count, GLenum type, const GLvoid *const* indices, GLsizei drawcount, const GLint * basevertex)
{
	_ptrc_glMultiDrawElementsBaseVertex = (PFN_PTRC_GLMULTIDRAWELEMENTSBASEVERTEXPROC)IntGetProcAddress("glMultiDrawElementsBaseVertex");
	_ptrc_glMultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
}

static void CODEGEN_FUNCPTR Switch_ProvokingVertex(GLenum mode)
{
	_ptrc_glProvokingVertex = (PFN_PTRC_GLPROVOKINGVERTEXPROC)IntGetProcAddress("glProvokingVertex");
	_ptrc_glProvokingVertex(mode);
}

static void CODEGEN_FUNCPTR Switch_SampleMaski(GLuint index, GLbitfield mask)
{
	_ptrc_glSampleMaski = (PFN_PTRC_GLSAMPLEMASKIPROC)IntGetProcAddress("glSampleMaski");
	_ptrc_glSampleMaski(index, mask);
}

static void CODEGEN_FUNCPTR Switch_TexImage2DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	_ptrc_glTexImage2DMultisample = (PFN_PTRC_GLTEXIMAGE2DMULTISAMPLEPROC)IntGetProcAddress("glTexImage2DMultisample");
	_ptrc_glTexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

static void CODEGEN_FUNCPTR Switch_TexImage3DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	_ptrc_glTexImage3DMultisample = (PFN_PTRC_GLTEXIMAGE3DMULTISAMPLEPROC)IntGetProcAddress("glTexImage3DMultisample");
	_ptrc_glTexImage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

static void CODEGEN_FUNCPTR Switch_WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	_ptrc_glWaitSync = (PFN_PTRC_GLWAITSYNCPROC)IntGetProcAddress("glWaitSync");
	_ptrc_glWaitSync(sync, flags, timeout);
}


// Extension: 3.3
static void CODEGEN_FUNCPTR Switch_BindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar * name)
{
	_ptrc_glBindFragDataLocationIndexed = (PFN_PTRC_GLBINDFRAGDATALOCATIONINDEXEDPROC)IntGetProcAddress("glBindFragDataLocationIndexed");
	_ptrc_glBindFragDataLocationIndexed(program, colorNumber, index, name);
}

static void CODEGEN_FUNCPTR Switch_BindSampler(GLuint unit, GLuint sampler)
{
	_ptrc_glBindSampler = (PFN_PTRC_GLBINDSAMPLERPROC)IntGetProcAddress("glBindSampler");
	_ptrc_glBindSampler(unit, sampler);
}

static void CODEGEN_FUNCPTR Switch_DeleteSamplers(GLsizei count, const GLuint * samplers)
{
	_ptrc_glDeleteSamplers = (PFN_PTRC_GLDELETESAMPLERSPROC)IntGetProcAddress("glDeleteSamplers");
	_ptrc_glDeleteSamplers(count, samplers);
}

static void CODEGEN_FUNCPTR Switch_GenSamplers(GLsizei count, GLuint * samplers)
{
	_ptrc_glGenSamplers = (PFN_PTRC_GLGENSAMPLERSPROC)IntGetProcAddress("glGenSamplers");
	_ptrc_glGenSamplers(count, samplers);
}

static GLint CODEGEN_FUNCPTR Switch_GetFragDataIndex(GLuint program, const GLchar * name)
{
	_ptrc_glGetFragDataIndex = (PFN_PTRC_GLGETFRAGDATAINDEXPROC)IntGetProcAddress("glGetFragDataIndex");
	return _ptrc_glGetFragDataIndex(program, name);
}

static void CODEGEN_FUNCPTR Switch_GetQueryObjecti64v(GLuint id, GLenum pname, GLint64 * params)
{
	_ptrc_glGetQueryObjecti64v = (PFN_PTRC_GLGETQUERYOBJECTI64VPROC)IntGetProcAddress("glGetQueryObjecti64v");
	_ptrc_glGetQueryObjecti64v(id, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 * params)
{
	_ptrc_glGetQueryObjectui64v = (PFN_PTRC_GLGETQUERYOBJECTUI64VPROC)IntGetProcAddress("glGetQueryObjectui64v");
	_ptrc_glGetQueryObjectui64v(id, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint * params)
{
	_ptrc_glGetSamplerParameterIiv = (PFN_PTRC_GLGETSAMPLERPARAMETERIIVPROC)IntGetProcAddress("glGetSamplerParameterIiv");
	_ptrc_glGetSamplerParameterIiv(sampler, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint * params)
{
	_ptrc_glGetSamplerParameterIuiv = (PFN_PTRC_GLGETSAMPLERPARAMETERIUIVPROC)IntGetProcAddress("glGetSamplerParameterIuiv");
	_ptrc_glGetSamplerParameterIuiv(sampler, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat * params)
{
	_ptrc_glGetSamplerParameterfv = (PFN_PTRC_GLGETSAMPLERPARAMETERFVPROC)IntGetProcAddress("glGetSamplerParameterfv");
	_ptrc_glGetSamplerParameterfv(sampler, pname, params);
}

static void CODEGEN_FUNCPTR Switch_GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint * params)
{
	_ptrc_glGetSamplerParameteriv = (PFN_PTRC_GLGETSAMPLERPARAMETERIVPROC)IntGetProcAddress("glGetSamplerParameteriv");
	_ptrc_glGetSamplerParameteriv(sampler, pname, params);
}

static GLboolean CODEGEN_FUNCPTR Switch_IsSampler(GLuint sampler)
{
	_ptrc_glIsSampler = (PFN_PTRC_GLISSAMPLERPROC)IntGetProcAddress("glIsSampler");
	return _ptrc_glIsSampler(sampler);
}

static void CODEGEN_FUNCPTR Switch_QueryCounter(GLuint id, GLenum target)
{
	_ptrc_glQueryCounter = (PFN_PTRC_GLQUERYCOUNTERPROC)IntGetProcAddress("glQueryCounter");
	_ptrc_glQueryCounter(id, target);
}

static void CODEGEN_FUNCPTR Switch_SamplerParameterIiv(GLuint sampler, GLenum pname, const GLint * param)
{
	_ptrc_glSamplerParameterIiv = (PFN_PTRC_GLSAMPLERPARAMETERIIVPROC)IntGetProcAddress("glSamplerParameterIiv");
	_ptrc_glSamplerParameterIiv(sampler, pname, param);
}

static void CODEGEN_FUNCPTR Switch_SamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint * param)
{
	_ptrc_glSamplerParameterIuiv = (PFN_PTRC_GLSAMPLERPARAMETERIUIVPROC)IntGetProcAddress("glSamplerParameterIuiv");
	_ptrc_glSamplerParameterIuiv(sampler, pname, param);
}

static void CODEGEN_FUNCPTR Switch_SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
	_ptrc_glSamplerParameterf = (PFN_PTRC_GLSAMPLERPARAMETERFPROC)IntGetProcAddress("glSamplerParameterf");
	_ptrc_glSamplerParameterf(sampler, pname, param);
}

static void CODEGEN_FUNCPTR Switch_SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat * param)
{
	_ptrc_glSamplerParameterfv = (PFN_PTRC_GLSAMPLERPARAMETERFVPROC)IntGetProcAddress("glSamplerParameterfv");
	_ptrc_glSamplerParameterfv(sampler, pname, param);
}

static void CODEGEN_FUNCPTR Switch_SamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	_ptrc_glSamplerParameteri = (PFN_PTRC_GLSAMPLERPARAMETERIPROC)IntGetProcAddress("glSamplerParameteri");
	_ptrc_glSamplerParameteri(sampler, pname, param);
}

static void CODEGEN_FUNCPTR Switch_SamplerParameteriv(GLuint sampler, GLenum pname, const GLint * param)
{
	_ptrc_glSamplerParameteriv = (PFN_PTRC_GLSAMPLERPARAMETERIVPROC)IntGetProcAddress("glSamplerParameteriv");
	_ptrc_glSamplerParameteriv(sampler, pname, param);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribDivisor(GLuint index, GLuint divisor)
{
	_ptrc_glVertexAttribDivisor = (PFN_PTRC_GLVERTEXATTRIBDIVISORPROC)IntGetProcAddress("glVertexAttribDivisor");
	_ptrc_glVertexAttribDivisor(index, divisor);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	_ptrc_glVertexAttribP1ui = (PFN_PTRC_GLVERTEXATTRIBP1UIPROC)IntGetProcAddress("glVertexAttribP1ui");
	_ptrc_glVertexAttribP1ui(index, type, normalized, value);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value)
{
	_ptrc_glVertexAttribP1uiv = (PFN_PTRC_GLVERTEXATTRIBP1UIVPROC)IntGetProcAddress("glVertexAttribP1uiv");
	_ptrc_glVertexAttribP1uiv(index, type, normalized, value);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	_ptrc_glVertexAttribP2ui = (PFN_PTRC_GLVERTEXATTRIBP2UIPROC)IntGetProcAddress("glVertexAttribP2ui");
	_ptrc_glVertexAttribP2ui(index, type, normalized, value);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value)
{
	_ptrc_glVertexAttribP2uiv = (PFN_PTRC_GLVERTEXATTRIBP2UIVPROC)IntGetProcAddress("glVertexAttribP2uiv");
	_ptrc_glVertexAttribP2uiv(index, type, normalized, value);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	_ptrc_glVertexAttribP3ui = (PFN_PTRC_GLVERTEXATTRIBP3UIPROC)IntGetProcAddress("glVertexAttribP3ui");
	_ptrc_glVertexAttribP3ui(index, type, normalized, value);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value)
{
	_ptrc_glVertexAttribP3uiv = (PFN_PTRC_GLVERTEXATTRIBP3UIVPROC)IntGetProcAddress("glVertexAttribP3uiv");
	_ptrc_glVertexAttribP3uiv(index, type, normalized, value);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	_ptrc_glVertexAttribP4ui = (PFN_PTRC_GLVERTEXATTRIBP4UIPROC)IntGetProcAddress("glVertexAttribP4ui");
	_ptrc_glVertexAttribP4ui(index, type, normalized, value);
}

static void CODEGEN_FUNCPTR Switch_VertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value)
{
	_ptrc_glVertexAttribP4uiv = (PFN_PTRC_GLVERTEXATTRIBP4UIVPROC)IntGetProcAddress("glVertexAttribP4uiv");
	_ptrc_glVertexAttribP4uiv(index, type, normalized, value);
}



static void ClearExtensionVariables()
{
	ogl_ext_EXT_texture_compression_s3tc = 0;
	ogl_ext_EXT_texture_sRGB = 0;
	ogl_ext_EXT_texture_filter_anisotropic = 0;
	ogl_ext_ARB_compressed_texture_pixel_storage = 0;
	ogl_ext_ARB_conservative_depth = 0;
	ogl_ext_ARB_ES2_compatibility = 0;
	ogl_ext_ARB_get_program_binary = 0;
	ogl_ext_ARB_explicit_uniform_location = 0;
	ogl_ext_ARB_internalformat_query = 0;
	ogl_ext_ARB_internalformat_query2 = 0;
	ogl_ext_ARB_map_buffer_alignment = 0;
	ogl_ext_ARB_program_interface_query = 0;
	ogl_ext_ARB_separate_shader_objects = 0;
	ogl_ext_ARB_shading_language_420pack = 0;
	ogl_ext_ARB_shading_language_packing = 0;
	ogl_ext_ARB_texture_buffer_range = 0;
	ogl_ext_ARB_texture_storage = 0;
	ogl_ext_ARB_texture_view = 0;
	ogl_ext_ARB_vertex_attrib_binding = 0;
	ogl_ext_ARB_viewport_array = 0;
	ogl_ext_ARB_arrays_of_arrays = 0;
	ogl_ext_ARB_clear_buffer_object = 0;
	ogl_ext_ARB_copy_image = 0;
	ogl_ext_ARB_ES3_compatibility = 0;
	ogl_ext_ARB_fragment_layer_viewport = 0;
	ogl_ext_ARB_framebuffer_no_attachments = 0;
	ogl_ext_ARB_invalidate_subdata = 0;
	ogl_ext_ARB_robust_buffer_access_behavior = 0;
	ogl_ext_ARB_stencil_texturing = 0;
	ogl_ext_ARB_texture_query_levels = 0;
	ogl_ext_ARB_texture_storage_multisample = 0;
	ogl_ext_KHR_debug = 0;
}

typedef struct ogl_MapTable_s
{
	char *extName;
	int *extVariable;
}ogl_MapTable;

static ogl_MapTable g_mappingTable[32] = 
{
	{"GL_EXT_texture_compression_s3tc", &ogl_ext_EXT_texture_compression_s3tc},
	{"GL_EXT_texture_sRGB", &ogl_ext_EXT_texture_sRGB},
	{"GL_EXT_texture_filter_anisotropic", &ogl_ext_EXT_texture_filter_anisotropic},
	{"GL_ARB_compressed_texture_pixel_storage", &ogl_ext_ARB_compressed_texture_pixel_storage},
	{"GL_ARB_conservative_depth", &ogl_ext_ARB_conservative_depth},
	{"GL_ARB_ES2_compatibility", &ogl_ext_ARB_ES2_compatibility},
	{"GL_ARB_get_program_binary", &ogl_ext_ARB_get_program_binary},
	{"GL_ARB_explicit_uniform_location", &ogl_ext_ARB_explicit_uniform_location},
	{"GL_ARB_internalformat_query", &ogl_ext_ARB_internalformat_query},
	{"GL_ARB_internalformat_query2", &ogl_ext_ARB_internalformat_query2},
	{"GL_ARB_map_buffer_alignment", &ogl_ext_ARB_map_buffer_alignment},
	{"GL_ARB_program_interface_query", &ogl_ext_ARB_program_interface_query},
	{"GL_ARB_separate_shader_objects", &ogl_ext_ARB_separate_shader_objects},
	{"GL_ARB_shading_language_420pack", &ogl_ext_ARB_shading_language_420pack},
	{"GL_ARB_shading_language_packing", &ogl_ext_ARB_shading_language_packing},
	{"GL_ARB_texture_buffer_range", &ogl_ext_ARB_texture_buffer_range},
	{"GL_ARB_texture_storage", &ogl_ext_ARB_texture_storage},
	{"GL_ARB_texture_view", &ogl_ext_ARB_texture_view},
	{"GL_ARB_vertex_attrib_binding", &ogl_ext_ARB_vertex_attrib_binding},
	{"GL_ARB_viewport_array", &ogl_ext_ARB_viewport_array},
	{"GL_ARB_arrays_of_arrays", &ogl_ext_ARB_arrays_of_arrays},
	{"GL_ARB_clear_buffer_object", &ogl_ext_ARB_clear_buffer_object},
	{"GL_ARB_copy_image", &ogl_ext_ARB_copy_image},
	{"GL_ARB_ES3_compatibility", &ogl_ext_ARB_ES3_compatibility},
	{"GL_ARB_fragment_layer_viewport", &ogl_ext_ARB_fragment_layer_viewport},
	{"GL_ARB_framebuffer_no_attachments", &ogl_ext_ARB_framebuffer_no_attachments},
	{"GL_ARB_invalidate_subdata", &ogl_ext_ARB_invalidate_subdata},
	{"GL_ARB_robust_buffer_access_behavior", &ogl_ext_ARB_robust_buffer_access_behavior},
	{"GL_ARB_stencil_texturing", &ogl_ext_ARB_stencil_texturing},
	{"GL_ARB_texture_query_levels", &ogl_ext_ARB_texture_query_levels},
	{"GL_ARB_texture_storage_multisample", &ogl_ext_ARB_texture_storage_multisample},
	{"GL_KHR_debug", &ogl_ext_KHR_debug},
};

static void LoadExtByName(const char *extensionName)
{
	ogl_MapTable *tableEnd = &g_mappingTable[32];
	ogl_MapTable *entry = &g_mappingTable[0];
	for(; entry != tableEnd; ++entry)
	{
		if(strcmp(entry->extName, extensionName) == 0)
			break;
	}
	
	if(entry != tableEnd)
		*(entry->extVariable) = 1;
}

void ProcExtsFromExtList()
{
	GLint iLoop;
	GLint iNumExtensions = 0;
	_ptrc_glGetIntegerv(GL_NUM_EXTENSIONS, &iNumExtensions);

	for(iLoop = 0; iLoop < iNumExtensions; iLoop++)
	{
		const char *strExtensionName = (const char *)_ptrc_glGetStringi(GL_EXTENSIONS, iLoop);
		LoadExtByName(strExtensionName);
	}
}

void ogl_CheckExtensions()
{
	ClearExtensionVariables();
	
	ProcExtsFromExtList();
}

