#include <render/GameRenderer.hpp>
#include <engine/GameWorld.hpp>
#include <engine/Animator.hpp>
#include <engine/GameState.hpp>
#include <data/Model.hpp>

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
#include <render/ObjectRenderer.hpp>

#include <render/GameShaders.hpp>
#include <core/Logger.hpp>

#include <deque>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <core/Profiler.hpp>

const size_t skydomeSegments = 8, skydomeRows = 10;
constexpr uint32_t kMissingTextureBytes[] = {
	0xFF0000FF, 0xFFFF00FF, 0xFF0000FF, 0xFFFF00FF,
	0xFFFF00FF, 0xFF0000FF, 0xFFFF00FF, 0xFF0000FF,
	0xFF0000FF, 0xFFFF00FF, 0xFF0000FF, 0xFFFF00FF,
	0xFFFF00FF, 0xFF0000FF, 0xFFFF00FF, 0xFF0000FF,
};

struct WaterVertex {
	static const AttributeList vertex_attributes() {
		return {
			{ATRS_Position, 2, sizeof(WaterVertex),  0ul}
		};
	}

	float x, y;
};

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

GameRenderer::GameRenderer(Logger* log, GameData* _data)
	: data(_data)
	, logger(log)
	, renderer(new OpenGLRenderer)
	, _renderAlpha(0.f)
	, _renderWorld(nullptr)
	, cullOverride(false)
	, map(renderer, _data)
	, water(this)
	, text(this)
{
	logger->info("Renderer", renderer->getIDString());

	worldProg = renderer->createShader(
				GameShaders::WorldObject::VertexShader,
				GameShaders::WorldObject::FragmentShader);

	renderer->setUniformTexture(worldProg, "texture", 0);
	renderer->setProgramBlockBinding(worldProg, "SceneData", 1);
	renderer->setProgramBlockBinding(worldProg, "ObjectData", 2);
	
	particleProg = renderer->createShader(
		GameShaders::WorldObject::VertexShader,
		GameShaders::Particle::FragmentShader);
	
	renderer->setUniformTexture(particleProg, "texture", 0);
	renderer->setProgramBlockBinding(particleProg, "SceneData", 1);
	renderer->setProgramBlockBinding(particleProg, "ObjectData", 2);

	skyProg = renderer->createShader(
		GameShaders::Sky::VertexShader,
		GameShaders::Sky::FragmentShader);

	renderer->setProgramBlockBinding(skyProg, "SceneData", 1);

	postProg = renderer->createShader(
		GameShaders::DefaultPostProcess::VertexShader,
		GameShaders::DefaultPostProcess::FragmentShader);

	glGenVertexArrays( 1, &vao );

	glGenTextures(1, &m_missingTexture);
	glBindTexture(GL_TEXTURE_2D, m_missingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, kMissingTextureBytes);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &framebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
	glGenTextures(2, fbTextures);
	
	glBindTexture(GL_TEXTURE_2D, fbTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, fbTextures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 128, 128, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTextures[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fbTextures[1], 0);
	
	// Give water renderer the data texture
	water.setDataTexture(1, fbTextures[1]);
	
	glGenRenderbuffers(1, fbRenderBuffers);
	glBindRenderbuffer(GL_RENDERBUFFER, fbRenderBuffers[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 128, 128);
	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbRenderBuffers[0]
	);
	
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
	std::vector<GLuint> skydomeIndBuff;
	skydomeIndBuff.resize(rows*segments*6);
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * skydomeIndBuff.size(), skydomeIndBuff.data(), GL_STATIC_DRAW);

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
	particleDraw.setFaceType(GL_TRIANGLE_STRIP);

	ssRectGeom.uploadVertices(sspaceRect);
	ssRectDraw.addGeometry(&ssRectGeom);
	ssRectDraw.setFaceType(GL_TRIANGLE_STRIP);

	ssRectProgram = compileProgram(GameShaders::ScreenSpaceRect::VertexShader,
								  GameShaders::ScreenSpaceRect::FragmentShader);

	ssRectTexture = glGetUniformLocation(ssRectProgram, "texture");
	ssRectColour = glGetUniformLocation(ssRectProgram, "colour");
	ssRectSize = glGetUniformLocation(ssRectProgram, "size");
	ssRectOffset = glGetUniformLocation(ssRectProgram, "offset");
	
	const static int cylsegments = 16;
	std::vector<Model::GeometryVertex> cylverts;
	for(int s = 0; s < cylsegments; ++s)
	{
		float theta = (2.f*glm::pi<float>()/cylsegments) * (s+0);
		float gamma = (2.f*glm::pi<float>()/cylsegments) * (s+1);
		glm::vec2 p0( glm::sin(theta), glm::cos(theta) );
		glm::vec2 p1( glm::sin(gamma), glm::cos(gamma) );
		
		p0 *= 0.5f;
		p1 *= 0.5f;
		
		cylverts.push_back({glm::vec3(p0, 2.f), glm::vec3(), glm::vec2(0.45f,0.6f), glm::u8vec4(255, 255, 255, 50)});
		cylverts.push_back({glm::vec3(p0,-1.f), glm::vec3(), glm::vec2(0.45f,0.4f), glm::u8vec4(255, 255, 255, 150)});
		cylverts.push_back({glm::vec3(p1, 2.f), glm::vec3(), glm::vec2(0.55f,0.6f), glm::u8vec4(255, 255, 255, 50)});
		
		cylverts.push_back({glm::vec3(p0,-1.f), glm::vec3(), glm::vec2(0.45f,0.4f), glm::u8vec4(255, 255, 255, 150)});
		cylverts.push_back({glm::vec3(p1,-1.f), glm::vec3(), glm::vec2(0.55f,0.4f), glm::u8vec4(255, 255, 255, 150)});
		cylverts.push_back({glm::vec3(p1, 2.f), glm::vec3(), glm::vec2(0.55f,0.6f), glm::u8vec4(255, 255, 255, 50)});
	}
	cylinderGeometry.uploadVertices<Model::GeometryVertex>(cylverts);
	cylinderBuffer.addGeometry(&cylinderGeometry);
	cylinderBuffer.setFaceType(GL_TRIANGLES);
}

