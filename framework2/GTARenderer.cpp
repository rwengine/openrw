#include <renderwure/render/GTARenderer.hpp>
#include <renderwure/engine/GTAEngine.hpp>

#include <glm/gtc/type_ptr.hpp>

const char *vertexShaderSource = "#version 130\n"
"in vec3 position;"
"in vec3 normal;"
"in vec2 texCoords;"
"out vec3 Normal;"
"out vec2 TexCoords;"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"
"void main()"
"{"
"	Normal = normal;"
"	TexCoords = texCoords;"
"	gl_Position = proj * view * model * vec4(position, 1.0);"
"}";
const char *fragmentShaderSource = "#version 130\n"
"in vec3 Normal;"
"in vec2 TexCoords;"
"uniform sampler2D texture;"
"uniform vec4 BaseColour;"
"uniform vec4 AmbientColour;"
"uniform vec3 SunDirection;"
"void main()"
"{"
"   vec4 c = texture2D(texture, TexCoords);"
"   if(c.a < 0.5) discard;"
"	gl_FragColor = AmbientColour + (c * BaseColour * clamp(dot(Normal, SunDirection), 0.2, 1));"
// "	gl_FragColor = vec4((Normal*0.5)+0.5, 1.0);"
// "	gl_FragColor = c * vec4((Normal*0.5)+0.5, 1.0);"
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

GTARenderer::GTARenderer()
: camera()
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

	uniModel = glGetUniformLocation(worldProgram, "model");
	uniView = glGetUniformLocation(worldProgram, "view");
	uniProj = glGetUniformLocation(worldProgram, "proj");
	uniCol = glGetUniformLocation(worldProgram, "BaseColour");
	uniAmbientCol = glGetUniformLocation(worldProgram, "AmbientColour");
	uniSunDirection = glGetUniformLocation(worldProgram, "SunDirection");
}

float mix(uint8_t a, uint8_t b, float num)
{
	return a+(b-a)*num;
}

