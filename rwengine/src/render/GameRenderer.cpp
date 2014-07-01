#include <render/GameRenderer.hpp>
#include <engine/GameWorld.hpp>
#include <engine/Animator.hpp>
#include <render/TextureAtlas.hpp>
#include <render/Model.hpp>

#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <ai/CharacterController.hpp>
#include <data/ObjectData.hpp>
#include <items/InventoryItem.hpp>

#include <render/GameShaders.hpp>

#include <deque>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

GLuint GameRenderer::currentUBO = 0;

const size_t skydomeSegments = 8, skydomeRows = 10;

struct WaterVertex {
	static const AttributeList vertex_attributes() {
		return {
			{ATRS_Position, 2, sizeof(WaterVertex),  0ul}
		};
	}

	float x, y;
};

std::vector<WaterVertex> waterLQVerts = {
	{1.0f, 1.0f},
	{0.0f, 1.0f},
	{1.0f,-0.0f},
	{0.0f,-0.0f}
};

std::vector<WaterVertex> waterHQVerts;

GeometryBuffer waterLQBuffer;
DrawBuffer waterLQDraw;
GeometryBuffer waterHQBuffer;
DrawBuffer waterHQDraw;

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

		std::cerr << "ERROR compiling shader: " << buffer << "\nSource: " << source << std::endl;
		delete[] buffer;
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

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return prog;
}

GameRenderer::GameRenderer(GameWorld* engine)
	: engine(engine), _renderAlpha(0.f)
{
	worldProgram = compileProgram(GameShaders::WorldObject::VertexShader,
								  GameShaders::WorldObject::FragmentShader);
	
	uniTexture = glGetUniformLocation(worldProgram, "texture");
	ubiScene = glGetUniformBlockIndex(worldProgram, "SceneData");
	ubiObject = glGetUniformBlockIndex(worldProgram, "ObjectData");

	glGenBuffers(1, &uboScene);
	glGenBuffers(1, &uboObject);

	glUniformBlockBinding(worldProgram, ubiScene, 1);
	glUniformBlockBinding(worldProgram, ubiObject, 2);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboScene);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboObject);

	skyProgram = compileProgram(GameShaders::Sky::VertexShader,
								GameShaders::Sky::FragmentShader);

	skyUniView = glGetUniformLocation(skyProgram, "view");
	skyUniProj = glGetUniformLocation(skyProgram, "proj");
	skyUniTop = glGetUniformLocation(skyProgram, "TopColor");
	skyUniBottom = glGetUniformLocation(skyProgram, "BottomColor");

	waterProgram = compileProgram(GameShaders::WaterHQ::VertexShader,
								  GameShaders::WaterHQ::FragmentShader);

	waterHeight = glGetUniformLocation(waterProgram, "height");
	waterTexture = glGetUniformLocation(waterProgram, "texture");
	waterSize = glGetUniformLocation(waterProgram, "size");
	waterMVP = glGetUniformLocation(waterProgram, "MVP");
	waterTime = glGetUniformLocation(waterProgram, "time");
	waterPosition = glGetUniformLocation(waterProgram, "worldP");
	waterWave = glGetUniformLocation(waterProgram, "waveParams");

	
	glGenVertexArrays( 1, &vao );

	// Upload water plane
	waterLQBuffer.uploadVertices(waterLQVerts);
	waterLQDraw.addGeometry(&waterLQBuffer);
	waterLQDraw.setFaceType(GL_TRIANGLE_STRIP);

	// Generate HQ water geometry
	int waterverts = 5;
	float vertStep = 1.f/waterverts;
	for(int x = 0; x < waterverts; ++x) {
		float xB = vertStep * x;
		for(int y = 0; y < waterverts; ++y) {
			float yB = vertStep * y;
			waterHQVerts.push_back({xB + vertStep, yB + vertStep});
			waterHQVerts.push_back({xB,            yB + vertStep});
			waterHQVerts.push_back({xB + vertStep, yB           });

			waterHQVerts.push_back({xB + vertStep, yB           });
			waterHQVerts.push_back({xB,            yB + vertStep});
			waterHQVerts.push_back({xB,            yB           });
		}
	}

	waterHQBuffer.uploadVertices(waterHQVerts);
	waterHQDraw.addGeometry(&waterHQBuffer);
	waterHQDraw.setFaceType(GL_TRIANGLES);


	// And our skydome while we're at it.
	glGenBuffers(1, &skydomeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skydomeVBO);
	size_t segments = skydomeSegments, rows = skydomeRows;

    float R = 1.f/(float)(rows-1);
    float S = 1.f/(float)(segments-1);
    glm::vec3 skydomeBuff[rows * segments];
    for( size_t r = 0, i = 0; r < rows; ++r) {
        for( size_t s = 0; s < segments; ++s) {
            skydomeBuff[i++] = glm::vec3(
                        cos(2.f * M_PI * s * S) * cos(M_PI_2 * r * R),
                        sin(2.f * M_PI * s * S) * cos(M_PI_2 * r * R),
                        sin(M_PI_2 * r * R)
                        );
		}
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(skydomeBuff), skydomeBuff, GL_STATIC_DRAW);

    glGenBuffers(1, &skydomeIBO);
    GLushort skydomeIndBuff[rows*segments*6];
    for( size_t r = 0, i = 0; r < (rows-1); ++r ) {
        for( size_t s = 0; s < (segments-1); ++s ) {
            skydomeIndBuff[i++] = r * segments + s;
            skydomeIndBuff[i++] = r * segments + (s+1);
            skydomeIndBuff[i++] = (r+1) * segments + (s+1);
            skydomeIndBuff[i++] = r * segments + s;
            skydomeIndBuff[i++] = (r+1) * segments + (s+1);
            skydomeIndBuff[i++] = (r+1) * segments + s;
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skydomeIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skydomeIndBuff), skydomeIndBuff, GL_STATIC_DRAW);

    glGenBuffers(1, &debugVBO);
    glGenTextures(1, &debugTex);
    glGenVertexArrays(1, &debugVAO);
}

