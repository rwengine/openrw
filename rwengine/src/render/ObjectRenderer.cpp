#include <render/ObjectRenderer.hpp>
#include <data/Skeleton.hpp>
#include <data/Model.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <data/CutsceneData.hpp>
#include <glm/gtc/type_ptr.hpp>

// Objects that we know how to turn into renderlist entries
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/ProjectileObject.hpp>
#include <objects/PickupObject.hpp>
#include <objects/CutsceneObject.hpp>
#include <items/InventoryItem.hpp>
#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif

constexpr float kDrawDistanceFactor = 1.0f;
constexpr float kWorldDrawDistanceFactor = kDrawDistanceFactor;
#if 0 // There's no distance based culling for these types of objects yet
constexpr float kVehicleDrawDistanceFactor = kDrawDistanceFactor;
constexpr float kPedestrianDrawDistanceFactor = kDrawDistanceFactor;
#endif

RenderKey createKey(bool transparent, float normalizedDepth, Renderer::Textures& textures)
{
	return ((transparent?0x1:0x0) << 31)
			| uint32_t(0x7FFFFF * (transparent? 1.f - normalizedDepth : normalizedDepth)) << 8
			| uint8_t(0xFF & (textures.size() > 0 ? textures[0] : 0)) << 0;
}

void ObjectRenderer::renderGeometry(Model* model,
									size_t g,
									const glm::mat4& modelMatrix,
									float opacity,
									GameObject* object,
									RenderList& outList)
{
	for(size_t sg = 0; sg < model->geometries[g]->subgeom.size(); ++sg)
	{
		Model::SubGeometry& subgeom = model->geometries[g]->subgeom[sg];

		bool isTransparent = false;

		Renderer::DrawParameters dp;

		dp.colour = {255, 255, 255, 255};
		dp.count = subgeom.numIndices;
		dp.start = subgeom.start;
		dp.textures = {0};
		dp.visibility = 1.f;

		if(object && object->type() == GameObject::Instance) {
			auto instance = static_cast<InstanceObject*>(object);
			dp.depthWrite = !(instance->object->flags & ObjectData::NO_ZBUFFER_WRITE);
		}

		if (model->geometries[g]->materials.size() > subgeom.material) {
			Model::Material& mat = model->geometries[g]->materials[subgeom.material];

			if(mat.textures.size() > 0 ) {
				auto tex = mat.textures[0].texture;
				if( ! tex )
				{
					auto& tC = mat.textures[0].name;
					auto& tA = mat.textures[0].alphaName;
					tex = m_world->data->findTexture(tC, tA);
					if( ! tex )
					{
						//logger->warning("Renderer", "Missing texture: " + tC + " " + tA);
						dp.textures = { m_errorTexture };
					}
					mat.textures[0].texture = tex;
				}
				if( tex )
				{
					if( tex->isTransparent() ) {
						isTransparent = true;
					}
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

			dp.visibility = opacity;

			if( dp.colour.a < 255 ) {
				isTransparent = true;
			}

			dp.diffuse = mat.diffuseIntensity;
			dp.ambient = mat.ambientIntensity;
		}

		dp.blend = isTransparent;

		glm::vec3 position(modelMatrix[3]);
		float distance = glm::length(m_camera.position - position);
		float depth = (distance - m_camera.frustum.near) / (m_camera.frustum.far - m_camera.frustum.near);
		outList.emplace_back(
							  createKey(isTransparent, depth * depth, dp.textures),
							  modelMatrix,
							  &model->geometries[g]->dbuff,
							  dp
						  );
	}
}
bool ObjectRenderer::renderFrame(Model* m,
								 ModelFrame* f,
								 const glm::mat4& matrix,
								 GameObject* object,
								 float opacity,
								 RenderList& outList)
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
				if(! m_camera.frustum.intersects(boundpos, bounds.radius)) {
					continue;
				}
			}

			renderGeometry(m, g, localmatrix, opacity, object, outList);
		}
	}

	for(ModelFrame* c : f->getChildren()) {
		renderFrame(m, c, localmatrix, object, opacity, outList);
	}
	return true;
}

