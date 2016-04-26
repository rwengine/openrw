#include <render/OpenGLRenderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

void Renderer::setViewport(const glm::ivec2& vp)
{
	viewport = vp;

	projection2D = glm::ortho(0.f, (float)viewport.x, (float)viewport.y, 0.f, -1.f, 1.f);
}

void Renderer::swap()
{
	drawCounter = 0;
	textureCounter = 0;
	bufferCounter = 0;
}

int Renderer::getDrawCount()
{
	return drawCounter;
}

int Renderer::getBufferCount()
{
	return bufferCounter;
}

int Renderer::getTextureCount()
{
	return textureCounter;
}

const Renderer::SceneUniformData& Renderer::getSceneData() const
{
	return lastSceneData;
}

void OpenGLRenderer::useDrawBuffer(DrawBuffer* dbuff)
{
	if( dbuff != currentDbuff )
	{
		glBindVertexArray(dbuff->getVAOName());
		currentDbuff = dbuff;
		bufferCounter++;
#if RW_USING(RENDER_PROFILER)
		if( currentDebugDepth > 0 )
		{
			profileInfo[currentDebugDepth-1].buffers++;
		}
#endif
	}
}

void OpenGLRenderer::useTexture(GLuint unit, GLuint tex)
{
	if( currentTextures[unit] != tex )
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, tex);
		currentTextures[unit] = tex;
		textureCounter++;
#if RW_USING(RENDER_PROFILER)
		if( currentDebugDepth > 0 )
		{
			profileInfo[currentDebugDepth-1].textures++;
		}
#endif

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

#if 0
template<>
void OpenGLRenderer::uploadUBO<OpenGLRenderer::ObjectUniformData>(GLuint buffer, const ObjectUniformData& data)
{
	if( currentUBO != buffer ) {
		glBindBuffer(GL_UNIFORM_BUFFER, buffer);
		currentUBO = buffer;
	}
	/*glBindBufferRange(GL_UNIFORM_BUFFER,
					  2,
					  UBOObject,
					  entryAlignment * currentObjectEntry,
					  sizeof(ObjectUniformData));*/
	glBufferSubData(GL_UNIFORM_BUFFER,
					0,
					sizeof(ObjectUniformData), &data);
#if RW_USING(RENDER_PROFILER)
	if( currentDebugDepth > 0 )
	{
		profileInfo[currentDebugDepth-1].uploads++;
	}
#endif
	currentObjectEntry = (currentObjectEntry+1) % maxObjectEntries;
}
#endif

OpenGLRenderer::OpenGLRenderer()
	: currentDbuff(nullptr)
	, currentProgram(nullptr)
	, maxObjectEntries(0)
	, currentObjectEntry(0)
	, entryAlignment(0)
	, currentDebugDepth(0)
{
	// We need to query for some profiling exts.
	ogl_CheckExtensions();

	glGenBuffers(1, &UBOScene);
	glGenBuffers(1, &UBOObject);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBOScene);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, UBOObject);
	
	swap();

	GLint maxUBOSize, UBOAlignment;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUBOSize);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UBOAlignment);
	entryAlignment = 128;
	maxObjectEntries = maxUBOSize / entryAlignment;
	std::cout << "Max UBO Size: " << maxUBOSize << std::endl;
	std::cout << "UBO Alignment: " << UBOAlignment << std::endl;
	std::cout << "Max batch size: " << maxObjectEntries << std::endl;

	glBindBuffer(GL_UNIFORM_BUFFER, UBOObject);
	glBufferData(GL_UNIFORM_BUFFER,
				 entryAlignment * maxObjectEntries,
				 NULL,
				 GL_STREAM_DRAW);

	glGenQueries(1, &debugQuery);
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

