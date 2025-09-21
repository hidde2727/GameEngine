cmake_minimum_required (VERSION 3.10)

set(GLFW_BUILD_WAYLAND ON)
set(GLFW_INCLUDE_VULKAN ON)
#set(GLFW_VULKAN_STATIC ON)
add_subdirectory("glfw")
add_subdirectory("entt")

set(SPIRV_SKIP_EXECUTABLES ON)
set(ENABLE_GLSLANG_BINARIES OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("glslang")

add_subdirectory("vma")