void ObjectRenderer::renderItem(InventoryItem *item,
								const glm::mat4 &modelMatrix,
								RenderList& outList)
{
	// srhand
	if (item->getModelID() == -1) {
		return; // No model for this item
	}

	std::shared_ptr<ObjectData> odata = m_world->data->findObjectType<ObjectData>(item->getModelID());
	auto weapons = m_world->data->models["weapons"];
	if( weapons && weapons->resource ) {
		auto itemModel = weapons->resource->findFrame(odata->modelName + "_l0");
		auto matrix = glm::inverse(itemModel->getTransform());
		if(itemModel) {
			renderFrame(weapons->resource,
						itemModel,
						modelMatrix * matrix,
						nullptr,
						1.f,
						outList);
		}
	}
}

void ObjectRenderer::renderInstance(InstanceObject *instance,
									RenderList& outList)
{
	if(!instance->model->resource) {
		return;
	}

	// Only draw visible objects
	if(!instance->getVisible()) {
		return;
	}


	// Handles times provided by TOBJ data
	const auto currentHour = m_world->getHour();
	if (instance->object->timeOff < instance->object->timeOn)
	{
		if ( currentHour >= instance->object->timeOff && currentHour < instance->object->timeOn )
			return;
	} else {
		if ( currentHour >= instance->object->timeOff || currentHour < instance->object->timeOn )
			return;
	}

	auto matrixModel = instance->getTimeAdjustedTransform(m_renderAlpha);

	float mindist = glm::length(instance->getPosition()-m_camera.position)
			- instance->model->resource->getBoundingRadius();
	mindist *= 1.f / kDrawDistanceFactor;

	Model* model = nullptr;
	ModelFrame* frame = nullptr;

	// These are used to gracefully fade out things that are just out of view distance.
	Model* fadingModel = nullptr;
	ModelFrame* fadingFrame = nullptr;
	auto fadingMatrix = matrixModel;
	float opacity = 0.f;
	constexpr float fadeRange = 50.f;

	if( instance->object->numClumps == 1 ) {
		// Is closest point greater than the *object* draw distance
		float objectRange = instance->object->drawDistance[0];
		float overlap = (mindist - objectRange);
		if( mindist > objectRange ) {
			// Check for LOD instances
			if ( instance->LODinstance ) {
				// Is the closest point greater than the *LOD* draw distance
				float LODrange = instance->LODinstance->object->drawDistance[0];
				if( mindist > LODrange ) {
				}
				else if (instance->LODinstance->model->resource) {
					// The model matrix needs to be for the LOD instead
					matrixModel = instance->LODinstance->getTimeAdjustedTransform(m_renderAlpha);
					// If the object is only just out of range, keep
					// rendering it and screen-door the LOD.
					if (overlap < fadeRange)
					{
						model = instance->LODinstance->model->resource;
						fadingModel = instance->model->resource;
						opacity = 1.f - (overlap / fadeRange);
					}
					else
					{
						model = instance->LODinstance->model->resource;
					}
				}
			}
			// We don't have a LOD object, so fade out gracefully.
			else if (overlap < fadeRange) {
				fadingModel = instance->model->resource;
				opacity = 1.f - (overlap / fadeRange);
			}
		}
		// Otherwise, if we aren't marked as a LOD model, we can render
		else if (! instance->object->LOD ) {
			model = instance->model->resource;
		}
	}
	else {
		auto root = instance->model->resource->frames[0];
		auto objectModel = instance->model->resource;
		fadingFrame = nullptr;
		fadingModel = nullptr;

		matrixModel *= root->getTransform();

		for (int i = 0; i < instance->object->numClumps-1; ++i)
		{
			auto ind = (instance->object->numClumps-1) - i;
			float lodDistance = instance->object->drawDistance[i];
			if( mindist > lodDistance ) {
				fadingFrame = root->getChildren()[ind];
				fadingModel = objectModel;
				opacity = 1.f - ((mindist - lodDistance)/fadeRange);
			}
			else
			{
				model = objectModel;
				frame = root->getChildren()[ind];
			}
		}
	}

	if( model ) {
		frame = frame ? frame : model->frames[0];
		renderFrame(model,
					frame,
					matrixModel * glm::inverse(frame->getTransform()),
					instance,
					1.f,
					outList);
	}
	if( fadingModel ) {
		if(opacity >= 0.01f) {
			fadingFrame = fadingFrame ? fadingFrame : fadingModel->frames[0];
			renderFrame(fadingModel,
						fadingFrame,
						fadingMatrix * glm::inverse(fadingFrame->getTransform()),
						instance,
						opacity,
						outList);
		}
	}
}

