cmake_minimum_required (VERSION 3.8)

add_subdirectory("glfw")
add_subdirectory("entt")

set(SPIRV_SKIP_EXECUTABLES ON)
set(ENABLE_GLSLANG_BINARIES OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("glslang")

add_subdirectory("vma")