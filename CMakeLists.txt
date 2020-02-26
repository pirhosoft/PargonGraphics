set(MODULE_NAME Graphics)
set(TARGET_NAME Pargon${MODULE_NAME})
set(MAIN_HEADER Include/Pargon/${MODULE_NAME}.h)

#Graphics.DirectX11.h

set(PUBLIC_HEADERS
	Include/Pargon/Graphics/Geometry.h
	Include/Pargon/Graphics/GraphicsDevice.h
	Include/Pargon/Graphics/GraphicsResource.h
	Include/Pargon/Graphics/Material.h
	Include/Pargon/Graphics/Renderer.h
	Include/Pargon/Graphics/Texture.h
)

set(SOURCES
	Source/Core/Geometry.cpp
	Source/Core/GraphicsDevice.cpp
	Source/Core/GraphicsResource.cpp
	Source/Core/Material.cpp
	Source/Core/Renderer.cpp
	Source/Core/Texture.cpp
)

set(DEPENDENCIES
	PargonApplication
	PargonContainers
	PargonFiles
	PargonMath
	PargonSerialization
)

set(DIRECTX11_SOURCES
	Source/DirectX11/DirectX11Geometry.cpp
	Source/DirectX11/DirectX11Geometry.h
	Source/DirectX11/DirectX11Material.cpp
	Source/DirectX11/DirectX11Material.h
	Source/DirectX11/DirectX11Renderer.cpp
	Source/DirectX11/DirectX11Renderer.h
	Source/DirectX11/DirectX11Texture.cpp
	Source/DirectX11/DirectX11Texture.h
)

include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX(d3d11.h HAS_DIRECTX11)

if(HAS_DIRECTX11)
	list(APPEND SOURCES ${DIRECTX11_SOURCES})
endif()

add_subdirectory(Dependencies/zlib)
add_subdirectory(Dependencies/libpng)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Include/Pargon/" FILES ${MAIN_HEADER})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Include/Pargon/${MODULE_NAME}/" PREFIX Include FILES ${PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source/" FILES ${SOURCES})

add_library(${TARGET_NAME} STATIC)
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_include_directories(${TARGET_NAME} PUBLIC Include)
target_include_directories(${TARGET_NAME} PRIVATE Source)
target_link_libraries(${TARGET_NAME} PUBLIC ${DEPENDENCIES})
target_link_libraries(${TARGET_NAME} PRIVATE libpng zlib)
target_sources(${TARGET_NAME} PRIVATE "${MAIN_HEADER}" "${PUBLIC_HEADERS}" "${SOURCES}")
