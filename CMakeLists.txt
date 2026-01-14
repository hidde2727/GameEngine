cmake_minimum_required (VERSION 3.10)

add_library (GameEngine STATIC
"src/GameEngine.h"

"src/core/PCH.h"
"src/core/PCH.cpp"
"src/core/Entry.h"
"src/core/Entry.cpp"
"src/core/Game.h"
"src/core/Game.cpp"
"src/core/Scene.h"
"src/core/Scene.cpp"
"src/core/Components.h"
"src/core/Components.cpp"
"src/core/System.h"

"src/renderer/Window.h"
"src/renderer/Window.cpp"
"src/renderer/TextureMap.h"
"src/renderer/TextureMap.cpp"
"src/renderer/RectanglePacker.h"
"src/renderer/RectanglePacker.cpp"
"src/renderer/ImageLoader.h"
"src/renderer/ImageLoader.cpp"
"src/renderer/TextLoader.h"
"src/renderer/TextLoader.cpp"

"src/renderer/ttf/FontParser.h"
"src/renderer/ttf/FontParser.cpp"
"src/renderer/ttf/InstructionExecutor.h"
"src/renderer/ttf/InstructionExecutor.cpp"

"src/renderer/vulkan/Context.h"
"src/renderer/vulkan/Context.cpp"
"src/renderer/vulkan/Swapchain.h"
"src/renderer/vulkan/Swapchain.cpp"
"src/renderer/vulkan/Pipeline.h"
"src/renderer/vulkan/Pipeline.cpp"
"src/renderer/vulkan/Renderpass.h"
"src/renderer/vulkan/Renderpass.cpp"
"src/renderer/vulkan/CommandBuffer.h"
"src/renderer/vulkan/CommandBuffer.cpp"
"src/renderer/vulkan/Buffers.h"
"src/renderer/vulkan/Buffers.cpp"
"src/renderer/vulkan/Texture.h"
"src/renderer/vulkan/Texture.cpp"

"src/network/WebHandler.h"
"src/network/WebHandler.cpp"
"src/network/WebsocketHandler.h"
"src/network/WebsocketRouter.h"

"src/network/HTTP/Request.h"
"src/network/HTTP/Request.cpp"
"src/network/HTTP/Response.h"
"src/network/HTTP/Response.cpp"
"src/network/HTTP/Connection.h"
"src/network/HTTP/Connection.cpp"
"src/network/HTTP/Router.h"
"src/network/HTTP/Router.cpp"
"src/network/HTTP/SessionStorage.h"
"src/network/HTTP/SessionStorage.cpp"

"src/network/websocket/Frame.h"
"src/network/websocket/Frame.cpp"
"src/network/websocket/Connection.h"
"src/network/websocket/Connection.cpp"
"src/network/websocket/BasicHandler.h"

"src/physics/Components.h"
"src/physics/Components.cpp"
"src/physics/Engine.h"
"src/physics/Engine.cpp"
"src/physics/CollisionManifold.h"
"src/physics/CollisionManifold.cpp"
"src/physics/Shapes.h"
"src/physics/Shapes.cpp"
"src/physics/QuadTree.h"
"src/physics/AABB.h"

"src/util/Log.h"
"src/util/Log.cpp"
"src/util/Hashing.h"
"src/util/Hashing.cpp"
"src/util/TemplateConcepts.h"
"src/util/BitMask.h"
"src/util/Endianess.h"
"src/util/Endianess.cpp"
"src/util/DebugGraphics.h"
"src/util/DebugGraphics.cpp"
"src/util/FileManager.h"
"src/util/FileManager.cpp"
"src/util/WeirdPointer.h"
"src/util/VectorStreamBuffer.h"
"src/util/Reflection.h"
"src/util/Strings.h"

"src/util/math/Vec2D.h"
"src/util/math/Vec3D.h"
"src/util/math/Area.h"
"src/util/math/EquationSolvers.h"
"src/util/math/Math.h"
"src/util/math/PI.h"
"src/util/math/Matrix.h"

"src/util/serialization/Serialization.h"
"src/util/serialization/JSON.h"
"src/util/serialization/Binary.h"
"src/util/serialization/ClassStructure.h"
)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_precompile_headers(GameEngine PUBLIC "src/core/PCH.h")

set(GameEngineFlags -freflection -std=c++26 -stdlib=libc++ -freflection-latest)
set(GameEngineLibraryFlags -stdlib=libc++)
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message("Enabling reflection")
    target_compile_options(GameEngine PUBLIC ${GameEngineFlags})
    target_link_options(GameEngine PUBLIC ${GameEngineFlags})
ELSE()
    message("This CMakeLists.txt does not now how to active reflection in the currently selected kit")
ENDIF()

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(GameEngine PRIVATE Vulkan::Vulkan)
target_include_directories(GameEngine PUBLIC ${Vulkan_INCLUDE_DIRS})

add_subdirectory ("lib")

# GLFW
target_link_libraries(GameEngine PRIVATE glfw)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/include")

# GLSL compiler
target_link_libraries(GameEngine PRIVATE glslang glslang-default-resource-limits)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/glslang")

# Asio
add_library(asio INTERFACE)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
target_include_directories(asio INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/asio/asio/include")
find_package(Threads)
target_link_libraries(asio INTERFACE Threads::Threads)
target_link_libraries(GameEngine PRIVATE asio)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/asio/asio/include")

# EnTT
target_link_libraries(GameEngine PRIVATE EnTT)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/entt/src")

# STB Images
add_library(stb-images INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb/stb_image.h")
target_include_directories(stb-images INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb")
target_link_libraries(GameEngine PRIVATE stb-images)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb")

# VMA
target_link_libraries(GameEngine PUBLIC GPUOpen::VulkanMemoryAllocator)

# Documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(Doxyfile.in Doxyfile)
  doxygen_add_docs(
    documentation
    ALL
    USE_STAMP_FILE
    COMMENT "Generating project documentation with custom Doxyfile"
    CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  )
else(DOXYGEN_FOUND)

endif(DOXYGEN_FOUND)