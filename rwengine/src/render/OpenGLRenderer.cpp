#include <render/OpenGLRenderer.hpp>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <sstream>
#include <iostream>

GLuint compileShader(GLenum type, const char *source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if( status != GL_TRUE ) {
		std::cerr << "[OGL] Shader Compilation Failed" << std::endl;
	}

	GLint len;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if( len > 1 ) {
		GLchar *buffer = new GLchar[len];
		glGetShaderInfoLog(shader, len, NULL, buffer);

		GLint sourceLen;
		glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &sourceLen);
		GLchar *sourceBuff = new GLchar[sourceLen];
		glGetShaderSource(shader, sourceLen, nullptr, sourceBuff);

		std::cerr << "[OGL] Shader InfoLog(" << shader << "):\n" << buffer << "\nSource:\n" << sourceBuff << std::endl;

		delete[] buffer;
		delete[] sourceBuff;
	}

	if (status != GL_TRUE) {
		exit(1);
	}

	return shader;
}

GLuint compileProgram(const char* vertex, const char* fragment)
{
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertex);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragment);
	GLuint prog	= glCreateProgram();
	glAttachShader(prog, vertexShader);
	glAttachShader(prog, fragmentShader);
	glLinkProgram(prog);

	GLint status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);

	if( status != GL_TRUE ) {
		std::cerr << "[OGL] Program Link Failed" << std::endl;
	}

	GLint len;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if( len > 1 ) {
		GLchar *buffer = new GLchar[len];
		glGetProgramInfoLog(prog, len, NULL, buffer);

		std::cerr << "[OGL] Program InfoLog(" << prog << "):\n" << buffer << std::endl;

		delete[] buffer;
	}

	if (status != GL_TRUE) {
		exit(1);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return prog;
}



void OpenGLRenderer::useDrawBuffer(DrawBuffer* dbuff)
{
	if( dbuff != currentDbuff )
	{
		glBindVertexArray(dbuff->getVAOName());
		currentDbuff = dbuff;
	}
}

void OpenGLRenderer::useTexture(GLuint tex)
{
	if( tex != currentTexture )
	{
		glBindTexture(GL_TEXTURE_2D, tex);
		currentTexture = tex;
	}
}

void OpenGLRenderer::useProgram(Renderer::ShaderProgram* p)
{
	if( p != currentProgram )
	{
		currentProgram = static_cast<OpenGLShaderProgram*>(p);
		glUseProgram( currentProgram->getName() );
	}
}

OpenGLRenderer::OpenGLRenderer()
	: currentDbuff(nullptr), currentTexture(0), currentProgram(nullptr)
{
	glGenBuffers(1, &UBOScene);
	glGenBuffers(1, &UBOObject);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBOScene);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, UBOObject);
}

std::string OpenGLRenderer::getIDString() const
{
	std::stringstream ss;
	ss << "OpenGL Renderer";
	ss << " Version: " << glGetString(GL_VERSION);
	ss << " (GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << ")";
	ss << " Vendor: " << glGetString(GL_VENDOR);
	return ss.str();
}

Renderer::ShaderProgram* OpenGLRenderer::createShader(const std::string& vert, const std::string& frag)
{
	return new OpenGLShaderProgram(
				compileProgram(vert.c_str(), frag.c_str()));
}

void OpenGLRenderer::setProgramBlockBinding(Renderer::ShaderProgram* p, const std::string& name, GLint point)
{
	OpenGLShaderProgram* glsh = static_cast<OpenGLShaderProgram*>(p);

	auto ubi = glGetUniformBlockIndex(glsh->getName(), name.c_str());
	glUniformBlockBinding(glsh->getName(), ubi, point);
}

void OpenGLRenderer::setUniformTexture(Renderer::ShaderProgram* p, const std::string& name, GLint tex)
{
	useProgram(p);

	glUniform1i(currentProgram->getUniformLocation(name), tex);
}

void OpenGLRenderer::setUniform(Renderer::ShaderProgram* p, const std::string& name, const glm::vec4& m)
{
	useProgram(p);

	glUniform4fv(currentProgram->getUniformLocation(name.c_str()), 1, glm::value_ptr(m));
}

void OpenGLRenderer::setUniform(Renderer::ShaderProgram* p, const std::string& name, const glm::vec3& m)
{
	useProgram(p);

	glUniform3fv(currentProgram->getUniformLocation(name.c_str()), 1, glm::value_ptr(m));
}

void OpenGLRenderer::setUniform(Renderer::ShaderProgram* p, const std::string& name, const glm::vec2& m)
{
	useProgram(p);

	glUniform2fv(currentProgram->getUniformLocation(name.c_str()), 1, glm::value_ptr(m));
}

void OpenGLRenderer::setUniform(Renderer::ShaderProgram* p, const std::string& name, float f)
{
	glUniform1fv(currentProgram->getUniformLocation(name.c_str()), 1, &f);
}

void OpenGLRenderer::clear(const glm::vec4& colour, bool clearColour, bool clearDepth)
{
	auto flags = 0;
	if( clearColour ) {
		flags |= GL_COLOR_BUFFER_BIT;
		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}
	if( clearDepth ) {
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	glClear(flags);
}

void OpenGLRenderer::setSceneParameters(const Renderer::SceneUniformData& data)
{
	uploadUBO(UBOScene, data);
}

void OpenGLRenderer::draw(const glm::mat4& model, DrawBuffer* draw, const Renderer::DrawParameters& p)
{
	useDrawBuffer(draw);
	useTexture(p.texture);

	ObjectUniformData oudata {
		model,
		glm::vec4(p.colour.r/255.f, p.colour.g/255.f, p.colour.b/255.f, 1.f),
		1.f,
		1.f,
		p.colour.a/255.f
	};
	uploadUBO(UBOObject, oudata);

	glDrawElements(draw->getFaceType(), p.count, GL_UNSIGNED_INT,
				   (void*) (sizeof(RenderIndex) * p.start));
}

void OpenGLRenderer::drawArrays(const glm::mat4& model, DrawBuffer* draw, const Renderer::DrawParameters& p)
{

	useDrawBuffer(draw);
	useTexture(p.texture);

	ObjectUniformData oudata {
		model,
		glm::vec4(p.colour.r/255.f, p.colour.g/255.f, p.colour.b/255.f, 1.f),
		1.f,
		1.f,
		p.colour.a/255.f
	};
	uploadUBO(UBOObject, oudata);

	glDrawArrays(draw->getFaceType(), p.start, p.count);
}
