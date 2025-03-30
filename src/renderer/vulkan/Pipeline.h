#ifndef ENGINE_RENDERER_VULKAN_PIPELINE_H
#define ENGINE_RENDERER_VULKAN_PIPELINE_H

#include "core/PCH.h"

#include "renderer/vulkan/Context.h"
#include "renderer/vulkan/Renderpass.h"
#include "renderer/vulkan/Texture.h"

#include "util/Hashing.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {

    #define ENGINE_RENDERER_VULKAN_IMAGE_BINDING 0
    #define ENGINE_RENDERER_VULKAN_IMAGESAMPLER_BINDING 1
    #define ENGINE_RENDERER_VULKAN_UNIFORMBUFFER_BINDING 2

    class PipelineCreator;
    class Pipeline {
    public:

        void Init(PipelineCreator& info, const Context& context, const RenderPass& renderPass, const uint32_t subpass=0);
        void Cleanup(const Context& context);

        void BindSamplerDescriptor(const Context& context, const TextureSampler sampler, const uint32_t arrayElement);
        // Bind a texture and return the array id (also sets the id on the texture)
        uint32_t BindTextureDescriptor(const Context& context, Texture& texture);
        void UnbindTextureDescriptor(const Context& context, Texture& texture);

    private:
        friend class CommandBuffer;

        VkPipeline _pipeline;
        VkPipelineLayout _pipelineLayout;

        VkDescriptorSetLayout _descriptorSetLayout;
        VkDescriptorPool _descriptorPool;
        std::vector<VkDescriptorSet> _descriptorSets;

        std::queue<uint32_t> _availableTextureSlots;
    };

    namespace Vertex {
        typedef std::pair<VkFormat, uint32_t> Attribute;
        constexpr Attribute Float = Attribute(VK_FORMAT_R32_SFLOAT, 4);
        constexpr Attribute Vec2 = Attribute(VK_FORMAT_R32G32_SFLOAT, 8);
        constexpr Attribute Vec3 = Attribute(VK_FORMAT_R32G32B32_SFLOAT, 12);
        constexpr Attribute Vec4 = Attribute(VK_FORMAT_R32G32B32A32_SFLOAT, 16);
        constexpr Attribute Int = Attribute(VK_FORMAT_R32_SINT, 4);
        constexpr Attribute IVec2 = Attribute(VK_FORMAT_R32G32_SINT, 8);
        constexpr Attribute IVec3 = Attribute(VK_FORMAT_R32G32B32_SINT, 12);
        constexpr Attribute IVec4 = Attribute(VK_FORMAT_R32G32B32A32_SINT, 16);
        constexpr Attribute UInt = Attribute(VK_FORMAT_R32_UINT, 4);
        constexpr Attribute UVec2 = Attribute(VK_FORMAT_R32G32_UINT, 8);
        constexpr Attribute UVec3 = Attribute(VK_FORMAT_R32G32B32_UINT, 12);
        constexpr Attribute UVec4 = Attribute(VK_FORMAT_R32G32B32A32_UINT, 16);
        constexpr Attribute Double = Attribute(VK_FORMAT_R64_SFLOAT, 8);
        constexpr Attribute DVec2 = Attribute(VK_FORMAT_R64G64_SFLOAT, 26);
        constexpr Attribute DVec3 = Attribute(VK_FORMAT_R64G64B64_SFLOAT, 24);
        constexpr Attribute DVec4 = Attribute(VK_FORMAT_R64G64B64A64_SFLOAT, 32);
    }
    class PipelineCreator {
    public:

        PipelineCreator();

        void SetShaders(const std::initializer_list<const char*> fileLocations);
        void SetDynamicState(const std::initializer_list<VkDynamicState> dynamicState);
        void SetVertexInput(const std::initializer_list<Vertex::Attribute> perVertex);
        void SetInputAssembly(const VkPrimitiveTopology topology, const VkBool32 primitiveRestart=VK_FALSE);
        void SetViewport(const VkViewport viewport, const VkRect2D scissorRect);

        // duplicateSets is here to allow for frames in flight (set it equal to the amount of frames in flight you will be using)
        void SetDescriptorInfo(const uint32_t duplicateSets, const uint32_t textures, const uint32_t imageSamplers, const uint32_t uniformBuffers);

    private:
        friend class Pipeline;

        std::vector<std::vector<uint32_t>> _shaders;
        std::vector<VkShaderModuleCreateInfo> _shaderInfos;
        std::vector<VkPipelineShaderStageCreateInfo> _shaderStageInfos;

        VkPipelineDynamicStateCreateInfo _dynamicStateInfo{};
        std::vector<VkDynamicState> _dynamicState;

        VkPipelineVertexInputStateCreateInfo _vertexInputInfo{};

        VkVertexInputBindingDescription _vertexBinding;
        std::vector<VkVertexInputAttributeDescription> _vertexAttributes;

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

        std::vector<VkDescriptorSetLayoutBinding> _descriptorLayoutBindings;
        std::vector<VkDescriptorBindingFlagsEXT> _descriptorLayoutBindingFlags;
        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT _descriptorLayoutBindingExtraInfo{};

        VkDescriptorSetLayoutCreateInfo _descriptorLayoutInfo{};
        std::vector<VkDescriptorPoolSize> _descriptorPoolSizes;
        VkDescriptorPoolCreateInfo _descriptorPoolInfo{};
        uint32_t _amountTextures;

    };

}
}
}

#endif