float mix(uint8_t a, uint8_t b, float num)
{
	return a+(b-a)*num;
}

#define GL_PLS() \
{ auto errc = glGetError(); \
	if(errc != GL_NO_ERROR) std::cout << __LINE__ << ": " << errc << std::endl;\
}

void GameRenderer::renderWorld(float alpha)
{
	_renderAlpha = alpha;

	glBindVertexArray( vao );
	
    float tod = fmod(engine->gameTime, 24.f * 60.f);

	// Requires a float 0-24
    auto weather = engine->gameData.weatherLoader.getWeatherData(WeatherLoader::Sunny, tod / 60.f);

    glm::vec3 skyTop = weather.skyTopColor;
    glm::vec3 skyBottom = weather.skyBottomColor;
    glm::vec3 ambient = weather.ambientColor;
    glm::vec3 dynamic = weather.directLightColor;

	float theta = (tod/(60.f * 24.f) - 0.5f) * 2 * 3.14159265;
	glm::vec3 sunDirection{
		sin(theta),
		0.0,
		cos(theta),
	};
    sunDirection = glm::normalize(sunDirection);
    camera.frustum.far = weather.farClipping;

	glUseProgram(worldProgram);

	auto view = camera.frustum.view;
	auto proj = camera.frustum.projection();

	uploadUBO<SceneUniformData>(
				uboScene,
				{
					proj,
					view,
					glm::vec4{ambient, 1.0f},
					glm::vec4{dynamic, 1.0f},
					weather.fogStart,
					camera.frustum.far
				});
	
	glClearColor(skyBottom.r, skyBottom.g, skyBottom.b, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera.frustum.update(proj * view);
	
	rendered = culled = 0;

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(uniTexture, 0);

	for( GameObject* object : engine->objects ) {
		switch(object->type()) {
		case GameObject::Character:
			renderPedestrian(static_cast<CharacterObject*>(object));
			break;
		case GameObject::Vehicle:
			renderVehicle(static_cast<VehicleObject*>(object));
			break;
		case GameObject::Instance:
			renderInstance(static_cast<InstanceObject*>(object));
			break;
		default: break;
		}
	}
	
	// Draw anything that got queued.
	for(auto it = transparentDrawQueue.begin();
		it != transparentDrawQueue.end();
		++it)
	{
		glBindVertexArray(it->model->geometries[it->g]->dbuff.getVAOName());

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it->model->geometries[it->g]->EBO);

		renderSubgeometry(it->model, it->g, it->sg, it->matrix, it->object, false);
	}
	transparentDrawQueue.clear();

	// Draw the water.
	glUseProgram( waterProgram );

	// TODO: Add some kind of draw distance

	float blockLQSize = WATER_WORLD_SIZE/WATER_LQ_DATA_SIZE;
	float blockHQSize = WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE;

	glm::vec2 waterOffset { -WATER_WORLD_SIZE/2.f, -WATER_WORLD_SIZE/2.f };
	glUniform1i(waterTexture, 0);
	glUniform2f(waterWave, WATER_SCALE, WATER_HEIGHT);
	auto waterTex = engine->gameData.textures[{"water_old",""}];
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex.texName);

	auto camposFlat = glm::vec2(camera.worldPos);

	glBindVertexArray( waterHQDraw.getVAOName() );

	// Draw High detail water
	glUniform1f(waterSize, blockHQSize);
	glUniform1f(waterTime, engine->gameTime);
	for( int x = 0; x < WATER_HQ_DATA_SIZE; x++ ) {
		for( int y = 0; y < WATER_HQ_DATA_SIZE; y++ ) {
			auto waterWS = waterOffset + glm::vec2(blockHQSize) * glm::vec2(x, y);
			auto cullWS = waterWS + (blockHQSize / 2.f);

			// Check that this is the right time to draw the HQ water
			if( glm::distance(camposFlat, cullWS) - blockHQSize >= WATER_HQ_DISTANCE ) continue;

			int i = (x*WATER_HQ_DATA_SIZE) + y;
			int hI = engine->gameData.realWater[i];
			if( hI >= NO_WATER_INDEX ) continue;
			float h = engine->gameData.waterHeights[hI];

			glUniform1f(waterHeight, h);
			auto MVP = proj * view;
			glUniform2fv(waterPosition, 1, glm::value_ptr(waterWS));
			glUniformMatrix4fv(waterMVP, 1, GL_FALSE, glm::value_ptr(MVP));
			glDrawArrays(waterHQDraw.getFaceType(), 0, waterHQVerts.size());
		}
	}

	glBindVertexArray( waterLQDraw.getVAOName() );
	glUniform2f(waterWave, 0.0f, 0.0f);

	glUniform1f(waterSize, blockLQSize);
	for( int x = 0; x < WATER_LQ_DATA_SIZE; x++ ) {
		for( int y = 0; y < WATER_LQ_DATA_SIZE; y++ ) {
			auto waterWS = waterOffset + glm::vec2(blockLQSize) * glm::vec2(x, y);
			auto cullWS = waterWS + (blockLQSize / 2.f);

			// Check that this is the right time to draw the LQ
			if( glm::distance(camposFlat, cullWS) - blockHQSize/4.f < WATER_HQ_DISTANCE ) continue;
			if( glm::distance(camposFlat, cullWS) - blockLQSize/2.f > camera.frustum.far ) continue;

			int i = (x*WATER_LQ_DATA_SIZE) + y;
			int hI = engine->gameData.visibleWater[i];
			if( hI >= NO_WATER_INDEX ) continue;
			float h = engine->gameData.waterHeights[hI];

			glUniform1f(waterHeight, h);
			auto MVP = proj * view;
			glUniform2fv(waterPosition, 1, glm::value_ptr(waterWS));
			glUniformMatrix4fv(waterMVP, 1, GL_FALSE, glm::value_ptr(MVP));
			glDrawArrays(waterLQDraw.getFaceType(), 0, 4);
		}
	}

	glBindVertexArray( vao );
	
	glUseProgram(skyProgram);
	
	glBindBuffer(GL_ARRAY_BUFFER, skydomeVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skydomeIBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glUniformMatrix4fv(skyUniView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(skyUniProj, 1, GL_FALSE, glm::value_ptr(proj));
	glUniform4f(skyUniTop, skyTop.r, skyTop.g, skyTop.b, 1.f);
	glUniform4f(skyUniBottom, skyBottom.r, skyBottom.g, skyBottom.b, 1.f);

	glDrawElements(GL_TRIANGLES, skydomeSegments * skydomeRows * 6, GL_UNSIGNED_SHORT, NULL);

	// Draw bullets like this for now
	if( _tracers.size() > 0 ) {
		glUseProgram(worldProgram);
		glBindVertexArray( vao );
		glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
		glBindTexture(GL_TEXTURE_2D, debugTex);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _tracers.size() * 2, _tracers.data(), GL_STREAM_DRAW);
		GLint posAttrib = glGetAttribLocation(worldProgram, "position");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
		uploadUBO<ObjectUniformData>(
							uboObject, {
								glm::mat4(),
								glm::vec4(1.f),
								1.f, 1.f
							});

		float img3[] = {1.f, 1.f, 0.f};
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB, 1, 1,
			0, GL_RGB, GL_FLOAT, img3
		);
		glDrawArrays(GL_LINES, 0, _tracers.size());

		_tracers.clear();
	}

	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray( 0 );
}

