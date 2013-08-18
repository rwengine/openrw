#include <renderwure/render/GTARenderer.hpp>
#include <renderwure/engine/GTAEngine.hpp>

#include <deque>

#include <glm/gtc/type_ptr.hpp>

const char *vertexShaderSource = "#version 130\n"
"in vec3 position;"
"in vec3 normal;"
"in vec2 texCoords;"
"in vec4 colour;"
"out vec3 Normal;"
"out vec2 TexCoords;"
"out vec4 Colour;"
"out vec4 EyeSpace;"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"
"void main()"
"{"
"	Normal = normal;"
"	TexCoords = texCoords;"
"	Colour = colour;"
"	vec4 eyeSpace = view * model * vec4(position, 1.0);"
"	EyeSpace = proj * eyeSpace;"
"	gl_Position = proj * eyeSpace;"
"}";
const char *fragmentShaderSource = "#version 130\n"
"in vec3 Normal;"
"in vec2 TexCoords;"
"in vec4 Colour;"
"in vec4 EyeSpace;"
"uniform sampler2D texture;"
"uniform vec4 BaseColour;"
"uniform vec4 AmbientColour;"
"uniform vec4 DynamicColour;"
"uniform vec3 SunDirection;"
"uniform float FogStart;"
"uniform float FogEnd;"
"uniform float MaterialDiffuse;"
"uniform float MaterialAmbient;"
"void main()"
"{"
"   vec4 c = texture2D(texture, TexCoords);"
"   if(c.a < 0.5) discard;"
"	float fogZ = (gl_FragCoord.z / gl_FragCoord.w);"
"	float fogfac = clamp( (FogEnd-fogZ)/(FogEnd-FogStart), 0.0, 1.0 );"
//"	float l = clamp(dot(Normal, SunDirection), 0.0, 1);"
//"	gl_FragColor = vec4(vec3(fogfac), 1.0);"
"	gl_FragColor = mix(AmbientColour, BaseColour * (vec4(0.5) + Colour * 0.5) * (vec4(0.5) + DynamicColour * 0.5) * c, fogfac);"
// "	gl_FragColor = vec4((Normal*0.5)+0.5, 1.0);"
// "	gl_FragColor = c * vec4((Normal*0.5)+0.5, 1.0);"
"}";

const char *skydomeVertexShaderSource = "#version 130\n"
"in vec3 position;"
"uniform mat4 view;"
"uniform mat4 proj;"
"out vec3 Position;"
"uniform float Far;"
"void main() {"
"	Position = position;"
"	vec4 viewsp = proj * mat4(mat3(view)) * vec4(position, 1.0);"
"	viewsp.z = viewsp.w - 0.000001;"
"	gl_Position = viewsp;"
"}";
const char *skydomeFragmentShaderSource = "#version 130\n"
"in vec3 Position;"
"uniform vec4 TopColor;"
"uniform vec4 BottomColor;"
"void main() {"
"	gl_FragColor = mix(BottomColor, TopColor, clamp(Position.z, 0, 1));"
"}";
const size_t skydomeSegments = 8, skydomeRows = 10;


float planedata[] = {
	// Vertices
	 1.0f, 1.0f, 0.f,
	-0.0f, 1.0f, 0.f,
	 1.0f,-0.0f, 0.f,
	-0.0f,-0.0f, 0.f,
	// UV coords
	1.f, 1.f,
	0.f, 1.f,
	1.f, 0.f,
	0.f, 0.f,
	// Normals
	0.f, 0.f, 1.f,
	0.f, 0.f, 1.f,
	0.f, 0.f, 1.f,
	0.f, 0.f, 1.f,
	// Colours
	1.f, 1.f, 1.f,
	1.f, 1.f, 1.f,
	1.f, 1.f, 1.f,
	1.f, 1.f, 1.f
};

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