void ObjectRenderer::renderCharacter(CharacterObject *pedestrian,
									 RenderList& outList)
{
	glm::mat4 matrixModel;

	if (pedestrian->getCurrentVehicle())
	{
		auto vehicle = pedestrian->getCurrentVehicle();
		auto seat = pedestrian->getCurrentSeat();
		matrixModel = vehicle->getTimeAdjustedTransform( m_renderAlpha );
		if (pedestrian->isEnteringOrExitingVehicle())
		{
			matrixModel = glm::translate(matrixModel, vehicle->getSeatEntryPosition(seat));
		}
		else
		{
			if (seat < vehicle->info->seats.size()) {
				matrixModel = glm::translate(matrixModel, vehicle->info->seats[seat].offset);
			}
		}
	}
	else
	{
		matrixModel = pedestrian->getTimeAdjustedTransform( m_renderAlpha );
	}

	if(!pedestrian->model->resource) return;

	auto root = pedestrian->model->resource->frames[0];

	renderFrame(pedestrian->model->resource,
				root->getChildren()[0],
				matrixModel,
				pedestrian,
				1.f,
				outList);

	if(pedestrian->getActiveItem()) {
		auto handFrame = pedestrian->model->resource->findFrame("srhand");
		glm::mat4 localMatrix;
		if( handFrame ) {
			while( handFrame->getParent() ) {
				localMatrix = pedestrian->skeleton->getMatrix(handFrame->getIndex()) * localMatrix;
				handFrame = handFrame->getParent();
			}
		}
		renderItem(pedestrian->getActiveItem(),
				   matrixModel * localMatrix,
				   outList);
	}
}

void ObjectRenderer::renderWheel(
		VehicleObject* vehicle,
		Model* model,
		const glm::mat4 &matrix,
		const std::string& name,
		RenderList& outList)
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
			if(! m_camera.frustum.intersects(bounds.center + glm::vec3(matrix[3]), bounds.radius)) {
				continue;
			}

			renderGeometry( model, g, matrix, 1.f, vehicle, outList);
		}
		break;
	}
}

void ObjectRenderer::renderVehicle(VehicleObject *vehicle,
								  RenderList& outList)
{
	RW_CHECK(vehicle->model, "Vehicle model is null");

	if(!vehicle->model) {
		return;
	}

	glm::mat4 matrixModel = vehicle->getTimeAdjustedTransform( m_renderAlpha );

	renderFrame(vehicle->model->resource,
				vehicle->model->resource->frames[0],
				matrixModel,
				vehicle,
				1.f,
				outList);

	// Draw wheels n' stuff
	for( size_t w = 0; w < vehicle->info->wheels.size(); ++w) {
		auto woi = m_world->data->findObjectType<ObjectData>(vehicle->vehicle->wheelModelID);
		if( woi ) {
			Model* wheelModel = m_world->data->models["wheels"]->resource;
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

				renderWheel(vehicle,
							wheelModel,
							wheelM,
							woi->modelName,
							outList);
			}
		}
	}
}