void GameRenderer::renderPedestrian(CharacterObject *pedestrian)
{
	glm::mat4 matrixModel;

	matrixModel = glm::translate(matrixModel, pedestrian->getPosition());
	matrixModel = matrixModel * glm::mat4_cast(pedestrian->getRotation());

	if(!pedestrian->model->model) return;

	renderModel(pedestrian->model->model, matrixModel, pedestrian, pedestrian->animator);

	if(pedestrian->getActiveItem()) {
		auto handFrame = pedestrian->model->model->findFrame("srhand");
		glm::mat4 localMatrix;
		if( handFrame ) {
			while( handFrame->getParent() ) {
				localMatrix = pedestrian->animator->getFrameMatrix(handFrame) * localMatrix;
				handFrame = handFrame->getParent();
			}
		}
		renderItem(pedestrian->getActiveItem(), matrixModel * localMatrix);
	}
}

void GameRenderer::renderVehicle(VehicleObject *vehicle)
{
	if(!vehicle->model)
	{
		std::cout << "model " <<  vehicle->vehicle->modelName << " not loaded (" << engine->gameData.models.size() << " models loaded)" << std::endl;
	}

	glm::mat4 matrixModel;
	matrixModel = glm::translate(matrixModel, vehicle->getPosition());
	matrixModel = matrixModel * glm::mat4_cast(vehicle->getRotation());

	renderModel(vehicle->model->model, matrixModel, vehicle);

	// Draw wheels n' stuff
	for( size_t w = 0; w < vehicle->info->wheels.size(); ++w) {
		auto woi = engine->objectTypes.find(vehicle->vehicle->wheelModelID);
		if(woi != engine->objectTypes.end()) {
			Model* wheelModel = engine->gameData.models["wheels"]->model;
			if( wheelModel ) {
				// Tell bullet to update the matrix for this wheel.
				vehicle->physVehicle->updateWheelTransform(w, false);
				glm::mat4 wheel_tf;
				vehicle->physVehicle->getWheelTransformWS(w).getOpenGLMatrix(glm::value_ptr(wheel_tf));
				wheel_tf = glm::scale(wheel_tf, glm::vec3(vehicle->vehicle->wheelScale));
				if(vehicle->physVehicle->getWheelInfo(w).m_chassisConnectionPointCS.x() < 0.f) {
					wheel_tf = glm::scale(wheel_tf, glm::vec3(-1.f, 1.f, 1.f));
				}
				renderWheel(wheelModel, wheel_tf, woi->second->modelName);
			}
			else {
				std::cout << "Wheel model " << woi->second->modelName << " not loaded" << std::endl;
			}
		}
	}
}

