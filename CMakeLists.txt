cmake_minimum_required (VERSION 3.8)

add_library (GameEngine STATIC
"src/GameEngine.h"

"src/core/PCH.h"
"src/core/Entry.h"
"src/core/Entry.cpp"
"src/core/Game.h"
"src/core/Game.cpp"
"src/core/Scene.h"
"src/core/Scene.cpp"

"src/renderer/Window.h"
"src/renderer/Window.cpp"
"src/renderer/vulkan/Context.h"
"src/renderer/vulkan/Context.cpp"
"src/renderer/vulkan/Swapchain.h"
"src/renderer/vulkan/Swapchain.cpp"

"src/util/Log.h"
"src/util/Log.cpp"
)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_precompile_headers(GameEngine PUBLIC "src/core/PCH.h")

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(GameEngine PRIVATE Vulkan::Vulkan)
target_include_directories(GameEngine PUBLIC ${Vulkan_INCLUDE_DIRS})

add_subdirectory ("lib")

# GLFW
target_link_libraries(GameEngine PRIVATE glfw)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/include")

# Asio
add_library(asio INTERFACE)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
target_include_directories(asio INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/asio/asio/include/asio")
find_package(Threads)
target_link_libraries(asio INTERFACE Threads::Threads)

# EnTT
target_link_libraries(GameEngine PRIVATE EnTT)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/entt/src")

# STB Images
add_library(stb-images INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb/stb_image.h")
target_include_directories(stb-images INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb")
target_link_libraries(GameEngine PRIVATE stb-images)
target_include_directories(GameEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb")