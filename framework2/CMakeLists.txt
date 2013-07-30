SET(RENDERWARE_SOURCES
    # RenderWare related
    BinaryStream.cpp
    TextureArchive.cpp
    TextureLoader.cpp
    WeatherLoader.cpp

    # Game data related
    LoaderCOL.cpp
    LoaderIMG.cpp
    LoaderIPL.cpp
    LoaderIDE.cpp
    LoaderDFF.cpp
    LoaderIFP.cpp
    GTAData.cpp
    GTAObjects.cpp
    GTAAINode.cpp

    GTAEngine.cpp

    # Rendering
    GTARenderer.cpp
    DebugDraw.cpp
)

SET(RENDERWARE_HEADERS
    include/renderwure/BinaryStream.hpp
    include/renderwure/TextureArchive.hpp

    include/renderwure/loaders/LoaderDFF.hpp
    include/renderwure/loaders/TextureLoader.hpp
    include/renderwure/loaders/WeatherLoader.hpp
    include/renderwure/loaders/LoaderCOL.hpp
    include/renderwure/loaders/LoaderIMG.hpp
    include/renderwure/loaders/LoaderIPL.hpp
    include/renderwure/loaders/LoaderIDE.hpp
    include/renderwure/loaders/LoaderIFP.hpp

    include/renderwure/engine/GTAData.hpp
    include/renderwure/engine/GTAEngine.hpp
    include/renderwure/engine/GTATypes.hpp

    include/renderwure/engine/GTAObjects.hpp
    include/renderwure/ai/GTAAINode.hpp

    include/renderwure/render/GTARenderer.hpp
    include/renderwure/render/DebugDraw.hpp
    include/renderwure/render/ViewCamera.hpp
    include/renderwure/render/ViewFrustum.hpp
)

add_library(renderware
    ${RENDERWARE_SOURCES}
    ${RENDERWARE_HEADERS}
)

target_link_libraries(renderware sfml-window)

include_directories(include /usr/include/bullet)
