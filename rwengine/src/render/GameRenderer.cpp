#include <render/GameRenderer.hpp>
#include <engine/GameWorld.hpp>
#include <engine/Animator.hpp>
#include <render/TextureAtlas.hpp>
#include <render/Model.hpp>

#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/PickupObject.hpp>
#include <objects/ProjectileObject.hpp>

#include <ai/CharacterController.hpp>
#include <data/ObjectData.hpp>
#include <items/InventoryItem.hpp>

#include <data/CutsceneData.hpp>
#include <data/Skeleton.hpp>
#include <objects/CutsceneObject.hpp>

#include <render/GameShaders.hpp>

#include <deque>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

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

/// @todo collapse all of these into "VertPNC" etc.
struct ParticleVert {
	static const AttributeList vertex_attributes() {
		return {
			{ATRS_Position, 2, sizeof(ParticleVert),  0ul},
			{ATRS_TexCoord, 2, sizeof(ParticleVert),  2ul * sizeof(float)},
			{ATRS_Colour, 3, sizeof(ParticleVert),  4ul * sizeof(float)}
		};
	}

	float x, y;
	float u, v;
	float r, g, b;
};

GeometryBuffer particleGeom;
DrawBuffer particleDraw;

std::vector<VertexP2> sspaceRect = {
	{-1.f, -1.f},
	{ 1.f, -1.f},
	{-1.f,  1.f},
	{ 1.f,  1.f},
};

GeometryBuffer ssRectGeom;
DrawBuffer ssRectDraw;

GameRenderer::GameRenderer(GameWorld* engine)
	: engine(engine), renderer(new OpenGLRenderer), _renderAlpha(0.f)
{
	engine->logInfo("[DRAW] " + renderer->getIDString());

	worldProg = renderer->createShader(
				GameShaders::WorldObject::VertexShader,
				GameShaders::WorldObject::FragmentShader);

	renderer->setUniformTexture(worldProg, "texture", 0);
	renderer->setProgramBlockBinding(worldProg, "SceneData", 1);
	renderer->setProgramBlockBinding(worldProg, "ObjectData", 2);

	particleProgram = compileProgram(GameShaders::WorldObject::VertexShader,
								  GameShaders::Particle::FragmentShader);

	/*uniTexture = glGetUniformLocation(particleProgram, "texture");
	ubiScene = glGetUniformBlockIndex(particleProgram, "SceneData");
	ubiObject = glGetUniformBlockIndex(particleProgram, "ObjectData");*/

	//glUniformBlockBinding(particleProgram, ubiScene, 1);
	//glUniformBlockBinding(particleProgram, ubiObject, 2);

	skyProg = renderer->createShader(
				GameShaders::Sky::VertexShader,
				GameShaders::Sky::FragmentShader);

	renderer->setProgramBlockBinding(skyProg, "SceneData", 1);

	waterProg = renderer->createShader(
				GameShaders::WaterHQ::VertexShader,
				GameShaders::WaterHQ::FragmentShader);

	renderer->setUniformTexture(waterProg, "texture", 0);

	renderer->setProgramBlockBinding(waterProg, "SceneData", 1);
	renderer->setProgramBlockBinding(waterProg, "ObjectData", 2);

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


	// Create the skydome

	size_t segments = skydomeSegments, rows = skydomeRows;

    float R = 1.f/(float)(rows-1);
    float S = 1.f/(float)(segments-1);
	std::vector<VertexP3> skydomeVerts;
	skydomeVerts.resize(rows * segments);
    for( size_t r = 0, i = 0; r < rows; ++r) {
        for( size_t s = 0; s < segments; ++s) {
			skydomeVerts[i++].position = glm::vec3(
                        cos(2.f * M_PI * s * S) * cos(M_PI_2 * r * R),
                        sin(2.f * M_PI * s * S) * cos(M_PI_2 * r * R),
                        sin(M_PI_2 * r * R)
                        );
		}
	}
	skyGbuff.uploadVertices(skydomeVerts);
	skyDbuff.addGeometry(&skyGbuff);
	skyDbuff.setFaceType(GL_TRIANGLES);

    glGenBuffers(1, &skydomeIBO);
	GLuint skydomeIndBuff[rows*segments*6];
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

	glBindVertexArray(0);

    glGenBuffers(1, &debugVBO);
    glGenTextures(1, &debugTex);
    glGenVertexArrays(1, &debugVAO);

	particleGeom.uploadVertices<ParticleVert>(
	{
					{ 0.5f, 0.5f, 1.f, 1.f, 1.f, 1.f, 1.f},
					{-0.5f, 0.5f, 0.f, 1.f, 1.f, 1.f, 1.f},
					{ 0.5f,-0.5f, 1.f, 0.f, 1.f, 1.f, 1.f},
					{-0.5f,-0.5f, 0.f, 0.f, 1.f, 1.f, 1.f}
	});
	particleDraw.addGeometry(&particleGeom);

	ssRectGeom.uploadVertices(sspaceRect);
	ssRectDraw.addGeometry(&ssRectGeom);

	ssRectProgram = compileProgram(GameShaders::ScreenSpaceRect::VertexShader,
								  GameShaders::ScreenSpaceRect::FragmentShader);

	ssRectTexture = glGetUniformLocation(ssRectProgram, "texture");
	ssRectColour = glGetUniformLocation(ssRectProgram, "colour");
	ssRectSize = glGetUniformLocation(ssRectProgram, "size");
	ssRectOffset = glGetUniformLocation(ssRectProgram, "offset");
}