GTARenderer::GTARenderer(GTAEngine* engine)
    : engine(engine)
{	
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	worldProgram = glCreateProgram();
	glAttachShader(worldProgram, vertexShader);
	glAttachShader(worldProgram, fragmentShader);
	glLinkProgram(worldProgram);
	glUseProgram(worldProgram);

	posAttrib = glGetAttribLocation(worldProgram, "position");
	texAttrib = glGetAttribLocation(worldProgram, "texCoords");
	normalAttrib = glGetAttribLocation(worldProgram, "normal");
	colourAttrib = glGetAttribLocation(worldProgram, "colour");

	uniModel = glGetUniformLocation(worldProgram, "model");
	uniView = glGetUniformLocation(worldProgram, "view");
	uniProj = glGetUniformLocation(worldProgram, "proj");
	uniCol = glGetUniformLocation(worldProgram, "BaseColour");
	uniAmbientCol = glGetUniformLocation(worldProgram, "AmbientColour");
	uniSunDirection = glGetUniformLocation(worldProgram, "SunDirection");
	uniDynamicCol = glGetUniformLocation(worldProgram, "DynamicColour");
	uniMatDiffuse = glGetUniformLocation(worldProgram, "MaterialDiffuse");
	uniMatAmbient = glGetUniformLocation(worldProgram, "MaterialAmbient");
	uniFogStart = glGetUniformLocation(worldProgram, "FogStart");
	uniFogEnd = glGetUniformLocation(worldProgram, "FogEnd");
	
	vertexShader = compileShader(GL_VERTEX_SHADER, skydomeVertexShaderSource);
	fragmentShader = compileShader(GL_FRAGMENT_SHADER, skydomeFragmentShaderSource);
	skyProgram = glCreateProgram();
	glAttachShader(skyProgram, vertexShader);
	glAttachShader(skyProgram, fragmentShader);
	glLinkProgram(skyProgram);
	glUseProgram(skyProgram);
	skyUniView = glGetUniformLocation(skyProgram, "view");
	skyUniProj = glGetUniformLocation(skyProgram, "proj");
	skyUniTop = glGetUniformLocation(skyProgram, "TopColor");
	skyUniBottom = glGetUniformLocation(skyProgram, "BottomColor");
	
	glGenVertexArrays( 1, &vao );
	
	// prepare our special internal plane.
	glGenBuffers(1, &planeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planedata), planedata, GL_STATIC_DRAW);
	
	// And our skydome while we're at it.
	glGenBuffers(1, &skydomeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skydomeVBO);
	size_t segments = skydomeSegments, rows = skydomeRows;
	float radius = 1.f;
	const float pi = 3.14159265;
	const float pio2 = (pi / 2.f);
	glm::vec3 skydomeBuff[rows * segments * 2];
	for( size_t s = 0, i = 0; s < segments; ++s) {
		for( size_t r = 0; r < rows; ++r) {
			skydomeBuff[i++] = glm::vec3(
				radius * cos( (s+1.f)/segments * pio2) * sin( 2.0f * (float)r/rows * pi),
				radius * cos( (s+1.f)/segments * pio2) * cos( 2.0f * (float)r/rows * pi),
				radius * sin( (s+1.f)/segments * pio2)
			);
			skydomeBuff[i++] = glm::vec3(
				radius * cos( (s+0.f)/segments * pio2) * sin( 2.0f * (float)r/rows * pi),
				radius * cos( (s+0.f)/segments * pio2) * cos( 2.0f * (float)r/rows * pi),
				radius * sin( (s+0.f)/segments * pio2)
			);
		}
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(skydomeBuff), skydomeBuff, GL_STATIC_DRAW);

    glGenBuffers(1, &debugVBO);
    glGenTextures(1, &debugTex);
    glGenVertexArrays(1, &debugVAO);
}

float mix(uint8_t a, uint8_t b, float num)
{
	return a+(b-a)*num;
}