GameRenderer::~GameRenderer()
{
	glDeleteFramebuffers(1, &framebufferName);
}

float mix(uint8_t a, uint8_t b, float num)
{
	return a+(b-a)*num;
}

void GameRenderer::setupRender()
{
	// Set the viewport
	const glm::ivec2& vp = getRenderer()->getViewport();
	glViewport(0, 0, vp.x, vp.y);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
}

void GameRenderer::renderWorld(GameWorld* world, const ViewCamera &camera, float alpha)
{
	_renderAlpha = alpha;
	_renderWorld = world;

	// Store the input camera,
	_camera = camera;

	setupRender();

	glBindVertexArray( vao );

	float tod = world->getHour() + world->getMinute()/60.f;

	// Requires a float 0-24
	auto weatherID = static_cast<WeatherLoader::WeatherCondition>(world->state->basic.nextWeather * 24);
	auto weather = world->data->weatherLoader.getWeatherData(weatherID, tod);

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

	_camera.frustum.near = world->state->cameraNear;
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
		_camera.frustum.far
	};

	renderer->setSceneParameters(sceneParams);
	
	renderer->clear(glm::vec4(skyBottom, 1.f));

	_camera.frustum.update(proj * view);
	if (cullOverride)
	{
		cullingCamera.frustum.update(
					cullingCamera.frustum.projection() * cullingCamera.getView());
	}
	
	culled = 0;

	renderer->useProgram(worldProg);

	//===============================================================
	//	Render List Construction
	//---------------------------------------------------------------

	RW_PROFILE_BEGIN("RenderList");

	// This is sequential at the moment, it should be easy to make it
	// run in parallel with a good threading system.
	RenderList renderList;
	// Naive optimisation, assume 10% hitrate
	renderList.reserve(world->allObjects.size() * 0.5f);

	RW_PROFILE_BEGIN("Build");

	ObjectRenderer objectRenderer(_renderWorld,
					  (cullOverride ? cullingCamera : _camera),
					  _renderAlpha,
					  getMissingTexture());

	// World Objects
	for (auto object : world->allObjects) {
		objectRenderer.buildRenderList(object, renderList);
	}
	RW_PROFILE_END();

	renderer->pushDebugGroup("Objects");
	renderer->pushDebugGroup("RenderList");

	// Also parallelizable
	RW_PROFILE_BEGIN("Sort");
	std::sort(renderList.begin(), renderList.end(),
			  [](const Renderer::RenderInstruction& a,
				 const Renderer::RenderInstruction&b) {
					return a.sortKey < b.sortKey;
				});
	RW_PROFILE_END();

	RW_PROFILE_BEGIN("Draw");
	renderer->drawBatched(renderList);
	RW_PROFILE_END();

	renderer->popDebugGroup();
	profObjects = renderer->popDebugGroup();

	RW_PROFILE_END();

	// Render arrows above anything that isn't radar only (or hidden)
	ModelRef& arrowModel = world->data->models["arrow"];
	if( arrowModel && arrowModel->resource )
	{
		auto arrowTex = world->data->textures[{"copblue",""}];
		auto arrowFrame = arrowModel->resource->findFrame( "arrow" );
		for( auto& blip : world->state->radarBlips )
		{
			if( blip.second.display == BlipData::Show )
			{
				glm::mat4 model;

				if( blip.second.target > 0 )
				{
					auto object = world->getBlipTarget(blip.second);
					if( object )
					{
						model = object->getTimeAdjustedTransform( _renderAlpha );
					}
				}
				else
				{
					model = glm::translate( model, blip.second.coord );
				}

				float a = world->getGameTime() * glm::pi<float>();
				model = glm::translate( model, glm::vec3(0.f, 0.f, 2.5f + glm::sin( a ) * 0.5f) );
				model = glm::rotate( model, a, glm::vec3(0.f, 0.f, 1.f) );
				model = glm::scale( model, glm::vec3(1.5f, 1.5f, 1.5f) );

				Renderer::DrawParameters dp;
				dp.textures = {arrowTex->getName()};
				dp.ambient = 1.f;
				dp.colour = glm::u8vec4(255, 255, 255, 255);

				auto geom = arrowModel->resource->geometries[arrowFrame->getGeometries()[0]];
				Model::SubGeometry& sg = geom->subgeom[0];

				dp.start = sg.start;
				dp.count = sg.numIndices;
				dp.diffuse = 1.f;

				renderer->draw( model, &geom->dbuff, dp );
			}
		}
	}

	// Draw goal indicators
	glDepthMask(GL_FALSE);
	renderer->useProgram( particleProg );
	for(auto& i : world->getAreaIndicators())
	{
		renderAreaIndicator( &i );
	}
	glDepthMask(GL_TRUE);

	renderer->pushDebugGroup("Water");

	water.render(this, world);

	profWater = renderer->popDebugGroup();

	renderer->pushDebugGroup("Sky");

	glBindVertexArray( vao );

	Renderer::DrawParameters dp;
	dp.start = 0;
	dp.count = skydomeSegments * skydomeRows * 6;

	renderer->useProgram(skyProg);
	renderer->setUniform(skyProg, "TopColor", glm::vec4(skyTop, 1.f));
	renderer->setUniform(skyProg, "BottomColor", glm::vec4(skyBottom, 1.f));

	renderer->draw(glm::mat4(), &skyDbuff, dp);

	profSky = renderer->popDebugGroup();

	renderer->pushDebugGroup("Effects");
	renderEffects(world);
	profEffects = renderer->popDebugGroup();

	glDisable(GL_DEPTH_TEST);

	GLuint splashTexName = 0;
	auto fc = world->state->fadeColour;
	if((fc.r + fc.g + fc.b) == 0 && world->state->currentSplash.size() > 0) {
		auto splash = world->data->findTexture(world->state->currentSplash);
		if ( splash )
		{
			splashTexName = splash->getName();
		}
	}

	if( (world->state->isCinematic || world->state->currentCutscene ) && splashTexName != 0 ) {
		renderLetterbox();
	}

	float fadeTimer = world->getGameTime() - world->state->fadeStart;
	if( fadeTimer < world->state->fadeTime || !world->state->fadeOut ) {
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
		if( world->state->fadeTime > 0.f ) {
			fadeFrac = std::min(fadeTimer / world->state->fadeTime, 1.f);
		}

		float a = world->state->fadeOut ? 1.f - fadeFrac : fadeFrac;

		glm::vec4 fadeNormed(fc.r / 255.f, fc.g/ 255.f, fc.b/ 255.f, a);

		glUniform4fv(ssRectColour, 1, glm::value_ptr(fadeNormed));

		glBindVertexArray( ssRectDraw.getVAOName() );
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	if( (world->state->isCinematic || world->state->currentCutscene ) && splashTexName == 0 ) {
		renderLetterbox();
	}

	renderPostProcess();

	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray( 0 );
}

