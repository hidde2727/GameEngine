#ifndef ENGINE_RENDERER_VULKAN_PIPELINE_H
#define ENGINE_RENDERER_VULKAN_PIPELINE_H

#include "core/PCH.h"
#include "renderer/vulkan/Context.h"
#include "util/Hashing.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    class PipelineCreator;
    class Pipeline {
    public:

        void Init(PipelineCreator info, const Context context);
        void Cleanup(const Context context);

    private:

    };

    class PipelineCreator {
    public:

        void SetShaders(const std::initializer_list<const char*> fileLocations);

    private:
        friend class Pipeline;

        std::vector<std::vector<uint32_t>> _shaders;
        std::vector<VkShaderModuleCreateInfo> _shaderInfos;
        std::vector<VkPipelineShaderStageCreateInfo> _shaderStageInfos;

    };

}
}
}

#endif