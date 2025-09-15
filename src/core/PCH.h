#include "util/Log.h"
#include "util/Math.h"

#define GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <entt/entt.hpp>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Include/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>

#include <vk_mem_alloc.h>

#include <stb_image.h>

#include <asio.hpp>

#include <string>
#include <vector>
#include <initializer_list>
#include <filesystem>
#include <fstream>
#include <queue>
#include <functional>

#define PI 3.141592653589793