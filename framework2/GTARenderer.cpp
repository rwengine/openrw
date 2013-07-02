#include "renderwure/render/GTARenderer.hpp"
#include <renderwure/engine/GTAEngine.hpp>
#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

const char *vertexShaderSource = "#version 130\n"
"in vec3 position;"
"in vec2 texCoords;"
"out vec2 TexCoords;"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"
"void main()"
"{"
"	TexCoords = texCoords;"
"	gl_Position = proj * view * model * vec4(position, 1.0);"
"}";
const char *fragmentShaderSource = "#version 130\n"
"in vec2 TexCoords;"
"uniform sampler2D texture;"
"void main()"
"{"
"   vec4 c = texture2D(texture, TexCoords);"
"   if(c.a < 0.9) discard;"
"	gl_FragColor = c;"
"}";

GLuint uniModel, uniProj, uniView;
GLuint posAttrib, texAttrib;
GLuint worldProgram;

GLuint compileShader(GLenum type, const char *source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		GLchar *buffer = new GLchar[len];
		glGetShaderInfoLog(shader, len, NULL, buffer);

		std::cerr << "ERROR compiling shader: " << buffer << std::endl;
		delete[] buffer;
		exit(1);
	}

	return shader;
}


GTARenderer::GTARenderer()
: camera()
{
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	GLuint worldProgram = glCreateProgram();
	glAttachShader(worldProgram, vertexShader);
	glAttachShader(worldProgram, fragmentShader);
	glLinkProgram(worldProgram);
	glUseProgram(worldProgram);

	posAttrib = glGetAttribLocation(worldProgram, "position");
	texAttrib = glGetAttribLocation(worldProgram, "texCoords");

	uniModel = glGetUniformLocation(worldProgram, "model");
	uniView = glGetUniformLocation(worldProgram, "view");
	uniProj = glGetUniformLocation(worldProgram, "proj");
}

void GTARenderer::renderWorld(GTAEngine* engine)
{
	glm::mat4 proj = camera.frustum.projection();
	glm::mat4 view = camera.view;
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
	
	camera.frustum.update();	

	auto& textureLoader = engine->gameData.textureLoader;

	for (size_t i = 0; i < engine->instances.size(); ++i) {
		auto &obj = engine->instances[i];
		std::string modelname = obj.model;
		if (modelname.substr(0, 3) == "LOD")
			continue;
		auto &model = engine->gameData.models[modelname];
		// std::cout << "Rendering " << modelname << std::endl;
		
		if(!model)
		{
			std::cout << "model " << modelname << " not there (" << engine->gameData.models.size() << " models loaded)" << std::endl;
		}

		for (size_t g = 0; g < model->geometries.size(); g++) {

			// This is a hack I have no idea why negating the quaternion fixes the issue but it does.
			glm::quat rot(-obj.rotW, obj.rotX, obj.rotY, obj.rotZ);
			glm::mat4 matrixModel;
			matrixModel = glm::translate(matrixModel, glm::vec3(obj.posX, obj.posY, obj.posZ));
			matrixModel = glm::scale(matrixModel, glm::vec3(obj.scaleX, obj.scaleY, obj.scaleZ));
			matrixModel = matrixModel * glm::mat4_cast(rot);
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(matrixModel));
			
			glBindBuffer(GL_ARRAY_BUFFER, model->geometries[g].VBO);
			glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)(model->geometries[g].vertices.size() * sizeof(float) * 3));
			glEnableVertexAttribArray(posAttrib);
			glEnableVertexAttribArray(texAttrib);
			
			for(size_t sg = 0; sg < model->geometries[g].subgeom.size(); ++sg) 
			{
				if (model->geometries[g].materials.size() > model->geometries[g].subgeom[sg].material) { 
					// std::cout << model->geometries[g].textures.size() << std::endl;
					// std::cout << "Looking for " << model->geometries[g].textures[0].name << std::endl;
					if(model->geometries[g].materials[model->geometries[g].subgeom[sg].material].textures.size() > 0) {
						textureLoader.bindTexture(model->geometries[g].materials[model->geometries[g].subgeom[sg].material].textures[0].name);
					}
				}
				
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->geometries[g].subgeom[sg].EBO);

				glDrawElements(GL_TRIANGLES, model->geometries[g].subgeom[sg].indices.size(), GL_UNSIGNED_INT, NULL);
			}
		}
	}
}