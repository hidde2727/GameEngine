cmake_minimum_required (VERSION 3.10)

set(GLFW_BUILD_WAYLAND ON)
set(GLFW_INCLUDE_VULKAN ON)
#set(GLFW_VULKAN_STATIC ON)
add_subdirectory("glfw")
add_subdirectory("entt")

set(SPIRV_SKIP_EXECUTABLES ON)
set(ENABLE_GLSLANG_BINARIES OFF)
set(BUILD_SHARED_LIBS OFF)
set(GLSLANG_TESTS_DEFAULT OFF)
set(GLSLANG_ENABLE_INSTALL_DEFAULT OFF)
set(ENABLE_OPT OFF)
add_compile_options(${GameEngineLibraryFlags})
add_link_options(${GameEngineLibraryFlags})
add_subdirectory("glslang")

add_subdirectory("vma")