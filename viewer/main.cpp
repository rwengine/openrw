#define GLEW_STATIC
#include <GL/glew.h>

#include <engine/GTAEngine.h>
#include <loaders/LoaderDFF.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>

#include <memory>

constexpr int WIDTH  = 800,
              HEIGHT = 600;

constexpr double PiOver180 = 3.1415926535897932384626433832795028/180;

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
"   vec4 c = texture2D(texture, TexCoords);"
"   if(c.a < 0.9) discard;"
"	gl_FragColor = c;"
"}";

GLuint uniModel, uniProj, uniView;
GLuint posAttrib, texAttrib;

LoaderDFF dffLoader;
GTAEngine* gta = nullptr;

glm::vec3 selectedModelCenter;

glm::vec3 plyPos;
glm::vec2 plyLook;

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
	
	// GTA GET
	gta = new GTAEngine(gtapath);
	
	// This is harcoded in GTA III for some reason
	gta->gameData.loadIMG("/models/gta3");
	
	gta->load();
	
	// Test out a known IPL.
	gta->loadItems(gtapath + "/data/maps/industsw/industSW.ipl");
	//gta->loadItems(gtapath + "/data/maps/industnw/industNW.ipl");
	//gta->loadItems(gtapath + "/data/maps/industse/industSE.ipl");
	//gta->loadItems(gtapath + "/data/maps/industne/industNE.ipl");
	
	plyPos = gta->itemCentroid / (float) gta->instances.size();
}

void update()
{
	float dt = 1.0/60.0; // we'll be fine
	static int i = 0;
	constexpr float moveSpeed = 20;

	sf::Vector2i screenCenter{sf::Vector2i{window.getSize()} / 2};
	sf::Vector2i mousePos = sf::Mouse::getPosition(window);
	sf::Vector2i deltaMouse = mousePos - screenCenter;
	sf::Mouse::setPosition(screenCenter, window);

	plyLook.x += deltaMouse.x / 10.0;
	plyLook.y += deltaMouse.y / 10.0;

	if (plyLook.y > 90)
		plyLook.y = 90;
	else if (plyLook.y < -90)
		plyLook.y = -90;

	bool doMove = false;
	int direction = 1;
	int strafeDirection = 0;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
		doMove = true;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
		doMove = true;
		direction = -1;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
		doMove = true;
		strafeDirection = -1;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
		doMove = true;
		strafeDirection = 1;
	}
	if (doMove) {
		plyPos += dt * moveSpeed * direction * glm::vec3{
			sin((plyLook.x + 90*strafeDirection) * PiOver180),
			cos((plyLook.x + 90*strafeDirection) * PiOver180),
			-sin(plyLook.y * PiOver180),
		};
	}

	glm::mat4 view;
	view = glm::rotate(view, -90.f, glm::vec3(1, 0, 0));
	view = glm::rotate(view, plyLook.y, glm::vec3(1, 0, 0));
	view = glm::rotate(view, plyLook.x, glm::vec3(0, 0, 1));
	view = glm::translate(view, -plyPos);
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	i++;
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	auto& textureLoader = gta->gameData.textureLoader;

	for (size_t i = 0; i < gta->instances.size(); ++i) {
		auto &obj = gta->instances[i];
		std::string modelname = obj.model;
		if (modelname.substr(0, 3) == "LOD")
			continue;
		auto &model = gta->gameData.models[modelname];
		// std::cout << "Rendering " << modelname << std::endl;
		
		if(!model)
		{
			std::cout << "model " << modelname << " not there (" << gta->gameData.models.size() << " models loaded)" << std::endl;
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