void GTARenderer::renderWorld()
{
	glBindVertexArray( vao );
	
	float gameTime = fmod(engine->gameTime, 24.f);
	int hour = floor(gameTime);
	int hournext = (hour + 1) % 24;

	// std::cout << leclock << " " << hour << std::endl;
	auto weather = engine->gameData.weatherLoader.weather[hour];
	auto weathernext = engine->gameData.weatherLoader.weather[hournext];
	
	float interpolate = gameTime - hour;
	glm::vec3 skyTop{
		mix(weather.skyTopColor.r, weathernext.skyTopColor.r, interpolate) / 255.0,
		mix(weather.skyTopColor.g, weathernext.skyTopColor.g, interpolate) / 255.0,
		mix(weather.skyTopColor.b, weathernext.skyTopColor.b, interpolate) / 255.0,
	};
	glm::vec3 skyBottom{
		mix(weather.skyBottomColor.r, weathernext.skyBottomColor.r, interpolate) / 255.0,
		mix(weather.skyBottomColor.g, weathernext.skyBottomColor.g, interpolate) / 255.0,
		mix(weather.skyBottomColor.b, weathernext.skyBottomColor.b, interpolate) / 255.0,
	};
	
	glm::vec3 ambient{
		mix(weather.ambientColor.r, weathernext.ambientColor.r, interpolate) / 255.0,
		mix(weather.ambientColor.g, weathernext.ambientColor.g, interpolate) / 255.0,
		mix(weather.ambientColor.b, weathernext.ambientColor.b, interpolate) / 255.0,
	};
	glm::vec3 dynamic{
		mix(weather.directLightColor.r, weathernext.directLightColor.r, interpolate) / 255.0,
		mix(weather.directLightColor.g, weathernext.directLightColor.g, interpolate) / 255.0,
		mix(weather.directLightColor.b, weathernext.directLightColor.b, interpolate) / 255.0,
	};
	float theta = (gameTime - 12.f)/24.0 * 2 * 3.14159265;
	glm::vec3 sunDirection{
		sin(theta),
		0.0,
		cos(theta),
	};
	sunDirection = glm::normalize(sunDirection);
	float weatherFar = weather.farClipping; //mix(weather.farClipping, weathernext.farClipping, interpolate);
	camera.frustum.far = weatherFar;
	/*
	std::cout << "CLOCK IS " << leclock << std::endl;
	std::cout << "AMBIENT " << ambient.x << ", " << ambient.y << ", " << ambient.z << std::endl;
	std::cout << "SUN DIR " << sunDirection.x << ", " << sunDirection.y << ", " << sunDirection.z << std::endl;
	*/
	
	glUseProgram(worldProgram);

    glUniform1f(uniFogStart, weather.fogStart);
	glUniform1f(uniFogEnd, camera.frustum.far);

	glUniform4f(uniAmbientCol, ambient.x, ambient.y, ambient.z, 1.f);
	glUniform4f(uniDynamicCol, dynamic.x, dynamic.y, dynamic.z, 1.f);
	glUniform3f(uniSunDirection, sunDirection.x, sunDirection.y, sunDirection.z);
	glUniform1f(uniMatDiffuse, 0.9f);
	glUniform1f(uniMatAmbient, 0.1f);
	
	glClearColor(skyBottom.r, skyBottom.g, skyBottom.b, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 proj = camera.frustum.projection();
	glm::mat4 view = camera.frustum.view;
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
	
	camera.frustum.update(camera.frustum.projection() * view);
	
	rendered = culled = 0;

	auto& textureLoader = engine->gameData.textureLoader;
	
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*3*4));
	glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*3*4 + sizeof(float)*2*4));
	glVertexAttribPointer(colourAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*3*4 + sizeof(float)*2*4 + sizeof(float)*3*4));
	glEnableVertexAttribArray(posAttrib);
	glEnableVertexAttribArray(texAttrib);
	glEnableVertexAttribArray(normalAttrib);
	glEnableVertexAttribArray(colourAttrib);
	textureLoader.bindTexture("water_old");
	
	for( size_t w = 0; w < engine->gameData.waterRects.size(); ++w) {
		GTATypes::WaterRect& r = engine->gameData.waterRects[w];
		glm::vec3 vert[4] = {
			glm::vec3(r.xRight, r.yTop,    r.height),
			glm::vec3(r.xLeft,  r.yTop,    r.height),
			glm::vec3(r.xRight, r.yBottom, r.height),
			glm::vec3(r.xLeft,  r.yBottom, r.height)
		};
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * 4, vert);
		
		glm::mat4 matrixModel(1.f);
		
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(matrixModel));
		glUniform4f(uniCol, 1.f, 1.f, 1.f, 1.f);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

    for(size_t i = 0; i < engine->pedestrians.size(); ++i) {
        GTACharacter* charac = engine->pedestrians[i];

        glm::mat4 matrixModel;
        matrixModel = glm::translate(matrixModel, charac->position);
        matrixModel = matrixModel * glm::mat4_cast(charac->rotation);

        if(!charac->model) continue;

        renderModel(charac->model, matrixModel, charac);
    }
	
	for(size_t i = 0; i < engine->objectInstances.size(); ++i) {
        GTAInstance& inst = *engine->objectInstances[i];
        LoaderIPLInstance &obj = inst.instance;
		
		if(((inst.object->flags & LoaderIDE::OBJS_t::NIGHTONLY) | (inst.object->flags & LoaderIDE::OBJS_t::DAYONLY)) != 0) {
			//continue;
		}

        if(!inst.model)
        {
            std::cout << "model " << obj.model << " not loaded (" << engine->gameData.models.size() << " models loaded)" << std::endl;
        }
		
        glm::mat4 matrixModel;
        matrixModel = glm::translate(matrixModel, inst.position);
        matrixModel = glm::scale(matrixModel, inst.scale);
        matrixModel = matrixModel * glm::mat4_cast(inst.rotation);

        float mindist = 100000.f;
        for (size_t g = 0; g < inst.model->geometries.size(); g++)
        {
            RW::BSGeometryBounds& bounds = inst.model->geometries[g].geometryBounds;
            mindist = std::min(mindist, glm::length((glm::vec3(matrixModel[3])+bounds.center) - camera.worldPos) - bounds.radius);
        }
        
        if ( mindist > inst.object->drawDistance[0] ) {
			if ( !inst.LODinstance ) {
				culled++;
				continue;
			}
			else {
				if( mindist > inst.LODinstance->object->drawDistance[0] ) {
					culled++;
					continue;
				}
				else {
					renderModel(inst.LODinstance->model, matrixModel);
				}
			}
        }
		else if (! inst.object->LOD ) {
			renderModel(inst.model, matrixModel);
		}
	}
	
	for(size_t v = 0; v < engine->vehicleInstances.size(); ++v) {
        GTAVehicle& inst = engine->vehicleInstances[v];

        if(!inst.model)
		{
            std::cout << "model " <<  inst.vehicle->modelName << " not loaded (" << engine->gameData.models.size() << " models loaded)" << std::endl;
		}
		
		glm::mat4 matrixModel;
        matrixModel = glm::translate(matrixModel, inst.position);

        glm::mat4 matrixVehicle = matrixModel;

        renderModel(inst.model, matrixModel, &inst);

		// Draw wheels n' stuff
		for( size_t w = 0; w < inst.vehicle->wheelPositions.size(); ++w) {
			auto woi = engine->objectTypes.find(inst.vehicle->wheelModelID);
			if(woi != engine->objectTypes.end()) {
                Model* wheelModel = engine->gameData.models["wheels"];
				if( wheelModel) {
					auto wwpos = matrixVehicle * glm::vec4(inst.vehicle->wheelPositions[w], 1.f);
                    renderNamedFrame(wheelModel, glm::vec3(wwpos), glm::quat(), glm::vec3(1.f, inst.vehicle->wheelScale, inst.vehicle->wheelScale), woi->second->modelName);
				}
				else {
					std::cout << "Wheel model " << woi->second->modelName << " not loaded" << std::endl;
				}
			}
		}
	}
	
	glUseProgram(skyProgram);
	
	glBindBuffer(GL_ARRAY_BUFFER, skydomeVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glUniformMatrix4fv(skyUniView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(skyUniProj, 1, GL_FALSE, glm::value_ptr(proj));
	glUniform4f(skyUniTop, skyTop.r, skyTop.g, skyTop.b, 1.f);
	glUniform4f(skyUniBottom, skyBottom.r, skyBottom.g, skyBottom.b, 1.f);
		
	glDrawArrays(GL_TRIANGLE_STRIP, 0, skydomeSegments * skydomeRows * 2 + 1);
	
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray( 0 );
}

void GTARenderer::renderNamedFrame(Model* model, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const std::string& name)
{
	for (size_t f = 0; f < model->frames.size(); f++) 
	{
		if( model->frameNames.size() > f) {
			std::string& fname = model->frameNames[f];
			bool LOD = (fname.find("_l1") != fname.npos || fname.find("_l0") != fname.npos);
			if( LOD || fname != name ) {
				continue;
			}
		}
		else {
			continue;
		}
		
		size_t g = f;
		RW::BSGeometryBounds& bounds = model->geometries[g].geometryBounds;
		if(! camera.frustum.intersects(bounds.center + pos, bounds.radius)) {
			culled++;
			continue;
		}
		else {
			rendered++;
		}
		
		glm::mat4 matrixModel;
		matrixModel = glm::translate(matrixModel, pos);
		matrixModel = glm::scale(matrixModel, scale);
		matrixModel = matrixModel * glm::mat4_cast(rot);
		
        renderGeometry(model, g, matrixModel);
		break;
	}
}

void GTARenderer::renderGeometry(Model* model, size_t g, const glm::mat4& modelMatrix, GTAObject* object)
{
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniform4f(uniCol, 1.f, 1.f, 1.f, 1.f);

    glBindBuffer(GL_ARRAY_BUFFER, model->geometries[g].VBO);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)(model->geometries[g].vertices.size() * sizeof(float) * 3));
    glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 0,
        (void *) ((model->geometries[g].vertices.size() * sizeof(float) * 3) + (model->geometries[g].texcoords.size() * sizeof(float) * 2))
    );
    glVertexAttribPointer(colourAttrib, 4, GL_FLOAT, GL_FALSE, 0,
        (void *) ((model->geometries[g].vertices.size() * sizeof(float) * 3)
            + (model->geometries[g].texcoords.size() * sizeof(float) * 2)
            + (model->geometries[g].normals.size() * sizeof(float) * 3))
    );
    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(texAttrib);
    glEnableVertexAttribArray(normalAttrib);
    glEnableVertexAttribArray(colourAttrib);

    for(size_t sg = 0; sg < model->geometries[g].subgeom.size(); ++sg)
    {
        if (model->geometries[g].materials.size() > model->geometries[g].subgeom[sg].material) {
            Model::Material& mat = model->geometries[g].materials[model->geometries[g].subgeom[sg].material];

            if(mat.textures.size() > 0) {
                engine->gameData.textureLoader.bindTexture(mat.textures[0].name);
            }

            if( (model->geometries[g].flags & RW::BSGeometry::ModuleMaterialColor) == RW::BSGeometry::ModuleMaterialColor) {
                auto colmasked = mat.colour;
                size_t R = colmasked % 256; colmasked /= 256;
                size_t G = colmasked % 256; colmasked /= 256;
                size_t B = colmasked % 256; colmasked /= 256;
                if( object && object->type() == GTAObject::Vehicle ) {
                    auto vehicle = static_cast<GTAVehicle*>(object);
                    if( R == 60 && G == 255 && B == 0 ) {
                        glUniform4f(uniCol, vehicle->colourPrimary.r, vehicle->colourPrimary.g, vehicle->colourPrimary.b, 1.f);
                    }
                    else if( R == 255 && G == 0 && B == 175 ) {
                        glUniform4f(uniCol, vehicle->colourSecondary.r, vehicle->colourSecondary.g, vehicle->colourSecondary.b, 1.f);
                    }
                    else {
                        glUniform4f(uniCol, R/255.f, G/255.f, B/255.f, 1.f);
                    }
                }
                else {
                    glUniform4f(uniCol, R/255.f, G/255.f, B/255.f, 1.f);
                }
            }

            glUniform1f(uniMatDiffuse, mat.diffuseIntensity);
            glUniform1f(uniMatAmbient, mat.ambientIntensity);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->geometries[g].subgeom[sg].EBO);

        glDrawElements((model->geometries[g].facetype == Model::Triangles ? GL_TRIANGLES : GL_TRIANGLE_STRIP), model->geometries[g].subgeom[sg].indices.size(), GL_UNSIGNED_INT, NULL);
    }
}

