#include "util/Log.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <entt/entt.hpp>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Include/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>

#include <string>
#include <vector>
#include <initializer_list>
#include <filesystem>
#include <fstream>