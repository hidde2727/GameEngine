#include "util/BasicLog.h"// I would like to include Log.h, but this triggers a compiler front-end crash
#include "util/math/Math.h"

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

#include <forward_list>
#include <string>
#include <vector>
#include <map>
#include <flat_set>
#include <flat_map>
#include <queue>
#include <initializer_list>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <random>
#include <cctype>
#include <bit>
#include <charconv>
#include <typeindex>