void GTARenderer::renderWorld(GTAEngine* engine)
{
	static float letime = 0;
	float leclock = fmod(letime / 10, 24);
	int hour = int(leclock);
	int hournext = (hour + 1) % 24;

	// std::cout << leclock << " " << hour << std::endl;
	auto weather = engine->gameData.weatherLoader.weather[hour];
	auto weathernext = engine->gameData.weatherLoader.weather[hournext];
	auto color = weather.skyTopColor;
	auto colornext = weathernext.skyTopColor;

	float interpolate = leclock - int(leclock);
	float r = mix(color.r, colornext.r, interpolate) / 255.0;
	float g = mix(color.g, colornext.g, interpolate) / 255.0;
	float b = mix(color.b, colornext.b, interpolate) / 255.0;

	glm::vec3 ambient{
		mix(weather.ambientColor.r, weathernext.ambientColor.r, interpolate) / 255.0,
		mix(weather.ambientColor.g, weathernext.ambientColor.g, interpolate) / 255.0,
		mix(weather.ambientColor.b, weathernext.ambientColor.b, interpolate) / 255.0,
	};
	float theta = (leclock - 12)/24.0 * 2 * 3.14159265;
	glm::vec3 sunDirection{
		sin(theta),
		0.0,
		cos(theta),
	};
	sunDirection = glm::normalize(sunDirection);
	/*
	std::cout << "CLOCK IS " << leclock << std::endl;
	std::cout << "AMBIENT " << ambient.x << ", " << ambient.y << ", " << ambient.z << std::endl;
	std::cout << "SUN DIR " << sunDirection.x << ", " << sunDirection.y << ", " << sunDirection.z << std::endl;
	*/

	glUniform4f(uniAmbientCol, ambient.x, ambient.y, ambient.z, 1.f);
	glUniform3f(uniSunDirection, sunDirection.x, sunDirection.y, sunDirection.z);

	letime++;
	glClearColor(r, g, b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 proj = camera.frustum.projection();
	glm::mat4 view = camera.frustum.view;
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
	
	camera.frustum.update(camera.frustum.projection() * view);
	
	rendered = culled = 0;

	auto& textureLoader = engine->gameData.textureLoader;

	for(size_t i = 0; i < engine->objectInstances.size(); ++i) {
		GTAEngine::GTAInstance& inst = engine->objectInstances[i];
		LoaderIPLInstance &obj = inst.instance;
		std::string modelname = obj.model;
		
		std::unique_ptr<Model> &model = engine->gameData.models[modelname];
		
		glm::quat rot(-obj.rotW, obj.rotX, obj.rotY, obj.rotZ);
		glm::vec3 pos(obj.posX, obj.posY, obj.posZ);
		glm::vec3 scale(obj.scaleX, obj.scaleY, obj.scaleZ);
		
		float mindist = 100000.f;
		for (size_t g = 0; g < model->geometries.size(); g++) 
		{
			RW::BSGeometryBounds& bounds = model->geometries[g].geometryBounds;
			mindist = std::min(mindist, glm::length((pos+bounds.center) - camera.worldPos) - bounds.radius);
		}
		if( mindist > (inst.object->drawDistance[0] * (inst.object->LOD ? 1.f : 2.f))
			|| (inst.object->LOD && mindist < 250.f) ) {
			culled++;
			continue;
		}
		
		if(!model)
		{
			std::cout << "model " << modelname << " not there (" << engine->gameData.models.size() << " models loaded)" << std::endl;
		}

		renderObject(engine, model, pos, rot, scale);
	}
	
	for(size_t v = 0; v < engine->vehicleInstances.size(); ++v) {
		GTAEngine::GTAVehicle& inst = engine->vehicleInstances[v];
		std::string modelname = inst.vehicle->modelName;
		
		std::unique_ptr<Model> &model = engine->gameData.models[modelname];
		
		if(!model)
		{
			std::cout << "model " << modelname << " not there (" << engine->gameData.models.size() << " models loaded)" << std::endl;
		}
		
		glm::mat4 matrixModel;
		matrixModel = glm::translate(matrixModel, inst.position);
		//matrixModel = glm::scale(matrixModel, scale);
		////matrixModel = matrixModel * glm::mat4_cast(rot);
		
		glm::mat4 matrixVehicle = matrixModel;
		
		for (size_t a = 0; a < model->atomics.size(); a++) 
		{
			size_t g = model->atomics[a].geometry;
			RW::BSGeometryBounds& bounds = model->geometries[g].geometryBounds;
			if(! camera.frustum.intersects(bounds.center + inst.position, bounds.radius)) {
				culled++;
				continue;
			}
			else {
				rendered++;
			}
			
			matrixModel = matrixVehicle;
			
			// Hackily sort out the model data (Todo: be less hacky)
			size_t fi = model->atomics[a].frame;
			if(model->frameNames.size() > fi) {
				std::string& name = model->frameNames[fi];
				if( name.substr(name.size()-3) == "dam" || name.find("lo") != name.npos || name.find("dummy") != name.npos ) {
					continue;
				}
			}
			while(fi != 0) {
				matrixModel = glm::translate(matrixModel, model->frames[fi].position);
				matrixModel = matrixModel * glm::mat4(model->frames[fi].rotation);
				fi = model->frames[fi].index;
			}
			
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(matrixModel));
			if( (model->geometries[g].flags & RW::BSGeometry::ModuleMaterialColor) != RW::BSGeometry::ModuleMaterialColor) {
				glUniform4f(uniCol, 1.f, 1.f, 1.f, 1.f);
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, model->geometries[g].VBO);
			glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)(model->geometries[g].vertices.size() * sizeof(float) * 3));
			glEnableVertexAttribArray(posAttrib);
			glEnableVertexAttribArray(texAttrib);
			
			for(size_t sg = 0; sg < model->geometries[g].subgeom.size(); ++sg) 
			{
				if (model->geometries[g].materials.size() > model->geometries[g].subgeom[sg].material) { 
					Model::Material& mat = model->geometries[g].materials[model->geometries[g].subgeom[sg].material];
					// std::cout << model->geometries[g].textures.size() << std::endl;
					// std::cout << "Looking for " << model->geometries[g].textures[0].name << std::endl;
					if(mat.textures.size() > 0) {
						textureLoader.bindTexture(model->geometries[g].materials[model->geometries[g].subgeom[sg].material].textures[0].name);
					}
					
					if( (model->geometries[g].flags & RW::BSGeometry::ModuleMaterialColor) == RW::BSGeometry::ModuleMaterialColor) {
						auto colmasked = mat.colour;
						size_t R = colmasked % 256; colmasked /= 256;
						size_t G = colmasked % 256; colmasked /= 256;
						size_t B = colmasked % 256; colmasked /= 256;
						if( R == 60 && G == 255 && B == 0 ) {
							glUniform4f(uniCol, inst.colourPrimary.r, inst.colourPrimary.g, inst.colourPrimary.b, 1.f);
						}
						else if( R == 255 && G == 0 && B == 175 ) {
							glUniform4f(uniCol, inst.colourSecondary.r, inst.colourSecondary.g, inst.colourSecondary.b, 1.f);
						}
						else {
							glUniform4f(uniCol, R/255.f, G/255.f, B/255.f, 1.f);
						}
					}
				}
				else {
					
				}
				
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->geometries[g].subgeom[sg].EBO);

				glDrawElements(GL_TRIANGLES, model->geometries[g].subgeom[sg].indices.size(), GL_UNSIGNED_INT, NULL);
			}
		}
		
		// Draw wheels n' stuff
		for( size_t w = 0; w < inst.vehicle->wheelPositions.size(); ++w) {
			auto woi = engine->objectTypes.find(inst.vehicle->wheelModelID);
			if(woi != engine->objectTypes.end()) {
				std::unique_ptr<Model> &wheelModel = engine->gameData.models["wheels"];
				if( wheelModel) {
					auto wwpos = matrixVehicle * glm::vec4(inst.vehicle->wheelPositions[w], 1.f);
					renderNamedFrame(engine, wheelModel, glm::vec3(wwpos), glm::quat(), glm::vec3(1.f, inst.vehicle->wheelScale, inst.vehicle->wheelScale), woi->second->modelName);
				}
				else {
					std::cout << "Wheel model " << woi->second->modelName << " not loaded" << std::endl;
				}
			}
		}
	}
}

