#define GLEW_STATIC
#include <GL/glew.h>

#include "../framework/LoaderIPL.h"
#include "../framework/LoaderIMG.h"
#include "../framework/LoaderDFF.h"
#include "../framework/TextureLoader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>

#include <memory>

constexpr int WIDTH  = 800,
              HEIGHT = 600;

sf::Window window;

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
// "	gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
"	gl_FragColor = texture2D(texture, TexCoords);"
"}";

GLuint uniModel, uniProj, uniView;
GLuint posAttrib, texAttrib;

LoaderDFF dffLoader;
TextureLoader textureLoader;
LoaderIPL iplLoader;

std::map<std::string, std::unique_ptr<Model>> models;
Model *selectedModel;
glm::vec3 selectedModelCenter;

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

void handleEvent(sf::Event &event)
{
	switch (event.type) {
	case sf::Event::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::Escape:
			window.close();
			break;
		default: break;
		}
		break;
	default: break;
	}
}

void init(std::string gtapath)
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnable(GL_DEPTH_TEST);

	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	posAttrib = glGetAttribLocation(shaderProgram, "position");
	texAttrib = glGetAttribLocation(shaderProgram, "texCoords");

	uniModel = glGetUniformLocation(shaderProgram, "model");
	uniView = glGetUniformLocation(shaderProgram, "view");
	uniProj = glGetUniformLocation(shaderProgram, "proj");

	glm::mat4 proj = glm::perspective(80.f, (float) WIDTH/HEIGHT, 0.1f, 5000.f);
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	LoaderIMG imgLoader;

	if (imgLoader.load(gtapath +"/models/gta3")) {
		for (int i = 0; i < imgLoader.getAssetCount(); i++) {
			auto &asset = imgLoader.getAssetInfoByIndex(i);

			std::string filename = asset.name;

			auto filetype = filename.substr(filename.size() - 3);
			std::transform(filetype.begin(), filetype.end(), filetype.begin(), ::tolower);

			if (filetype == "dff") {
				std::string modelname = filename.substr(0, filename.size() - 4);

				char *file = imgLoader.loadToMemory(filename);
				models[modelname] = std::move(dffLoader.loadFromMemory(file));
			} else if (filetype == "txd") {
				char *file = imgLoader.loadToMemory(filename);
				textureLoader.loadFromMemory(file);
			}
		}
	}

	if (iplLoader.load(gtapath +"/data/maps/industSW.ipl")) {
		printf("IPL Loaded, size: %d\n", iplLoader.m_instances.size());

		// Get the center of the model by averaging all the vertices! Hax!
		for (int i = 0; i < iplLoader.m_instances.size(); i++) {
			selectedModelCenter += glm::vec3{
				iplLoader.m_instances[i].posX,
				iplLoader.m_instances[i].posY,
				iplLoader.m_instances[i].posZ
			};
		}
		selectedModelCenter /= iplLoader.m_instances.size();
	} else {
		printf("IPL failed to load.\n");
		exit(1);
	}

	textureLoader.loadFromFile("MISC.TXD");

	selectedModel = models["Jetty"].get();
}

void update()
{
	static int i = 0;
	constexpr float rotspeed = 80;

	glm::mat4 view;
	glm::vec3 pos{
		-selectedModelCenter.x + cos(i / rotspeed) * 400,
		-selectedModelCenter.y + sin(i / rotspeed) * 400,
		-selectedModelCenter.z - 200,
	};
	view = glm::rotate(view, -50.f, glm::vec3(1, 0, 0));
	view = glm::rotate(view, (i/(6.28f*rotspeed))*360 - 90, glm::vec3(0, 0, -1));
	view = glm::translate(view, pos);
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	i++;
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (size_t i = 0; i < iplLoader.m_instances.size(); ++i) {
		auto &obj = iplLoader.m_instances[i];
		std::string modelname = obj.model;
		if (modelname.substr(0, 3) == "LOD")
			continue;
		auto &model = models[modelname];
		// std::cout << "Rendering " << modelname << std::endl;

		for (size_t g = 0; g < model->geometries.size(); g++) {
			if (model->geometries[g].textures.size() > 0) {
				// std::cout << model->geometries[g].textures.size() << std::endl;
				// std::cout << "Looking for " << model->geometries[g].textures[0].name << std::endl;
				textureLoader.bindTexture(model->geometries[g].textures[0].name);
			}

			glm::mat4 matrixModel;
			glm::quat rot{obj.rotX, obj.rotY, obj.rotZ, obj.rotW};
			matrixModel = glm::translate(matrixModel, glm::vec3(obj.posX, obj.posY, obj.posZ));
			matrixModel = glm::rotate(matrixModel, glm::angle(rot), glm::axis(rot));
			matrixModel = glm::scale(matrixModel, glm::vec3(obj.scaleX, obj.scaleY, obj.scaleZ));
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(matrixModel));

			glBindBuffer(GL_ARRAY_BUFFER, model->geometries[g].VBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->geometries[g].EBO);
			glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(posAttrib);
			glEnableVertexAttribArray(texAttrib);

			glDrawElements(GL_TRIANGLES, model->geometries[g].triangles.size() * 3, GL_UNSIGNED_SHORT, NULL);
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " <path to GTA3 root folder>" << std::endl;
		exit(1);
	}

	glewExperimental = GL_TRUE;
	glewInit();

	window.create(sf::VideoMode(WIDTH, HEIGHT), "GTA3 Viewer", sf::Style::Close);
	window.setVerticalSyncEnabled(true);

	init(argv[1]);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleEvent(event);
		}

		update();

		render();
		window.display();
	}

	return 0;
}