void OpenGLRenderer::setUniform(Renderer::ShaderProgram* p, const std::string& name, const glm::mat4& m)
{
	useProgram(p);

	glUniformMatrix4fv(currentProgram->getUniformLocation(name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
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
	lastSceneData = data;
}

void OpenGLRenderer::draw(const glm::mat4& model, DrawBuffer* draw, const Renderer::DrawParameters& p)
{
	useDrawBuffer(draw);

	for( GLuint u = 0; u < p.textures.size(); ++u )
	{
		useTexture(u, p.textures[u]);
	}

	ObjectUniformData oudata {
		model,
		glm::vec4(p.colour.r/255.f, p.colour.g/255.f, p.colour.b/255.f, 1.f),
		1.f,
		1.f,
		p.colour.a/255.f
	};
	uploadUBO(UBOObject, oudata);

	drawCounter++;
#if RW_USING(RENDER_PROFILER)
	if( currentDebugDepth > 0 )
	{
		profileInfo[currentDebugDepth-1].draws++;
		profileInfo[currentDebugDepth-1].primitives += p.count;
	}
#endif

	glDrawElements(draw->getFaceType(), p.count, GL_UNSIGNED_INT,
				   (void*) (sizeof(RenderIndex) * p.start));
}

void OpenGLRenderer::drawArrays(const glm::mat4& model, DrawBuffer* draw, const Renderer::DrawParameters& p)
{
	useDrawBuffer(draw);

	for( GLuint u = 0; u < p.textures.size(); ++u )
	{
		useTexture(u, p.textures[u]);
	}

	ObjectUniformData oudata {
		model,
		glm::vec4(p.colour.r/255.f, p.colour.g/255.f, p.colour.b/255.f, 1.f),
		1.f,
		1.f,
		p.colour.a/255.f
	};
	uploadUBO(UBOObject, oudata);

	drawCounter++;
#if RW_USING(RENDER_PROFILER)
	if( currentDebugDepth > 0 )
	{
		profileInfo[currentDebugDepth-1].draws++;
		profileInfo[currentDebugDepth-1].primitives += p.count;
	}
#endif

	glDrawArrays(draw->getFaceType(), p.start, p.count);
}

void OpenGLRenderer::drawBatched(const RenderList& list)
{
#if 0 // Needs shader changes
	// Determine how many batches we need to process the entire list
	auto entries = list.size();
	glBindBuffer(GL_UNIFORM_BUFFER, UBOObject);
	for (int b = 0; b < entries; b += maxObjectEntries)
	{
		auto toConsume = std::min((GLuint)entries, b + maxObjectEntries) - b;
		std::vector<ObjectUniformData> uploadBuffer;
		uploadBuffer.resize(toConsume);
		for (int d = 0; d < toConsume; ++d)
		{
			auto& draw = list[b+d];
			uploadBuffer[d] = {
				draw.model,
				glm::vec4(draw.drawInfo.colour.r/255.f,
				draw.drawInfo.colour.g/255.f,
				draw.drawInfo.colour.b/255.f, 1.f),
				1.f,
				1.f,
				draw.drawInfo.colour.a/255.f
			};
		}
		glBufferData(GL_UNIFORM_BUFFER,
					 toConsume * sizeof(ObjectUniformData),
					 uploadBuffer.data(),
					 GL_STREAM_DRAW);

		// Dispatch individual draws
		for (int d = 0; d < toConsume; ++d)
		{
			auto& draw = list[b+d];
			useDrawBuffer(draw.dbuff);

			for( GLuint u = 0; u < draw.drawInfo.textures.size(); ++u )
			{
				useTexture(u, draw.drawInfo.textures[u]);
			}

			glDrawElements(draw.dbuff->getFaceType(), draw.drawInfo.count, GL_UNSIGNED_INT,
						   (void*) (sizeof(RenderIndex) * draw.drawInfo.start));
		}
	}
#else
	for(auto& ri : list)
	{
		draw(ri.model, ri.dbuff, ri.drawInfo);
	}
#endif
}

void OpenGLRenderer::invalidate()
{
	currentDbuff = nullptr;
	currentProgram = nullptr;
	currentTextures.clear();
	currentUBO = 0;
}

void OpenGLRenderer::pushDebugGroup(const std::string& title)
{
#if RW_USING(RENDER_PROFILER)
	if( ogl_ext_KHR_debug )
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, title.c_str());
		ProfileInfo& prof = profileInfo[currentDebugDepth];
		prof.buffers = prof.draws = prof.textures = prof.uploads = prof.primitives = 0;
		
		glQueryCounter(debugQuery, GL_TIMESTAMP);
		glGetQueryObjectui64v(debugQuery, GL_QUERY_RESULT, &prof.timerStart);
		
		currentDebugDepth++;
		assert( currentDebugDepth < MAX_DEBUG_DEPTH );
	}
#endif
}

const Renderer::ProfileInfo& OpenGLRenderer::popDebugGroup()
{
#if RW_USING(RENDER_PROFILER)
	if( ogl_ext_KHR_debug )
	{
		glPopDebugGroup();
		currentDebugDepth--;
		assert( currentDebugDepth >= 0 );

		ProfileInfo& prof = profileInfo[currentDebugDepth];

		glQueryCounter(debugQuery, GL_TIMESTAMP);
		GLuint64 current_time;
		glGetQueryObjectui64v(debugQuery, GL_QUERY_RESULT, &current_time);

		prof.duration = current_time - prof.timerStart;

		// Add counters to the parent group
		if( currentDebugDepth > 0 )
		{
			ProfileInfo& p = profileInfo[currentDebugDepth-1];
			p.draws += prof.draws;
			p.buffers += prof.buffers;
			p.primitives += prof.primitives;
			p.textures += prof.textures;
			p.uploads += prof.uploads;
		}

		return prof;
	}
#endif
	return profileInfo[0];
}