void GTARenderer::renderNamedFrame(GTAEngine* engine, const std::unique_ptr<Model>& model, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const std::string& name)
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
		
		//matrixModel = glm::translate(matrixModel, model->frames[f].position);
		//matrixModel = matrixModel * glm::mat4(model->frames[model->atomics[a].frame].rotation);
		
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(matrixModel));
		glUniform4f(uniCol, 1.f, 1.f, 1.f, 1.f);
		
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
					engine->gameData.textureLoader.bindTexture(model->geometries[g].materials[model->geometries[g].subgeom[sg].material].textures[0].name);
				}
			}
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->geometries[g].subgeom[sg].EBO);

			glDrawElements(GL_TRIANGLES, model->geometries[g].subgeom[sg].indices.size(), GL_UNSIGNED_INT, NULL);
		}
		break;
	}
}

void GTARenderer::renderObject(GTAEngine* engine, const std::unique_ptr<Model>& model, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
{
	for (size_t a = 0; a < model->atomics.size(); a++) 
	{
		size_t g = model->atomics[a].geometry;
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
		
		matrixModel = glm::translate(matrixModel, model->frames[model->atomics[a].frame].position);
		//matrixModel = matrixModel * glm::mat4(model->frames[model->atomics[a].frame].rotation);
		
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(matrixModel));
		glUniform4f(uniCol, 1.f, 1.f, 1.f, 1.f);
		
		glBindBuffer(GL_ARRAY_BUFFER, model->geometries[g].VBO);
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)(model->geometries[g].vertices.size() * sizeof(float) * 3));
		glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 0,
			(void *) ((model->geometries[g].vertices.size() * sizeof(float) * 3) + (model->geometries[g].texcoords.size() * sizeof(float) * 2))
		);
		// std::cout << "Num normals: " << model->geometries[g].normals.size() << std::endl;
		glEnableVertexAttribArray(posAttrib);
		glEnableVertexAttribArray(texAttrib);
		glEnableVertexAttribArray(normalAttrib);
		
		for(size_t sg = 0; sg < model->geometries[g].subgeom.size(); ++sg) 
		{
			if (model->geometries[g].materials.size() > model->geometries[g].subgeom[sg].material) { 
				// std::cout << model->geometries[g].textures.size() << std::endl;
				// std::cout << "Looking for " << model->geometries[g].textures[0].name << std::endl;
				if(model->geometries[g].materials[model->geometries[g].subgeom[sg].material].textures.size() > 0) {
					engine->gameData.textureLoader.bindTexture(model->geometries[g].materials[model->geometries[g].subgeom[sg].material].textures[0].name);
				}
			}
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->geometries[g].subgeom[sg].EBO);

			glDrawElements(GL_TRIANGLES, model->geometries[g].subgeom[sg].indices.size(), GL_UNSIGNED_INT, NULL);
		}
	}
}