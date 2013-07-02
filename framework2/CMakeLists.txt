add_library(renderware 
	
	# RenderWare related
	BinaryStream.cpp 
	TextureArchive.cpp
	LoaderDFF.cpp
	TextureLoader.cpp
	
	# Game data related
	LoaderCOL.cpp
	LoaderIMG.cpp
	LoaderIPL.cpp
	LoaderIDE.cpp
	GTAData.cpp
	
	GTAEngine.cpp
	
	# Rendering
	GTARenderer.cpp
)

target_link_libraries(renderware sfml-window)

include_directories(include)
