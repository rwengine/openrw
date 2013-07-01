#define GLEW_STATIC
#include <GL/glew.h>

#include "../framework/LoaderIPL.h"
#include "../framework/LoaderIMG.h"
#include "../framework/LoaderDFF.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>

constexpr int WIDTH  = 800,
              HEIGHT = 600;

sf::Window window;

const char *vertexShaderSource = "#version 130\n"
"in vec3 position;"
// "in vec2 texCoords;"
// "out vec2 TexCoords;"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"
"void main()"
"{"
// "	TexCoords = texCoords;"
"	gl_Position = proj * model * vec4(position, 1.0);"
"}";
const char *fragmentShaderSource = "#version 130\n"
// "in vec2 TexCoords;"
// "uniform sampler2D texture;"
"void main()"
"{"
"	gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
// "	gl_FragColor = texture2D(texture, TexCoords);"
"}";

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

GLuint uniModel, uniProj, uniView;
GLuint posAttrib;
GLuint VBO, EBO;

LoaderDFF dffLoader;

LoaderDFF::Geometry *selectedGeometry;

void init(std::string filepath)
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	// glEnable(GL_DEPTH_TEST);

	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	posAttrib = glGetAttribLocation(shaderProgram, "position");
	GLuint texAttrib = glGetAttribLocation(shaderProgram, "texCoords");

	uniModel = glGetUniformLocation(shaderProgram, "model");
	uniView = glGetUniformLocation(shaderProgram, "view");
	uniProj = glGetUniformLocation(shaderProgram, "proj");

	LoaderIPL iplLoader;
	LoaderIMG imgLoader;

	if (iplLoader.load(filepath)) {
		printf("IPL Loaded, size: %d\n", iplLoader.m_instances.size());

		if (imgLoader.load("/home/iostream/.wine/drive_c/Program Files (x86)/Rockstar Games/GTAIII/models/gta3")) {
			std::string filename = iplLoader.m_instances[0].model + ".dff";
			std::cout << "Loading " << filename << std::endl;
			char *file = imgLoader.loadToMemory(filename);

			dffLoader.loadFromMemory(file);

			selectedGeometry = &dffLoader.geometries[0];
/*
			for (int i = 0; i < 10; i++) {
				auto v = selectedGeometry->vertices[i];
				std::cout << v.x << ", " << v.y << ", " << v.z << std::endl;
			}
*/

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(
				GL_ARRAY_BUFFER,
				selectedGeometry->vertices.size() * 3 * sizeof(float),
				&selectedGeometry->vertices[0],
				GL_STATIC_DRAW
			);

			uint16_t indicies[selectedGeometry->triangles.size() * 3];
			size_t i = 0;
			for (auto &tri : selectedGeometry->triangles) {
				indicies[i]     = tri.first;
				indicies[i + 1] = tri.second;
				indicies[i + 2] = tri.third;
				i += 3;
			}
			/*
			for (int i = 0; i < 8; i++) {
				glm::vec3 t{indicies[i*3], indicies[i*3 + 1], indicies[i*3 + 2]};
				auto v1 = selectedGeometry->vertices[t.x];
				auto v2 = selectedGeometry->vertices[t.y];
				auto v3 = selectedGeometry->vertices[t.z];
				std::cout << t.x << ", " << t.y << ", " << t.z << std::endl;
				std::cout << v1.x << ", " << v1.y << ", " << v1.z << std::endl;
				std::cout << v2.x << ", " << v2.y << ", " << v2.z << std::endl;
				std::cout << v3.x << ", " << v3.y << ", " << v3.z << std::endl;
				std::cout << std::endl;
			}
			*/
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER,
				sizeof(indicies),
				indicies,
				GL_STATIC_DRAW
			);

			glm::mat4 proj = glm::perspective(80.f, (float) WIDTH/HEIGHT, 0.1f, 100.f);
			glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
		} else {
			std::cerr << "IMG failed to load" << std::endl;
			exit(1);
		}
/*
		for (size_t i = 0; i < iplLoader.m_instances.size(); ++i) {
			printf("IPL entry id: %d\n", iplLoader.m_instances[i].id);
		}
*/
	} else {
		printf("IPL failed to load.\n");
	}
}

void update()
{
	static int i = 0;

	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0, -10.0));
	model = glm::rotate(model, 100.f, glm::vec3(1, 0, 0));
	model = glm::rotate(model, i*1.f, glm::vec3(0, 0, 1));
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	i++;
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);

	glDrawElements(GL_TRIANGLES, selectedGeometry->triangles.size() * 3, GL_UNSIGNED_SHORT, NULL);
}

int main(int argc, char *argv[])
{
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