void ObjectRenderer::renderPickup(PickupObject *pickup,
								  RenderList& outList)
{
	if( ! pickup->isEnabled() ) return;

	glm::mat4 modelMatrix = glm::translate(glm::mat4(), pickup->getPosition());
	modelMatrix = glm::rotate(modelMatrix, m_world->getGameTime(), glm::vec3(0.f, 0.f, 1.f));

	auto odata = m_world->data->findObjectType<ObjectData>(pickup->getModelID());

	Model* model = nullptr;
	ModelFrame* itemModel = nullptr;

	/// @todo Better determination of is this object a weapon.
	if( odata->ID >= 170 && odata->ID <= 184 )
	{
		auto weapons = m_world->data->models["weapons"];
		if( weapons && weapons->resource && odata ) {
			model = weapons->resource;
			itemModel = weapons->resource->findFrame(odata->modelName + "_l0");
			RW_CHECK(itemModel, "Weapon Frame not present int weapon model");
			if ( ! itemModel )
			{
				return;
			}
		}
	}
	else
	{
		auto handle = m_world->data->models[odata->modelName];
		RW_CHECK( handle && handle->resource, "Pickup has no model");
		if ( handle && handle->resource )
		{
			model = handle->resource;
			itemModel = model->frames[model->rootFrameIdx];
		}
	}

	if ( itemModel ) {
		auto matrix = glm::inverse(itemModel->getTransform());
		renderFrame(model,
					itemModel,
					modelMatrix * matrix,
					pickup,
					1.f,
					outList);
	}
}

void ObjectRenderer::renderCutsceneObject(CutsceneObject *cutscene,
										  RenderList& outList)
{
	if(!m_world->state->currentCutscene) return;

	if(!cutscene->model->resource)
	{
		return;
	}

	glm::mat4 matrixModel;
	auto cutsceneOffset = m_world->state->currentCutscene->meta.sceneOffset + glm::vec3(0.f, 0.f, 1.f);

	if( cutscene->getParentActor() ) {
		matrixModel = glm::translate(matrixModel, cutsceneOffset);
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
		matrixModel = glm::translate(matrixModel, cutsceneOffset);
	}

	auto model = cutscene->model->resource;
	if( cutscene->getParentActor() ) {
		glm::mat4 align;
		/// @todo figure out where this 90 degree offset is coming from.
		align = glm::rotate(align, glm::half_pi<float>(), {0.f, 1.f, 0.f});
		renderFrame(model,
					model->frames[0],
					matrixModel * align,
					cutscene,
					1.f,
					outList);
	}
	else {
		renderFrame(model,
					model->frames[0],
					matrixModel,
					cutscene,
					1.f,
					outList);
	}
}

void ObjectRenderer::renderProjectile(ProjectileObject *projectile,
									  RenderList& outList)
{
	glm::mat4 modelMatrix = projectile->getTimeAdjustedTransform(m_renderAlpha);

	auto odata = m_world->data->findObjectType<ObjectData>(projectile->getProjectileInfo().weapon->modelID);
	auto weapons = m_world->data->models["weapons"];

	RW_CHECK(weapons, "Weapons model not loaded");

	if( weapons && weapons->resource ) {
		auto itemModel = weapons->resource->findFrame(odata->modelName + "_l0");
		auto matrix = glm::inverse(itemModel->getTransform());
		RW_CHECK(itemModel, "Weapon frame not in model");
		if(itemModel) {
			renderFrame(weapons->resource,
						itemModel,
						modelMatrix * matrix,
						projectile,
						1.f,
						outList);
		}
	}
}

void ObjectRenderer::buildRenderList(GameObject* object, RenderList& outList)
{
	if( object->skeleton )
	{
		object->skeleton->interpolate(m_renderAlpha);
	}

	// Right now specialized on each object type
	switch(object->type()) {
	case GameObject::Instance:
		renderInstance(static_cast<InstanceObject*>(object),
					   outList);
		break;
	case GameObject::Character:
		renderCharacter(static_cast<CharacterObject*>(object),
						outList);
		break;;
	case GameObject::Vehicle:
		renderVehicle(static_cast<VehicleObject*>(object),
					  outList);
		break;;
	case GameObject::Pickup:
		renderPickup(static_cast<PickupObject*>(object),
					 outList);
		break;
	case GameObject::Projectile:
		renderProjectile(static_cast<ProjectileObject*>(object),
						 outList);
		break;
	case GameObject::Cutscene:
		renderCutsceneObject(static_cast<CutsceneObject*>(object),
							 outList);
		break;
	default:
		break;
	}
}