void GameRenderer::renderInstance(InstanceObject *instance)
{
	if(instance->object->timeOn != instance->object->timeOff) {
		// Update rendering flags.
		if(engine->getHour() < instance->object->timeOn
			&& engine->getHour() > instance->object->timeOff) {
			return;
		}
	}

	if(!instance->model->model)
	{
		return;
	}

	glm::mat4 matrixModel;
	if( instance->body ) {
		instance->body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(matrixModel));
	}
	else {
		matrixModel = glm::translate(matrixModel, instance->position);
		matrixModel = glm::scale(matrixModel, instance->scale);
		matrixModel = matrixModel * glm::mat4_cast(instance->rotation);
	}

	float mindist = 100000.f;
	for (size_t g = 0; g < instance->model->model->geometries.size(); g++)
	{
		RW::BSGeometryBounds& bounds = instance->model->model->geometries[g]->geometryBounds;
		mindist = std::min(mindist, glm::length((glm::vec3(matrixModel[3])+bounds.center) - camera.worldPos) - bounds.radius);
	}

	if( instance->object->numClumps == 1 ) {
		if( mindist > instance->object->drawDistance[0] ) {
			// Check for LOD instances
			if ( instance->LODinstance ) {
				if( mindist > instance->LODinstance->object->drawDistance[0] ) {
					culled++;
					return;
				}
				else if (instance->LODinstance->model->model) {
					renderModel(instance->LODinstance->model->model, matrixModel);
				}
			}
		}
		else if (! instance->object->LOD ) {
			renderModel(instance->model->model, matrixModel);
		}
	}
	else {
		if( mindist > instance->object->drawDistance[1] ) {
			culled++;
			return;
		}
		else if( mindist > instance->object->drawDistance[0] ) {
			// Figure out which one is the LOD.
			auto RF = instance->model->model->frames[0];
			auto LODindex = RF->getChildren().size() - 2;
			auto f = RF->getChildren()[LODindex];
			renderFrame(instance->model->model, f, matrixModel * glm::inverse(f->getTransform()), nullptr);
		}
		else {
			// Draw the real object
			auto RF = instance->model->model->frames[0];
			auto LODindex = RF->getChildren().size() - 1;
			auto f = RF->getChildren()[LODindex];
			renderFrame(instance->model->model, f, matrixModel * glm::inverse(f->getTransform()), nullptr);
		}
	}
}