void GTARenderer::renderModel(Model* model, const glm::mat4& modelMatrix, GTAObject* object)
{
    for (size_t a = 0; a < model->atomics.size(); a++)
    {
        size_t g = model->atomics[a].geometry;
        RW::BSGeometryBounds& bounds = model->geometries[g].geometryBounds;
        if(! camera.frustum.intersects(bounds.center + glm::vec3(modelMatrix[3]), bounds.radius)) {
            culled++;
            continue;
        }
        else {
            rendered++;
        }

        int32_t fi = model->atomics[a].frame;
        if( object && object->type() == GTAObject::Vehicle ) {
            if(model->frameNames.size() > fi) {
                std::string& name = model->frameNames[fi];
                if( name.substr(name.size()-3) == "dam" || name.find("lo") != name.npos || name.find("dummy") != name.npos ) {
                    continue;
                }
            }
        }

        if( (model->geometries[g].flags & RW::BSGeometry::ModuleMaterialColor) != RW::BSGeometry::ModuleMaterialColor) {
            glUniform4f(uniCol, 1.f, 1.f, 1.f, 1.f);
        }

        renderGeometry(model, g, modelMatrix * model->getFrameMatrix(model->atomics[a].frame), object);
    }
}

void GTARenderer::renderPaths()
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

    for( size_t n = 0; n < engine->ainodes.size(); ++n ) {
        auto start = engine->ainodes[n];
		
		if( start->type == GTAAINode::Pedestrian ) {
			pedlines.push_back(start->position);
			pedlines.push_back(start->position+glm::vec3(0.f, 0.f, 1.f));
		}	
		else {
			carlines.push_back(start->position);
			carlines.push_back(start->position+glm::vec3(0.f, 0.f, 1.f));
		}
			
		for( size_t c = 0; c < start->connections.size(); ++c ) {
			auto end = start->connections[c];
			
			if( start->type == GTAAINode::Pedestrian ) {	
				pedlines.push_back(start->position);
				pedlines.push_back(end->position);
			}
			else {
				carlines.push_back(start->position);
				carlines.push_back(end->position);
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
