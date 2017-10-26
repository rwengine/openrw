###########################################################
##      RWENGINE
###########################################################

set(RWENGINE_SOURCES
    src/BinaryStream.cpp
    src/BinaryStream.hpp
    src/TextureArchive.cpp
    src/TextureArchive.hpp

    src/ai/AIGraph.cpp
    src/ai/AIGraph.hpp
    src/ai/AIGraphNode.cpp
    src/ai/AIGraphNode.hpp
    src/ai/CharacterController.cpp
    src/ai/CharacterController.hpp
    src/ai/DefaultAIController.cpp
    src/ai/DefaultAIController.hpp
    src/ai/PlayerController.cpp
    src/ai/PlayerController.hpp
    src/ai/TrafficDirector.cpp
    src/ai/TrafficDirector.hpp

    src/audio/SoundManager.cpp
    src/audio/SoundManager.hpp
    src/audio/alCheck.cpp
    src/audio/alCheck.hpp

    src/core/Logger.cpp
    src/core/Logger.hpp
    src/core/Profiler.cpp
    src/core/Profiler.hpp

    src/data/AnimGroup.cpp
    src/data/AnimGroup.hpp
    src/data/Chase.cpp
    src/data/Chase.hpp
    src/data/CollisionModel.hpp
    src/data/CutsceneData.hpp
    src/data/GameTexts.cpp
    src/data/GameTexts.hpp
    src/data/InstanceData.hpp
    src/data/ModelData.cpp
    src/data/ModelData.hpp
    src/data/PathData.hpp
    src/data/PedData.cpp
    src/data/PedData.hpp
    src/data/WeaponData.hpp
    src/data/ZoneData.hpp

    src/dynamics/CollisionInstance.cpp
    src/dynamics/CollisionInstance.hpp
    src/dynamics/RaycastCallbacks.hpp

    src/engine/Animator.cpp
    src/engine/Animator.hpp
    src/engine/GameData.cpp
    src/engine/GameData.hpp
    src/engine/GameState.cpp
    src/engine/GameState.hpp
    src/engine/GameWorld.cpp
    src/engine/GameWorld.hpp
    src/engine/SaveGame.cpp
    src/engine/SaveGame.hpp
    src/engine/ScreenText.cpp
    src/engine/ScreenText.hpp

    src/items/Weapon.cpp
    src/items/Weapon.hpp

    src/loaders/GenericDATLoader.cpp
    src/loaders/GenericDATLoader.hpp
    src/loaders/LoaderCOL.cpp
    src/loaders/LoaderCOL.hpp
    src/loaders/LoaderCutsceneDAT.cpp
    src/loaders/LoaderCutsceneDAT.hpp
    src/loaders/LoaderGXT.cpp
    src/loaders/LoaderGXT.hpp
    src/loaders/LoaderIDE.cpp
    src/loaders/LoaderIDE.hpp
    src/loaders/LoaderIFP.cpp
    src/loaders/LoaderIFP.hpp
    src/loaders/LoaderIPL.cpp
    src/loaders/LoaderIPL.hpp
    src/loaders/WeatherLoader.cpp
    src/loaders/WeatherLoader.hpp

    src/objects/CharacterObject.cpp
    src/objects/CharacterObject.hpp
    src/objects/CutsceneObject.cpp
    src/objects/CutsceneObject.hpp
    src/objects/GameObject.cpp
    src/objects/GameObject.hpp
    src/objects/InstanceObject.cpp
    src/objects/InstanceObject.hpp
    src/objects/ItemPickup.cpp
    src/objects/ItemPickup.hpp
    src/objects/ObjectTypes.hpp
    src/objects/PickupObject.cpp
    src/objects/PickupObject.hpp
    src/objects/ProjectileObject.cpp
    src/objects/ProjectileObject.hpp
    src/objects/VehicleInfo.hpp
    src/objects/VehicleObject.cpp
    src/objects/VehicleObject.hpp

    src/render/DebugDraw.cpp
    src/render/DebugDraw.hpp
    src/render/GameRenderer.cpp
    src/render/GameRenderer.hpp
    src/render/GameShaders.cpp
    src/render/GameShaders.hpp
    src/render/MapRenderer.cpp
    src/render/MapRenderer.hpp
    src/render/ObjectRenderer.cpp
    src/render/ObjectRenderer.hpp
    src/render/OpenGLRenderer.cpp
    src/render/OpenGLRenderer.hpp
    src/render/TextRenderer.cpp
    src/render/TextRenderer.hpp
    src/render/ViewCamera.hpp
    src/render/ViewFrustum.hpp
    src/render/VisualFX.cpp
    src/render/VisualFX.hpp
    src/render/WaterRenderer.cpp
    src/render/WaterRenderer.hpp

    src/script/SCMFile.cpp
    src/script/SCMFile.hpp
    src/script/ScriptDisassembly.cpp
    src/script/ScriptDisassembly.hpp
    src/script/ScriptFunctions.cpp
    src/script/ScriptFunctions.hpp
    src/script/ScriptMachine.cpp
    src/script/ScriptMachine.hpp
    src/script/ScriptModule.cpp
    src/script/ScriptModule.hpp
    src/script/ScriptTypes.cpp
    src/script/ScriptTypes.hpp
    src/script/modules/GTA3Module.cpp
    src/script/modules/GTA3Module.hpp
    )

add_library(rwengine
    ${RWENGINE_SOURCES}
)

target_link_libraries(rwengine
    PUBLIC
        rwlib
        bullet::bullet
        ffmpeg::ffmpeg
        glm::glm
        OpenAL::OpenAL
    )

target_include_directories(rwengine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