void GameRenderer::renderPostProcess()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glStencilMask(0xFF);
	glClearStencil(0x00);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	renderer->useProgram(postProg);
	
	Renderer::DrawParameters wdp;
	wdp.start = 0;
	wdp.count = ssRectGeom.getCount();
	wdp.textures = {fbTextures[0]};
	
	renderer->drawArrays(glm::mat4(), &ssRectDraw, wdp);
}

void GameRenderer::renderGeometry(Model* model, size_t g, const glm::mat4& modelMatrix, float opacity, GameObject* object)
{
	for(size_t sg = 0; sg < model->geometries[g]->subgeom.size(); ++sg)
	{
		Model::SubGeometry& subgeom = model->geometries[g]->subgeom[sg];

		Renderer::DrawParameters dp;

		dp.colour = {255, 255, 255, 255};
		dp.count = subgeom.numIndices;
		dp.start = subgeom.start;
		dp.textures = {0};

		if (model->geometries[g]->materials.size() > subgeom.material) {
			Model::Material& mat = model->geometries[g]->materials[subgeom.material];

			if(mat.textures.size() > 0 ) {
				auto tex = mat.textures[0].texture;
				if( ! tex )
				{
					auto& tC = mat.textures[0].name;
					auto& tA = mat.textures[0].alphaName;
					tex = data->findTexture(tC, tA);
					if( ! tex )
					{
						//logger->warning("Renderer", "Missing texture: " + tC + " " + tA);
					}
					mat.textures[0].texture = tex;
				}
				if( tex )
				{
					dp.textures = {tex->getName()};
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

			dp.diffuse = mat.diffuseIntensity;
			dp.ambient = mat.ambientIntensity;
		}

		renderer->draw(modelMatrix, &model->geometries[g]->dbuff, dp);
	}
}

#define GOAL_RINGS 3
void GameRenderer::renderAreaIndicator(const AreaIndicatorInfo* info)
{
	glm::mat4 m(1.f);
	m = glm::translate(m, info->position);
	glm::vec3 scale = info->radius + 0.15f * glm::sin(_renderWorld->getGameTime() * 5.f);
	
	Renderer::DrawParameters dp;
	dp.textures = {data->findTexture("cloud1")->getName()};
	dp.ambient = 1.f;
	dp.colour = glm::u8vec4(50, 100, 255, 128);
	dp.start = 0;
	dp.count = cylinderGeometry.getCount();
	dp.diffuse = 1.f;
	
	for(int i = 0; i < GOAL_RINGS; i++)
	{
		glm::mat4 mt = m;
		glm::vec3 final = scale * glm::pow(0.9f, i + 1.0f);
		mt = glm::scale(mt, glm::vec3(final.x, final.y, 1.0f + i * 0.1f));
		int reverse = (i % 2 ? 1 : -1);
		mt = glm::rotate(mt, reverse * _renderWorld->getGameTime() * 0.5f, glm::vec3(0.f, 0.f, 1.f) );
		
		renderer->drawArrays(mt, &cylinderBuffer, dp);
	}
}

void GameRenderer::renderEffects(GameWorld* world)
{
	renderer->useProgram( particleProg );

	auto cpos = _camera.position;
	auto cfwd = glm::normalize(glm::inverse(_camera.rotation) * glm::vec3(0.f, 1.f, 0.f));
	
	auto& effects = world->effects;

	std::sort( effects.begin(), effects.end(),
			   [&](const VisualFX* a, const VisualFX* b) {
		return glm::distance( a->getPosition(), cpos ) > glm::distance( b->getPosition(), cpos );
	});

	for(VisualFX* fx : effects) {
		// Other effects not implemented yet
		if( fx->getType() != VisualFX::Particle ) continue;
		
		auto& particle = fx->particle;

		auto& p = particle.position;

		glm::mat4 m(1.f);

		// Figure the direction to the camera center.
		auto amp = cpos - p;
		glm::vec3 ptc = particle.up;

		if( particle.orientation == VisualFX::ParticleData::UpCamera ) {
			ptc = glm::normalize(amp - (glm::dot(amp, cfwd))*cfwd);
		}
		else if( particle.orientation == VisualFX::ParticleData::Camera ) {
			ptc = amp;
		}

		glm::vec3 f = glm::normalize(particle.direction);
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
		m = glm::scale(glm::inverse(m), glm::vec3(particle.size, 1.f));

		//m = glm::translate(m, p);

		Renderer::DrawParameters dp;
		dp.textures = {particle.texture->getName()};
		dp.ambient = 1.f;
		dp.colour = glm::u8vec4(particle.colour * 255.f);
		dp.start = 0;
		dp.count = 4;
		dp.diffuse = 1.f;
		
		renderer->drawArrays(m, &particleDraw, dp);
	}
}

void GameRenderer::drawOnScreenText()
{
	/// @ TODO
}

void GameRenderer::drawTexture(TextureData* texture, glm::vec4 extents)
{
	glUseProgram(ssRectProgram);

	// Move into NDC
	extents.x /= renderer->getViewport().x;
	extents.y /= renderer->getViewport().y;
	extents.z /= renderer->getViewport().x;
	extents.w /= renderer->getViewport().y;
	extents.x += extents.z / 2.f;
	extents.y += extents.w / 2.f;
	extents.x -= .5f;
	extents.y -= .5f;
	extents *= glm::vec4(2.f,-2.f, 1.f, 1.f);

	glEnable(GL_BLEND);
	glUniform2f(ssRectOffset, extents.x, extents.y);
	glUniform2f(ssRectSize, extents.z, extents.w);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->getName());
	glUniform1i(ssRectTexture, 0);
	glUniform4f(ssRectColour, 0.f, 0.f, 0.f, 1.f);

	glBindVertexArray( ssRectDraw.getVAOName() );
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Ooops
	renderer->invalidate();
}

void GameRenderer::drawColour(const glm::vec4& colour, glm::vec4 extents)
{
	glUseProgram(ssRectProgram);

	// Move into NDC
	extents.x /= renderer->getViewport().x;
	extents.y /= renderer->getViewport().y;
	extents.z /= renderer->getViewport().x;
	extents.w /= renderer->getViewport().y;
	extents.x += extents.z / 2.f;
	extents.y += extents.w / 2.f;
	extents.x -= .5f;
	extents.y -= .5f;
	extents *= glm::vec4(2.f,-2.f, 1.f, 1.f);

	glEnable(GL_BLEND);
	glUniform2f(ssRectOffset, extents.x, extents.y);
	glUniform2f(ssRectSize, extents.z, extents.w);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(ssRectTexture, 0);
	glUniform4f(ssRectColour, colour.r, colour.g, colour.b, colour.a);

	glBindVertexArray( ssRectDraw.getVAOName() );
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Ooops
	renderer->invalidate();
}

bool GameRenderer::renderFrame(Model* m, ModelFrame* f, const glm::mat4& matrix, GameObject* object, float opacity, bool queueTransparent)
{
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

void GameRenderer::renderModel(Model* model, const glm::mat4& modelMatrix, GameObject* object)
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

void GameRenderer::setViewport(int w, int h)
{
	auto& lastViewport = renderer->getViewport();
	if( lastViewport.x != w || lastViewport.y != h)
	{
		renderer->setViewport({w, h});
		
		glBindTexture(GL_TEXTURE_2D, fbTextures[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, fbTextures[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, w, h, 0, GL_RED, GL_FLOAT, NULL);
		
		glBindRenderbuffer(GL_RENDERBUFFER, fbRenderBuffers[0]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	}
}