void GameRenderer::renderWheel(Model* model, const glm::mat4 &matrix, const std::string& name)
{
	for (const ModelFrame* f : model->frames) 
	{
		const std::string& fname = f->getName();
		if( fname != name ) {
			continue;
		}

		auto firstLod = f->getChildren()[0];

		for( auto& g : firstLod->getGeometries() ) {
			RW::BSGeometryBounds& bounds = model->geometries[g]->geometryBounds;
			if(! camera.frustum.intersects(bounds.center + glm::vec3(matrix[3]), bounds.radius)) {
				culled++;
				continue;
			}

			renderGeometry(model, g, matrix);
		}
		break;
	}
}

void GameRenderer::renderItem(InventoryItem *item, const glm::mat4 &modelMatrix)
{
	// srhand
	std::shared_ptr<ObjectData> odata = engine->objectTypes[item->getModelID()];
	auto weapons = engine->gameData.models["weapons"];
	if( weapons && weapons->model ) {
		auto itemModel = weapons->model->findFrame(odata->modelName + "_l0");
		auto matrix = glm::inverse(itemModel->getTransform());
		if(itemModel) {
			renderFrame(weapons->model, itemModel, modelMatrix * matrix, nullptr);
		}
		else {
			std::cerr << "weapons.dff missing frame " << odata->modelName << std::endl;
		}
	}
	else {
		std::cerr << "weapons.dff not loaded" << std::endl;
	}
}

void GameRenderer::renderGeometry(Model* model, size_t g, const glm::mat4& modelMatrix, GameObject* object)
{
	glBindVertexArray(model->geometries[g]->dbuff.getVAOName());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->geometries[g]->EBO);

	for(size_t sg = 0; sg < model->geometries[g]->subgeom.size(); ++sg)
	{
		if(! renderSubgeometry(model, g, sg, modelMatrix, object)) {
			// If rendering was rejected, queue for later.
			transparentDrawQueue.push_back(
				{model, g, sg, modelMatrix, object}
			);
		}
    }
}

bool GameRenderer::renderFrame(Model* m, ModelFrame* f, const glm::mat4& matrix, GameObject* object, bool queueTransparent)
{
	auto localmatrix = matrix;

	if(object && object->animator) {
		localmatrix *= object->animator->getFrameMatrix(f, _renderAlpha, object->isAnimationFixed());
	}
	else {
		localmatrix *= f->getTransform();
	}

	bool vis = (object == nullptr || object->animator == nullptr) ||
			object->animator->getFrameVisibility(f);

	for(size_t g : f->getGeometries()) {
		if(!vis ) continue;

		RW::BSGeometryBounds& bounds = m->geometries[g]->geometryBounds;
		if(! camera.frustum.intersects(bounds.center + glm::vec3(matrix[3]), bounds.radius)) {
			continue;
		}

		renderGeometry(m,
						g, localmatrix,
						object);
	}
	
	for(ModelFrame* c : f->getChildren()) {
		renderFrame(m, c, localmatrix, object, queueTransparent);
	}
	return true;
}

bool GameRenderer::renderSubgeometry(Model* model, size_t g, size_t sg, const glm::mat4& matrix, GameObject* object, bool queueTransparent)
{
	auto& subgeom = model->geometries[g]->subgeom[sg];

	/*
	 * model matrix,
	 * material diffuse
	 * material ambient
	 * materialcolour
	 */
	ObjectUniformData oudata {
		matrix,
		glm::vec4(1.f),
		1.f, 1.f
	};

	if (model->geometries[g]->materials.size() > subgeom.material) {
		Model::Material& mat = model->geometries[g]->materials[subgeom.material];

		if(mat.textures.size() > 0 ) {
			auto& tC = mat.textures[0].name;
			auto& tA = mat.textures[0].alphaName;
			auto t = engine->gameData.textures.find({tC, tA});
			if(t != engine->gameData.textures.end()) {
				TextureInfo& tex = t->second;
				if(tex.transparent && queueTransparent) {
					return false;
				}
				glBindTexture(GL_TEXTURE_2D, tex.texName);
			}
			else {
				// Texture pair is missing?
			}
		}
		else {
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		if( (model->geometries[g]->flags & RW::BSGeometry::ModuleMaterialColor) == RW::BSGeometry::ModuleMaterialColor) {
			auto col = mat.colour;
			if(col.a < 255 && queueTransparent) return false;
			if( object && object->type() == GameObject::Vehicle ) {
				auto vehicle = static_cast<VehicleObject*>(object);
				if( (mat.flags&Model::MTF_PrimaryColour) != 0 ) {
					oudata.colour = glm::vec4(vehicle->colourPrimary, 1.f);
				}
				else if( (mat.flags&Model::MTF_SecondaryColour) != 0 ) {
					oudata.colour = glm::vec4(vehicle->colourSecondary, 1.f);
				}
				else {
					oudata.colour = {col.r/255.f, col.g/255.f, col.b/255.f, col.a/255.f};
				}
			}
			else {
				oudata.colour = {col.r/255.f, col.g/255.f, col.b/255.f, col.a/255.f};
			}
		}

		oudata.diffuse = mat.diffuseIntensity;
		oudata.ambient = mat.ambientIntensity;
	}

	uploadUBO(uboObject, oudata);

	rendered++;

	glDrawElements(model->geometries[g]->dbuff.getFaceType(),
								subgeom.numIndices, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * subgeom.start));
	
	return true;
}

void GameRenderer::renderModel(Model* model, const glm::mat4& modelMatrix, GameObject* object, Animator *animator)
{
	renderFrame(model, model->frames[model->rootFrameIdx], modelMatrix, object);
}

void GameRenderer::renderPaths()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, debugTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    static std::vector<glm::vec3> carlines;
    static std::vector<glm::vec3> pedlines;

    GLint posAttrib = glGetAttribLocation(worldProgram, "position");
    GLint uniModel = glGetUniformLocation(worldProgram, "model");

    glBindVertexArray( vao );

    for( size_t n = 0; n < engine->aigraph.nodes.size(); ++n ) {
        auto start = engine->aigraph.nodes[n];
		
		if( start->type == AIGraphNode::Pedestrian ) {
			pedlines.push_back(start->position);
			if( start->external ) {
				pedlines.push_back(start->position+glm::vec3(0.f, 0.f, 2.f));
			}
			else {
				pedlines.push_back(start->position+glm::vec3(0.f, 0.f, 1.f));
			}
		}	
		else {
			carlines.push_back(start->position-glm::vec3(start->size / 2.f, 0.f, 0.f));
			carlines.push_back(start->position+glm::vec3(start->size / 2.f, 0.f, 0.f));
		}

		for( size_t c = 0; c < start->connections.size(); ++c ) {
			auto end = start->connections[c];
			
			if( start->type == AIGraphNode::Pedestrian ) {	
				pedlines.push_back(start->position + glm::vec3(0.f, 0.f, 1.f));
				pedlines.push_back(end->position + glm::vec3(0.f, 0.f, 1.f));
			}
			else {
				carlines.push_back(start->position + glm::vec3(0.f, 0.f, 1.f));
				carlines.push_back(end->position + glm::vec3(0.f, 0.f, 1.f));
			}
		}
    }

    glm::mat4 model;
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
    glEnableVertexAttribArray(posAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * carlines.size(), &(carlines[0]), GL_STREAM_DRAW);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    float img[] = {1.f, 0.f, 0.f};
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, 1, 1,
        0, GL_RGB, GL_FLOAT, img
    );

    glDrawArrays(GL_LINES, 0, carlines.size());

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * pedlines.size(), &(pedlines[0]), GL_STREAM_DRAW);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    float img2[] = {0.f, 1.f, 0.f};
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, 1, 1,
        0, GL_RGB, GL_FLOAT, img2
    );

    glDrawArrays(GL_LINES, 0, pedlines.size());

    pedlines.clear();
	carlines.clear();
    glBindVertexArray( 0 );
}
