#pragma once
#ifndef _OPENGLRENDERER_HPP_
#define _OPENGLRENDERER_HPP_

#include <engine/RWTypes.hpp>
#include <render/DrawBuffer.hpp>
#include <render/GeometryBuffer.hpp>

typedef std::uint32_t RenderIndex;

struct VertexP3
{
	glm::vec3 position;

	static const AttributeList vertex_attributes() {
		return {
			{ATRS_Position, 3, sizeof(VertexP3),  0ul},
		};
	}
};

/// @todo normalize this to have the same interface as VertexP3
struct VertexP2 {
	static const AttributeList vertex_attributes() {
		return {
			{ATRS_Position, 2, sizeof(VertexP2),  0ul}
		};
	}

	float x, y;
};

class Renderer
{
public:

	struct DrawParameters
	{
		GLuint texture; /// @todo replace with texture handle.
		glm::u8vec4 colour;
		float ambient;
		float diffuse;

		size_t count; /// The number of indicies to draw
		unsigned int start; /// Start index.
	};

	struct ObjectUniformData {
		glm::mat4 model;
		glm::vec4 colour;
		float diffuse;
		float ambient;
		float visibility;
	};

	struct SceneUniformData {
		glm::mat4 projection;
		glm::mat4 view;
		glm::vec4 ambient;
		glm::vec4 dynamic;
		glm::vec4 fogColour;
		glm::vec4 campos;
		float fogStart;
		float fogEnd;
	};

	class ShaderProgram {
		// This just provides an opaque handle for external users.
	};

	virtual std::string getIDString() const = 0;

	virtual ShaderProgram* createShader(const std::string& vert, const std::string& frag) = 0;

	virtual void useProgram(ShaderProgram* p) = 0;

	/// @todo dont use GLint in the interface.
	virtual void setProgramBlockBinding(ShaderProgram* p, const std::string& name, GLint point) = 0;
	virtual void setUniformTexture(ShaderProgram*p, const std::string& name, GLint tex) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, const glm::mat4& m) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, const glm::vec4& v) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, const glm::vec3& v) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, const glm::vec2& v) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, float f) = 0;

	virtual void clear(const glm::vec4& colour, bool clearColour = true, bool clearDepth = true) = 0;

	virtual void setSceneParameters(const SceneUniformData& data) = 0;

	virtual void draw(const glm::mat4& model, DrawBuffer* draw, const DrawParameters& p) = 0;
	virtual void drawArrays(const glm::mat4& model, DrawBuffer* draw, const DrawParameters& p) = 0;

	void setViewport(const glm::ivec2& vp) { viewport = vp; }
	const glm::ivec2& getViewport() const { return viewport; }

	glm::mat4 get2DProjection() const;

	virtual void invalidate() = 0;

private:
	glm::ivec2 viewport;
};

class OpenGLRenderer : public Renderer
{
public:

	class OpenGLShaderProgram : public ShaderProgram {
		GLuint program;
		std::map<std::string, GLint> uniforms;
	public:
		OpenGLShaderProgram(GLuint p)
			: program(p)
		{ }

		GLuint getName() const { return program; }

		GLint getUniformLocation(const std::string& name) {
			auto c = uniforms.find(name.c_str());
			GLint loc = -1;
			if( c == uniforms.end() ) {
				loc = glGetUniformLocation(program, name.c_str());
				uniforms[name] = loc;
			}
			else {
				loc = c->second;
			}
			return loc;
		}
	};

	OpenGLRenderer();

	std::string getIDString() const;

	ShaderProgram* createShader(const std::string &vert, const std::string &frag);
	void setProgramBlockBinding(ShaderProgram* p, const std::string &name, GLint point);
	void setUniformTexture(ShaderProgram* p, const std::string &name, GLint tex);
	void setUniform(ShaderProgram* p, const std::string& name, const glm::mat4& m);
	void setUniform(ShaderProgram* p, const std::string& name, const glm::vec4& m);
	void setUniform(ShaderProgram* p, const std::string& name, const glm::vec3& m);
	void setUniform(ShaderProgram* p, const std::string& name, const glm::vec2& m);
	void setUniform(ShaderProgram* p, const std::string& name, float f);
	void useProgram(ShaderProgram* p);

	void clear(const glm::vec4 &colour, bool clearColour, bool clearDepth);

	void setSceneParameters(const SceneUniformData &data);

	void draw(const glm::mat4& model, DrawBuffer* draw, const DrawParameters& p);
	void drawArrays(const glm::mat4& model, DrawBuffer* draw, const DrawParameters& p);

	void invalidate();

private:
	DrawBuffer* currentDbuff;

	void useDrawBuffer(DrawBuffer* dbuff);

	GLuint currentTexture;
	void useTexture(GLuint tex);

	OpenGLShaderProgram* currentProgram;

	GLuint currentUBO;
	template<class T> void uploadUBO(GLuint buffer, const T& data)
	{
		if( currentUBO != buffer ) {
			glBindBuffer(GL_UNIFORM_BUFFER, buffer);
			currentUBO = buffer;
		}
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T), &data, GL_DYNAMIC_DRAW);
	}

	GLuint UBOObject;
	GLuint UBOScene;
};

/// @todo remove these from here
GLuint compileShader(GLenum type, const char *source);
GLuint compileProgram(const char* vertex, const char* fragment);

#endif