float mix(uint8_t a, uint8_t b, float num)
{
	return a+(b-a)*num;
}

#define GL_PLS() \
{ auto errc = glGetError(); \
	if(errc != GL_NO_ERROR) std::cout << __LINE__ << ": " << errc << std::endl;\
}

void GameRenderer::renderWorld(const ViewCamera &camera, float alpha)
{
	_renderAlpha = alpha;

	// Store the input camera,
	_camera = camera;

	glBindVertexArray( vao );

	float tod = engine->state.hour + engine->state.minute/60.f;

	// Requires a float 0-24
	auto weatherID = static_cast<WeatherLoader::WeatherCondition>(engine->state.currentWeather * 24);
	auto weather = engine->gameData.weatherLoader.getWeatherData(weatherID, tod);

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

	_camera.frustum.near = engine->state.cameraNear;
	_camera.frustum.far = weather.farClipping;

	auto view = _camera.getView();
	auto proj = _camera.frustum.projection();

	Renderer::SceneUniformData sceneParams {
		proj,
		view,
		glm::vec4{ambient, 0.0f},
		glm::vec4{dynamic, 0.0f},
		glm::vec4(skyBottom, 1.f),
		glm::vec4(camera.position, 0.f),
		weather.fogStart,
		camera.frustum.far
	};

	renderer->setSceneParameters(sceneParams);
	
	renderer->clear(glm::vec4(skyBottom, 1.f));

	_camera.frustum.update(proj * view);
	
	rendered = culled = geoms = frames = 0;

	renderer->useProgram(worldProg);

	glActiveTexture(GL_TEXTURE0);

	for( GameObject* object : engine->objects ) {
		
		if( object->skeleton )
		{
			object->skeleton->interpolate(_renderAlpha);
		}
		
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
		case GameObject::Pickup:
			renderPickup(static_cast<PickupObject*>(object));
			break;
		case GameObject::Projectile:
			renderProjectile(static_cast<ProjectileObject*>(object));
			break;
		case GameObject::Cutscene:
			renderCutsceneObject(static_cast<CutsceneObject*>(object));
			break;
		default: break;
		}
	}
	
	// Draw anything that got queued.
	for(auto it = transparentDrawQueue.begin();
		it != transparentDrawQueue.end();
		++it)
	{
		renderer->draw(it->matrix, &it->model->geometries[it->g]->dbuff, it->dp);
	}
	transparentDrawQueue.clear();

	// Draw the water.
	renderer->useProgram( waterProg );

	float blockLQSize = WATER_WORLD_SIZE/WATER_LQ_DATA_SIZE;
	float blockHQSize = WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE;

	glm::vec2 waterOffset { -WATER_WORLD_SIZE/2.f, -WATER_WORLD_SIZE/2.f };
	auto waterTex = engine->gameData.textures[{"water_old",""}];
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex.texName);

	auto camposFlat = glm::vec2(camera.position);

	Renderer::DrawParameters wdp;
	wdp.start = 0;
	wdp.count = waterHQVerts.size();
	wdp.texture = waterTex.texName;

	renderer->useProgram(waterProg);
	renderer->setSceneParameters(sceneParams);

	// Draw High detail water
	renderer->setUniform(waterProg, "size", blockHQSize);
	renderer->setUniform(waterProg, "time", engine->gameTime);
	renderer->setUniform(waterProg, "waveParams", glm::vec2(WATER_SCALE, WATER_HEIGHT));

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

			glm::mat4 m;
			m = glm::translate(m, glm::vec3(waterWS, h));

			renderer->drawArrays(m, &waterHQDraw, wdp);
		}
	}


	wdp.count = waterLQVerts.size();
	renderer->setUniform(waterProg, "size", blockLQSize);
	renderer->setUniform(waterProg, "waveParams", glm::vec2(0.f));

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

			glm::mat4 m;
			m = glm::translate(m, glm::vec3(waterWS, h));

			renderer->drawArrays(m, &waterLQDraw, wdp);
		}
	}

	glBindVertexArray( vao );

	Renderer::DrawParameters dp;
	dp.start = 0;
	dp.count = skydomeSegments * skydomeRows * 6;

	renderer->useProgram(skyProg);
	renderer->setUniform(skyProg, "TopColor", glm::vec4(skyTop, 1.f));
	renderer->setUniform(skyProg, "BottomColor", glm::vec4(skyBottom, 1.f));

	renderer->draw(glm::mat4(), &skyDbuff, dp);

	renderParticles();

	glActiveTexture(GL_TEXTURE0);

	glDisable(GL_DEPTH_TEST);

	GLuint splashTexName = 0;
	auto fc = engine->state.fadeColour;
	if((fc.r + fc.g + fc.b) == 0 && engine->state.currentSplash.size() > 0) {
		std::pair<std::string, std::string> splashName(engine->state.currentSplash, "");
		auto splshfnd = engine->gameData.textures.find(splashName);
		if( splshfnd != engine->gameData.textures.end() ) {
			splashTexName = splshfnd->second.texName;
		}
	}

	if( engine->state.currentCutscene && splashTexName != 0 ) {
		renderLetterbox();
	}

	float fadeTimer = engine->gameTime - engine->state.fadeStart;
	if( fadeTimer <= engine->state.fadeTime || !engine->state.fadeOut ) {
		glUseProgram(ssRectProgram);
		glUniform2f(ssRectOffset, 0.f, 0.f);
		glUniform2f(ssRectSize, 1.f, 1.f);

		glUniform1i(ssRectTexture, 0);

		if(splashTexName != 0) {
			glBindTexture(GL_TEXTURE_2D, splashTexName);
			fc = glm::u16vec3(0, 0, 0);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		float fadeFrac = 0.f;
		if( engine->state.fadeTime > 0.f ) {
			fadeFrac = std::min(fadeTimer / engine->state.fadeTime, 1.f);
		}

		float a = engine->state.fadeOut ? 1.f - fadeFrac : fadeFrac;

		glm::vec4 fadeNormed(fc.r / 255.f, fc.g/ 255.f, fc.b/ 255.f, a);

		glUniform4fv(ssRectColour, 1, glm::value_ptr(fadeNormed));

		glBindVertexArray( ssRectDraw.getVAOName() );
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	if( engine->state.currentCutscene && splashTexName == 0 ) {
		renderLetterbox();
	}

	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray( 0 );
}

void GameRenderer::renderPedestrian(CharacterObject *pedestrian)
{
	glm::mat4 matrixModel = pedestrian->getTimeAdjustedTransform( _renderAlpha );

	if(!pedestrian->model->model) return;
	
	// Apply the inverse of the root transform from the current animation.
	if( pedestrian->isAnimationFixed() )
	{
		auto rtranslate = pedestrian->animator->getTimeTranslation(_renderAlpha);
		matrixModel = glm::translate(matrixModel, -rtranslate);
	}

	auto root = pedestrian->model->model->frames[0];
	
	renderFrame(pedestrian->model->model, root->getChildren()[0], matrixModel, pedestrian, 1.f, pedestrian->animator);

	if(pedestrian->getActiveItem()) {
		auto handFrame = pedestrian->model->model->findFrame("srhand");
		glm::mat4 localMatrix;
		if( handFrame ) {
			while( handFrame->getParent() ) {
				localMatrix = pedestrian->skeleton->getMatrix(handFrame->getIndex()) * localMatrix;
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

	glm::mat4 matrixModel = vehicle->getTimeAdjustedTransform( _renderAlpha );

	renderModel(vehicle->model->model, matrixModel, vehicle);

	// Draw wheels n' stuff
	for( size_t w = 0; w < vehicle->info->wheels.size(); ++w) {
		auto woi = engine->findObjectType<ObjectData>(vehicle->vehicle->wheelModelID);
		if( woi ) {
			Model* wheelModel = engine->gameData.models["wheels"]->model;
			auto& wi = vehicle->physVehicle->getWheelInfo(w);
			if( wheelModel ) {
				// Construct our own matrix so we can use the local transform
				vehicle->physVehicle->updateWheelTransform(w, false);
				/// @todo migrate this into Vehicle physics tick so we can interpolate old -> new

				glm::mat4 wheelM ( matrixModel );

				auto up = -wi.m_wheelDirectionCS;
				auto right = wi.m_wheelAxleCS;
				auto fwd = up.cross(right);
				btQuaternion steerQ(up, wi.m_steering);
				btQuaternion rollQ(right, -wi.m_rotation);

				btMatrix3x3 basis(
						right[0], fwd[0], up[0],
						right[1], fwd[1], up[1],
						right[2], fwd[2], up[2]
						);


				btTransform t;
				t.setBasis(btMatrix3x3(steerQ) * btMatrix3x3(rollQ) * basis);
				t.setOrigin(wi.m_chassisConnectionPointCS + wi.m_wheelDirectionCS * wi.m_raycastInfo.m_suspensionLength);

				t.getOpenGLMatrix(glm::value_ptr(wheelM));
				wheelM = matrixModel * wheelM;

				wheelM = glm::scale(wheelM, glm::vec3(vehicle->vehicle->wheelScale));
				if(wi.m_chassisConnectionPointCS.x() < 0.f) {
					wheelM = glm::scale(wheelM, glm::vec3(-1.f, 1.f, 1.f));
				}

				renderWheel(wheelModel, wheelM, woi->modelName);
			}
			else {
				std::cout << "Wheel model " << woi->modelName << " not loaded" << std::endl;
			}
		}
	}
}

void GameRenderer::renderInstance(InstanceObject *instance)
{
	if(instance->object && instance->object->timeOn != instance->object->timeOff) {
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
		instance->body->body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(matrixModel));
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
		mindist = std::min(mindist, glm::length((glm::vec3(matrixModel[3])+bounds.center) - _camera.position) - bounds.radius);
	}

	Model* model = nullptr;
	ModelFrame* frame = nullptr;

	// These are used to gracefully fade out things that are just out of view distance.
	Model* fadingModel = nullptr;
	ModelFrame* fadingFrame = nullptr;
	float opacity = 0.f;

	if( instance->object->numClumps == 1 ) {
		// Object consists of a single clump.
		if( mindist > instance->object->drawDistance[0] ) {
			// Check for LOD instances
			if ( instance->LODinstance ) {
				if( mindist > instance->LODinstance->object->drawDistance[0] ) {
					culled++;
					return;
				}
				else if (instance->LODinstance->model->model) {
					model = instance->LODinstance->model->model;

					fadingModel = instance->model->model;
					opacity = (mindist) / instance->object->drawDistance[0];
				}
			}
			else {
				fadingModel = instance->model->model;
				opacity = (mindist) / instance->object->drawDistance[0];
			}
		}
		else if (! instance->object->LOD ) {
			model = instance->model->model;
			opacity = (mindist) / instance->object->drawDistance[0];
		}
	}
	else {
		if( mindist > instance->object->drawDistance[1] ) {
			culled++;
			return;
		}

		auto root = instance->model->model->frames[0];
		int lodInd = 1;
		if( mindist > instance->object->drawDistance[0] ) {
			lodInd = 2;
		}
		auto LODindex = root->getChildren().size() - lodInd;
		auto f = root->getChildren()[LODindex];
		model = instance->model->model;
		frame = f;

		if( lodInd == 2 ) {
			fadingModel = model;
			fadingFrame = root->getChildren()[LODindex+1];
			opacity = (mindist) / instance->object->drawDistance[0];
		}
	}

	if( model ) {
		frame = frame ? frame : model->frames[0];
		renderFrame(model, frame, matrixModel * glm::inverse(frame->getTransform()), nullptr, 1.f);
	}
	if( fadingModel ) {
		// opacity is the distance over the culling limit,
		opacity = 2.0f - opacity;
		if(opacity > 0.f) {
			fadingFrame = fadingFrame ? fadingFrame : fadingModel->frames[0];
			renderFrame(fadingModel, fadingFrame, matrixModel * glm::inverse(fadingFrame->getTransform()), nullptr, opacity);
		}
	}
}

void GameRenderer::renderPickup(PickupObject *pickup)
{
	if( ! pickup->isEnabled() ) return;

	glm::mat4 modelMatrix = glm::translate(glm::mat4(), pickup->getPosition());
	modelMatrix = glm::rotate(modelMatrix, engine->gameTime, glm::vec3(0.f, 0.f, 1.f));

	auto odata = engine->findObjectType<ObjectData>(pickup->getModelID());
	auto weapons = engine->gameData.models["weapons"];
	if( weapons && weapons->model && odata ) {
		auto itemModel = weapons->model->findFrame(odata->modelName + "_l0");
		auto matrix = glm::inverse(itemModel->getTransform());
		if(itemModel) {
			renderFrame(weapons->model, itemModel, modelMatrix * matrix, nullptr, 1.f);
		}
		else {
			std::cerr << "weapons.dff missing frame " << odata->modelName << std::endl;
		}
	}
	else {
		std::cerr << "weapons.dff not loaded (" << pickup->getModelID() << ")" << std::endl;
	}
}

void GameRenderer::renderCutsceneObject(CutsceneObject *cutscene)
{
	if(!engine->state.currentCutscene) return;

	if(!cutscene->model->model)
	{
		return;
	}

	glm::mat4 matrixModel;

	if( cutscene->getParentActor() ) {
		matrixModel = glm::translate(matrixModel, engine->state.currentCutscene->meta.sceneOffset + glm::vec3(0.f, 0.f, 1.f));
		//matrixModel = cutscene->getParentActor()->getTimeAdjustedTransform(_renderAlpha);
		//matrixModel = glm::translate(matrixModel, glm::vec3(0.f, 0.f, 1.f));
		glm::mat4 localMatrix;
		auto boneframe = cutscene->getParentFrame();
		while( boneframe ) {
			localMatrix = cutscene->getParentActor()->skeleton->getMatrix(boneframe->getIndex()) * localMatrix;
			boneframe = boneframe->getParent();
		}
		matrixModel = matrixModel * localMatrix;
	}
	else {
		matrixModel = glm::translate(matrixModel, engine->state.currentCutscene->meta.sceneOffset + glm::vec3(0.f, 0.f, 1.f));
	}

	float mindist = 100000.f;
	for (size_t g = 0; g < cutscene->model->model->geometries.size(); g++)
	{
		RW::BSGeometryBounds& bounds = cutscene->model->model->geometries[g]->geometryBounds;
		mindist = std::min(mindist, glm::length((glm::vec3(matrixModel[3])+bounds.center) - _camera.position) - bounds.radius);
	}

	if( cutscene->getParentActor() ) {
		glm::mat4 align;
		/// @todo figure out where this 90 degree offset is coming from.
		align = glm::rotate(align, glm::half_pi<float>(), {0.f, 1.f, 0.f});
		renderModel(cutscene->model->model, matrixModel * align, cutscene);
	}
	else {
		renderModel(cutscene->model->model, matrixModel, cutscene);
	}
}

void GameRenderer::renderProjectile(ProjectileObject *projectile)
{
	glm::mat4 modelMatrix = projectile->getTimeAdjustedTransform(_renderAlpha);

	auto odata = engine->findObjectType<ObjectData>(projectile->getProjectileInfo().weapon->modelID);
	auto weapons = engine->gameData.models["weapons"];
	if( weapons && weapons->model ) {
		auto itemModel = weapons->model->findFrame(odata->modelName + "_l0");
		auto matrix = glm::inverse(itemModel->getTransform());
		if(itemModel) {
			renderFrame(weapons->model, itemModel, modelMatrix * matrix, nullptr, 1.f);
		}
		else {
			std::cerr << "weapons.dff missing frame " << odata->modelName << std::endl;
		}
	}
	else {
		std::cerr << "weapons.dff not loaded" << std::endl;
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
			if(! _camera.frustum.intersects(bounds.center + glm::vec3(matrix[3]), bounds.radius)) {
				culled++;
				continue;
			}

			renderGeometry(model, g, matrix, 1.f);
		}
		break;
	}
}

void GameRenderer::renderItem(InventoryItem *item, const glm::mat4 &modelMatrix)
{
	// srhand
	std::shared_ptr<ObjectData> odata = engine->findObjectType<ObjectData>(item->getModelID());
	auto weapons = engine->gameData.models["weapons"];
	if( weapons && weapons->model ) {
		auto itemModel = weapons->model->findFrame(odata->modelName + "_l0");
		auto matrix = glm::inverse(itemModel->getTransform());
		if(itemModel) {
			renderFrame(weapons->model, itemModel, modelMatrix * matrix, nullptr, 1.f);
		}
		else {
			std::cerr << "weapons.dff missing frame " << odata->modelName << std::endl;
		}
	}
	else {
		std::cerr << "weapons.dff not loaded" << std::endl;
	}
}

void GameRenderer::renderGeometry(Model* model, size_t g, const glm::mat4& modelMatrix, float opacity, GameObject* object)
{
	geoms++;

	for(size_t sg = 0; sg < model->geometries[g]->subgeom.size(); ++sg)
	{
		Model::SubGeometry& subgeom = model->geometries[g]->subgeom[sg];

		bool abortTransparent = false;

		Renderer::DrawParameters dp;

		dp.colour = {255, 255, 255, 255};
		dp.count = subgeom.numIndices;
		dp.start = subgeom.start;
		dp.texture = 0;

		if (model->geometries[g]->materials.size() > subgeom.material) {
			Model::Material& mat = model->geometries[g]->materials[subgeom.material];

			if(mat.textures.size() > 0 ) {
				auto& tC = mat.textures[0].name;
				auto& tA = mat.textures[0].alphaName;
				auto t = engine->gameData.textures.find({tC, tA});
				if(t != engine->gameData.textures.end()) {
					TextureInfo& tex = t->second;
					if(tex.transparent) {
						abortTransparent = true;
					}
					dp.texture = tex.texName;
				}
			}

			if( (model->geometries[g]->flags & RW::BSGeometry::ModuleMaterialColor) == RW::BSGeometry::ModuleMaterialColor) {
				dp.colour = mat.colour;

				if( object && object->type() == GameObject::Vehicle ) {
					auto vehicle = static_cast<VehicleObject*>(object);
					if( dp.colour.r == 60 && dp.colour.g == 255 && dp.colour.b == 0 ) {
						dp.colour = glm::u8vec4(vehicle->colourPrimary, 255);
					}
					else if( dp.colour.r == 255 && dp.colour.g == 0 && dp.colour.b == 175 ) {
						dp.colour = glm::u8vec4(vehicle->colourSecondary, 255);
					}
				}
			}

			dp.colour.a *= opacity;

			if( dp.colour.a < 255 ) {
				abortTransparent = true;
			}

			dp.diffuse = mat.diffuseIntensity;
			dp.ambient = mat.ambientIntensity;
		}

		rendered++;

		if( abortTransparent ) {
			transparentDrawQueue.push_back(
				{model, g, sg, modelMatrix, dp, object}
			);
		}
		else {
			renderer->draw(modelMatrix, &model->geometries[g]->dbuff, dp);
		}
	}
}

void GameRenderer::renderParticles()
{
	_particles.erase( std::remove_if(_particles.begin(), _particles.end(),
				   [&](FXParticle& p) {
		if ( ( engine->gameTime - p.starttime ) > p.lifetime ) {
			return true;
		}
		float t = engine->gameTime - p.starttime;
		p._currentPosition = p.position + (p.direction * p.velocity) * t;
		return false;
	}), _particles.end() );

	glUseProgram( particleProgram );
	glBindVertexArray( particleDraw.getVAOName() );

	auto cpos = _camera.position;
	auto cfwd = glm::normalize(glm::inverse(_camera.rotation) * glm::vec3(0.f, 1.f, 0.f));

	std::sort( _particles.begin(), _particles.end(),
			   [&](const FXParticle& a, const FXParticle& b) {
		return glm::distance( a._currentPosition, cpos ) > glm::distance( b._currentPosition, cpos );
	});

	for(FXParticle& part : _particles) {
		glBindTexture(GL_TEXTURE_2D, part.texture);
		auto& p = part._currentPosition;

		glm::mat4 m(1.f);

		// Figure the direction to the camera center.
		auto amp = cpos - p;
		glm::vec3 ptc = part.up;

		if( part.orientation == FXParticle::UpCamera ) {
			ptc = glm::normalize(amp - (glm::dot(amp, cfwd))*cfwd);
		}
		else if( part.orientation == FXParticle::Camera ) {
			ptc = amp;
		}

		glm::vec3 f = glm::normalize(part.direction);
		glm::vec3 s = glm::cross(f, glm::normalize(ptc));
		glm::vec3 u	= glm::cross(s, f);
		m[0][0] = s.x;
		m[1][0] = s.y;
		m[2][0] = s.z;
		m[0][1] =-f.x;
		m[1][1] =-f.y;
		m[2][1] =-f.z;
		m[0][2] = u.x;
		m[1][2] = u.y;
		m[2][2] = u.z;
		m[3][0] =-glm::dot(s, p);
		m[3][1] = glm::dot(f, p);
		m[3][2] =-glm::dot(u, p);

		m = glm::scale(glm::inverse(m), glm::vec3(part.size, 1.f));
		/*uploadUBO<ObjectUniformData>(
							uboObject, {
								m,
								part.colour,
								1.f, 1.f, 1.f
							});*/

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void GameRenderer::drawOnScreenText()
{
	/// @ TODO
}

bool GameRenderer::renderFrame(Model* m, ModelFrame* f, const glm::mat4& matrix, GameObject* object, float opacity, bool queueTransparent)
{
	frames++;
	auto localmatrix = matrix;
	bool vis = true;

	if(object && object->skeleton) {
		// Skeleton is loaded with the correct matrix via Animator.
		localmatrix *= object->skeleton->getMatrix(f);

		vis = object->skeleton->getData(f->getIndex()).enabled;
	}
	else {
		localmatrix *= f->getTransform();
	}

	if( vis ) {
		for(size_t g : f->getGeometries()) {
			if( !object || !object->animator )
			{
				RW::BSGeometryBounds& bounds = m->geometries[g]->geometryBounds;
				
				glm::vec3 boundpos = bounds.center + glm::vec3(localmatrix[3]);
				if(! _camera.frustum.intersects(boundpos, bounds.radius)) {
					culled++;
					continue;
				}
			}

			renderGeometry(m, g, localmatrix, opacity, object);
		}
	}
	
	for(ModelFrame* c : f->getChildren()) {
		renderFrame(m, c, localmatrix, object, queueTransparent);
	}
	return true;
}

void GameRenderer::renderModel(Model* model, const glm::mat4& modelMatrix, GameObject* object, Animator *animator)
{
	renderFrame(model, model->frames[model->rootFrameIdx], modelMatrix, object, 1.f);
}

void GameRenderer::renderPaths()
{
	/*glActiveTexture(GL_TEXTURE0);
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
	glBindVertexArray( 0 );*/
}

void GameRenderer::renderLetterbox()
{
	glUseProgram(ssRectProgram);
	const float cinematicExperienceSize = 0.15f;
	glUniform2f(ssRectOffset, 0.f, -1.f * (1.f - cinematicExperienceSize));
	glUniform2f(ssRectSize, 1.f, cinematicExperienceSize);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(ssRectTexture, 0);
	glUniform4f(ssRectColour, 0.f, 0.f, 0.f, 1.f);

	glBindVertexArray( ssRectDraw.getVAOName() );
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glUniform2f(ssRectOffset, 0.f, 1.f * (1.f - cinematicExperienceSize));
	glUniform2f(ssRectSize, 1.f, cinematicExperienceSize);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void GameRenderer::addParticle(const FXParticle &particle)
{
	_particles.push_back(particle);
}
