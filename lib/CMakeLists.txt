cmake_minimum_required (VERSION 3.8)

set(GLFW_BUILD_WAYLAND OFF)
find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})
set(GLFW_INCLUDE_VULKAN ON)
#set(GLFW_VULKAN_STATIC ON)
add_subdirectory("glfw")
add_subdirectory("entt")

set(SPIRV_SKIP_EXECUTABLES ON)
set(ENABLE_GLSLANG_BINARIES OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("glslang")

add_subdirectory("vma")