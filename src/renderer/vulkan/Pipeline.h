#ifndef ENGINE_RENDERER_VULKAN_PIPELINE_H
#define ENGINE_RENDERER_VULKAN_PIPELINE_H

#include "core/PCH.h"
#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/Renderpass.h"
#include "util/Hashing.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    class PipelineCreator;
    class Pipeline {
    public:

        void Init(PipelineCreator& info, const Context& context, const RenderPass& renderPass, const uint32_t subpass=0);
        void Cleanup(const Context& context);

    private:
        friend class CommandBuffer;

        VkPipeline _pipeline;
        VkPipelineLayout _pipelineLayout;
    };

    class PipelineCreator {
    public:

        PipelineCreator();

        void SetShaders(const std::initializer_list<const char*> fileLocations);
        void SetDynamicState(const std::initializer_list<VkDynamicState> dynamicState);
        void SetVertexInput();
        void SetInputAssembly(const VkPrimitiveTopology topology, const VkBool32 primitiveRestart=VK_FALSE);
        void SetViewport(const VkViewport viewport, const VkRect2D scissorRect);

    private:
        friend class Pipeline;

        std::vector<std::vector<uint32_t>> _shaders;
        std::vector<VkShaderModuleCreateInfo> _shaderInfos;
        std::vector<VkPipelineShaderStageCreateInfo> _shaderStageInfos;

        VkPipelineDynamicStateCreateInfo _dynamicStateInfo{};
        std::vector<VkDynamicState> _dynamicState;

        VkPipelineVertexInputStateCreateInfo _vertexInputInfo{};

        VkPipelineInputAssemblyStateCreateInfo _inputAssembly{};

        VkPipelineViewportStateCreateInfo _viewportState{};
        VkViewport _viewport{}; 
        VkRect2D _scissorRect{};

        VkPipelineRasterizationStateCreateInfo _rasterizer{};

        VkPipelineMultisampleStateCreateInfo _multisampling{};

        std::vector<VkPipelineColorBlendAttachmentState> _colorBlendAttachments{};
        VkPipelineColorBlendStateCreateInfo _colorBlending{};
        
        VkPipelineLayoutCreateInfo _pipelineLayoutInfo{};
        
        VkGraphicsPipelineCreateInfo _pipelineInfo{};

    };

}
}
}

#endif