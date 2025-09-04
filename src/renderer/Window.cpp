#include "renderer/window.h"
    #include <bitset>

namespace Engine {
namespace Renderer {
    
    void Window::Init(const uint32_t textureMapSlots, const std::string engineResourceDirectory) {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, FramebufferResize);

        Vulkan::ContextCreationInfo contextInfo;
        contextInfo.SetExtensions({}, true);
        contextInfo.SetValidationLayers({"VK_LAYER_KHRONOS_validation"}, &DebugCallback);
        contextInfo.SetNeccesaryQueues({
            Vulkan::QueueType::GraphicsQueue,
            Vulkan::QueueType::KHRPresentQueue
        });
        contextInfo.SetDeviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
        _vkContext.Init(contextInfo, _window);
        

        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);
        _framebufferSize.x = (float)width;
        _framebufferSize.y = (float)height;
        _vkSwapchain.Init(_vkContext, (uint32_t)width, (uint32_t)height);
#if ENGINE_ENABLE_DEBUG_GRAPHICS
        _vkRenderPass.Init(_vkContext, _vkSwapchain, 3);
#else
        _vkRenderPass.Init(_vkContext, _vkSwapchain, 2);
#endif
        _vkSwapchain.Resize(_vkContext, _vkRenderPass, width, height);

        // Rect pipeline
        Vulkan::PipelineCreator rectPipelineInfo;
        rectPipelineInfo.SetShaders({ (engineResourceDirectory+"/shaders/rect.vert").c_str(), (engineResourceDirectory+"/shaders/rect.frag").c_str() }, "resources/engine/");
        rectPipelineInfo.SetVertexInput({Vulkan::Vertex::UInt }, { Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec3, Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec2, Vulkan::Vertex::UInt });
        rectPipelineInfo.SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        rectPipelineInfo.SetDescriptorInfo(2, 16, 2, 0);
        rectPipelineInfo.SetPushConstantInput({ Vulkan::Vertex::Vec2 }, VK_SHADER_STAGE_VERTEX_BIT);
        rectPipelineInfo.EnableAlphaBlending();
        _vkRectPipeline.Init(rectPipelineInfo, _vkContext, _vkRenderPass, 0);

        // Text pipeline
        Vulkan::PipelineCreator textPipelineInfo;
        textPipelineInfo.SetShaders({ (engineResourceDirectory+"/shaders/text.vert").c_str(), (engineResourceDirectory+"/shaders/text.frag").c_str() }, "resources/engine/");
        textPipelineInfo.SetVertexInput({Vulkan::Vertex::Vec2 }, { Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec3, Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec2, Vulkan::Vertex::UInt, Vulkan::Vertex::Float });
        textPipelineInfo.SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        textPipelineInfo.SetDescriptorInfo(2, 16, 2, 0);
        textPipelineInfo.SetPushConstantInput({ Vulkan::Vertex::Vec2 }, VK_SHADER_STAGE_VERTEX_BIT);
        textPipelineInfo.EnableAlphaBlending();
        _vkTextPipeline.Init(textPipelineInfo, _vkContext, _vkRenderPass, 1);

        _vkCommandBuffer.Init(_vkContext, Vulkan::QueueType::GraphicsQueue, 2);
        _vkInFlightFence = _vkCommandBuffer.CreateFence(_vkContext, true);
        _vkImageAvailableSemaphore = _vkCommandBuffer.CreateSemaphore(_vkContext);
        _vkRenderFinishedSemaphore = _vkCommandBuffer.CreateSemaphore(_vkContext);

        VertexDataRect rectangleData[] = { VertexDataRect(0), VertexDataRect(1), VertexDataRect(2), VertexDataRect(3) };
        _vkRectPerVertexBuffer.Init(_vkContext, sizeof(VertexDataRect) * 4);
        _vkRectPerVertexBuffer.SetData(_vkContext, rectangleData);

        VertexDataText textRectangleData[] = { VertexDataText({0,0}), VertexDataText({1,0}), VertexDataText({0,1}), VertexDataText({1,1}) };
        _vkTextPerVertexBuffer.Init(_vkContext, sizeof(VertexDataText) * 4);
        _vkTextPerVertexBuffer.SetData(_vkContext, textRectangleData);

        _vkRectVertexBuffer.Init(_vkContext, sizeof(InstanceDataRect) * 4);
        _vkTextVertexBuffer.Init(_vkContext, sizeof(InstanceDataText) * 4);

        Vulkan::TransferBuffer indexTransfer;
        indexTransfer.Init(_vkContext, 6*sizeof(uint16_t));
        _vkIndexBuffer.Init(_vkContext, 6*sizeof(uint16_t), true);
        indexTransfer.SetData<uint16_t>(_vkContext, {0, 1, 2, 2, 1, 3});
        indexTransfer.CopyTo(_vkContext, &_vkIndexBuffer);
        indexTransfer.Cleanup(_vkContext);
        
        _textureMaps.resize(textureMapSlots);
        
        _pixelSampler.Init(_vkContext, VK_FILTER_NEAREST, VK_FILTER_NEAREST);
        _vkRectPipeline.BindSamplerDescriptor(_vkContext, _pixelSampler, 0);
        _vkTextPipeline.BindSamplerDescriptor(_vkContext, _pixelSampler, 0);
        _linearSampler.Init(_vkContext, VK_FILTER_LINEAR, VK_FILTER_LINEAR);
        _vkRectPipeline.BindSamplerDescriptor(_vkContext, _linearSampler, 1);
        _vkTextPipeline.BindSamplerDescriptor(_vkContext, _linearSampler, 1);

        // Debug pipeline
#if ENGINE_ENABLE_DEBUG_GRAPHICS
        Vulkan::PipelineCreator debugPipelineInfo;
        debugPipelineInfo.SetShaders({ (engineResourceDirectory+"/shaders/debug.vert").c_str(), (engineResourceDirectory+"/shaders/debug.frag").c_str() }, "resources/engine/");
        debugPipelineInfo.SetVertexInput({ Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec3 });
        debugPipelineInfo.SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        debugPipelineInfo.SetDescriptorInfo(2, 0, 0, 0);
        debugPipelineInfo.SetPushConstantInput({ Vulkan::Vertex::Vec2 }, VK_SHADER_STAGE_VERTEX_BIT);
        debugPipelineInfo.SetInputAssembly(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 0);
        debugPipelineInfo.EnableAlphaBlending();
        _vkDebugPipeline.Init(debugPipelineInfo, _vkContext, _vkRenderPass, 2);

        _vkDebugVertexBuffer.Init(_vkContext, 1);
#endif
    }
    void Window::Cleanup() {
        _vkContext.WaitIdle();

        _pixelSampler.Cleanup(_vkContext);
        _linearSampler.Cleanup(_vkContext);

        for(TextureMap& textureMap : _textureMaps) {
            textureMap.Cleanup(_vkContext, { &_vkRectPipeline, &_vkTextPipeline });
        }
        _vkIndexBuffer.Cleanup(_vkContext);
        _vkRectPerVertexBuffer.Cleanup(_vkContext);
        _vkRectVertexBuffer.Cleanup(_vkContext);
        _vkTextPerVertexBuffer.Cleanup(_vkContext);
        _vkTextVertexBuffer.Cleanup(_vkContext);

#if ENGINE_ENABLE_DEBUG_GRAPHICS
        _vkDebugPipeline.Cleanup(_vkContext);
        _vkDebugVertexBuffer.Cleanup(_vkContext);
#endif

        _vkCommandBuffer.Cleanup(_vkContext);
        _vkRectPipeline.Cleanup(_vkContext);
        _vkTextPipeline.Cleanup(_vkContext);
        _vkSwapchain.Cleanup(_vkContext);
        _vkRenderPass.Cleanup(_vkContext);
        _vkContext.CleanUp();

        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(_window);
    }
    void Window::Update() {
        glfwPollEvents();
    }
    void Window::Draw(entt::registry& registry, const uint32_t amountRectangles, const uint32_t amountText) {
        if(_framebufferResized) {
            _framebufferResized = false; 
            int width = 0, height = 0;
            glfwGetFramebufferSize(_window, &width, &height);
            _framebufferSize.x = (float)width;
            _framebufferSize.y = (float)height;
            if(width==0 || height==0) return;
            _vkSwapchain.Resize(_vkContext, _vkRenderPass, width, height);
        }
        if(_framebufferSize.x == 0 || _framebufferSize.y == 0) return;

        _vkRectVertexBuffer.Resize(_vkContext, amountRectangles*sizeof(InstanceDataRect));
        _vkTextVertexBuffer.Resize(_vkContext, amountText*sizeof(InstanceDataText));

        {// Rectangle data
			auto group = registry.group<Component::Texture>(entt::get<Component::Position>);
            _vkRectVertexBuffer.StartTransferingData(_vkContext);
			for (const auto [entity, texture, pos] : group.each()) {
				_vkRectVertexBuffer.AddData(InstanceDataRect(
                    pos.GetPrecalculated(texture._size.x, texture._size.y),
                    Util::Vec3F(1.f, 1.f, 1.f),
                    Util::Vec2F(texture._textureArea.x, texture._textureArea.y),
                    Util::Vec2F(texture._textureArea.w, texture._textureArea.h),
                    texture._descriptorID
                ));
			}
            _vkRectVertexBuffer.EndTransferingData(_vkContext, _vkCommandBuffer);
		}
        {// Text data
			auto group = registry.group<Component::Text>(entt::get<Component::Position>);
            _vkTextVertexBuffer.StartTransferingData(_vkContext);
			for (const auto [entity, text, pos] : group.each()) {
                float x = pos._pos.x;
                float y = pos._pos.y;
            for(const auto renderInfo : text._renderInfo) {
                x = pos._pos.x + renderInfo._position.x;
                y = pos._pos.y + renderInfo._position.y;
                _vkTextVertexBuffer.AddData(InstanceDataText(
                    Util::Vec2F(x , y),
                    Util::Vec2F(renderInfo._position.w , renderInfo._position.h),
                    Util::Vec3F(1.f, 1.f, 1.f),
                    Util::Vec2F(renderInfo._textureArea.x, renderInfo._textureArea.y), 
                    Util::Vec2F(renderInfo._textureArea.w, renderInfo._textureArea.h),
                    renderInfo._descriptorID,
                    renderInfo._pxRange
                ));
            }
			}
            _vkTextVertexBuffer.EndTransferingData(_vkContext, _vkCommandBuffer);
		}

        _vkCommandBuffer.AcquireNextSwapchainFrame(_vkContext, _vkSwapchain, _vkImageAvailableSemaphore);
        _vkCommandBuffer.WaitFence(_vkContext, _vkInFlightFence);

        _vkCommandBuffer.StartRecording(_vkContext);
        _vkCommandBuffer.BeginRenderPass(_vkRenderPass, _vkSwapchain, {{{0,0,0,1.f}}}, true);

        _vkCommandBuffer.BindGraphicsPipeline(_vkRectPipeline);
        _vkCommandBuffer.SetPushConstantData(_vkRectPipeline, _framebufferSize, VK_SHADER_STAGE_VERTEX_BIT);
        _vkCommandBuffer.BindDescriptorSet(_vkRectPipeline);
        _vkCommandBuffer.BindVertexBuffer(_vkRectPerVertexBuffer, 0);
        _vkCommandBuffer.BindVertexBuffer(_vkRectVertexBuffer, 1);
        _vkCommandBuffer.BindIndexBuffer(_vkIndexBuffer);
        _vkCommandBuffer.DrawIndexed(6, amountRectangles);

        _vkCommandBuffer.NextSubPass();

        _vkCommandBuffer.BindGraphicsPipeline(_vkTextPipeline);
        _vkCommandBuffer.SetPushConstantData(_vkTextPipeline, _framebufferSize, VK_SHADER_STAGE_VERTEX_BIT);
        _vkCommandBuffer.BindDescriptorSet(_vkTextPipeline);
        _vkCommandBuffer.BindVertexBuffer(_vkTextPerVertexBuffer, 0);
        _vkCommandBuffer.BindVertexBuffer(_vkTextVertexBuffer, 1);
        _vkCommandBuffer.BindIndexBuffer(_vkIndexBuffer);
        _vkCommandBuffer.DrawIndexed(6, amountText);

#if ENGINE_ENABLE_DEBUG_GRAPHICS
        _vkDebugVertexBuffer.Resize(_vkContext, (uint32_t)(_debugLines.size()*sizeof(DebugLine)));
        _vkDebugVertexBuffer.SetData(_vkContext, _debugLines);

        _vkCommandBuffer.NextSubPass();

        _vkCommandBuffer.BindGraphicsPipeline(_vkDebugPipeline);
        _vkCommandBuffer.SetPushConstantData(_vkDebugPipeline, _framebufferSize, VK_SHADER_STAGE_VERTEX_BIT);
        _vkCommandBuffer.BindVertexBuffer(_vkDebugVertexBuffer, 0);
        _vkCommandBuffer.Draw((int)_debugLines.size()*2, 1);
        _debugLines.clear();
#endif

        _vkCommandBuffer.EndRenderPass();
        _vkCommandBuffer.EndRecording();

        _vkCommandBuffer.Submit(
            _vkContext, 
            { {_vkImageAvailableSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT} }, 
            {_vkRenderFinishedSemaphore}, 
            _vkInFlightFence
        );
        _vkCommandBuffer.PresentResult(_vkContext, _vkSwapchain, { _vkRenderFinishedSemaphore });
        _vkContext.WaitIdle();
    }

    
    void Window::StartAssetLoading(const size_t textureMapID) {
        _textureMaps[textureMapID].StartLoading();
    }
    void Window::SetAssetLoadingCacheName(const size_t textureMapID, const std::string cacheName) {
        _textureMaps[textureMapID].SetCacheName(cacheName);
    }
    AssetID Window::AddAsset(const size_t textureMapID, std::unique_ptr<AssetLoader> assetLoader, const uint32_t assetTypeID) {
        return (AssetID)(
            (textureMapID << ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS) | 
            (assetTypeID << ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS) | 
            _textureMaps[textureMapID].AddTextureLoader(std::move(assetLoader))
        );
    }
    void Window::EndAssetLoading(const size_t textureMapID) {
        _textureMaps[textureMapID].EndLoading(_vkContext, { &_vkRectPipeline, &_vkTextPipeline }, _engineResourceDirectory);
    }
    void Window::CleanupAssets(const size_t textureMapID) {
        _textureMaps[textureMapID].Cleanup(_vkContext, { &_vkRectPipeline, &_vkTextPipeline });
    }

    std::shared_ptr<ImageRenderInfo> Window::GetTextureInfo(AssetID asset) {
        if(Util::GetBits<uint32_t>(asset, ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS, ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS) != ENGINE_RENDERER_ASSETTYPE_TEXTURE) 
            THROW("Cannot acces an asset that is not a texture with the GetTextureInfo function")
        return reinterpret_pointer_cast<ImageRenderInfo>(
            _textureMaps[asset >> ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS].GetRenderInfo(asset & (0xFFFFFFFF >> ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS))
        );
    }
    std::shared_ptr<TextRenderInfo> Window::GetTextInfo(AssetID asset) {
        if(Util::GetBits<uint32_t>(asset, ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS, ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS) != ENGINE_RENDERER_ASSETTYPE_TEXT) 
            THROW("Cannot acces an asset that is not text with the GetTextInfo function")
        return reinterpret_pointer_cast<TextRenderInfo>(
            _textureMaps[asset >> ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS].GetRenderInfo(asset & (0xFFFFFFFF >> ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS))
        );
    }

    std::string Window::GetVulkanDeviceLimits() {
        VkPhysicalDeviceProperties properties = _vkContext.GetPhysicalDeviceProperties();
        return
        "maxImageDimension1D: " + std::to_string(properties.limits.maxImageDimension1D) +
        "\nmaxImageDimension2D: " + std::to_string(properties.limits.maxImageDimension2D) +
        "\nmaxImageDimension3D: " + std::to_string(properties.limits.maxImageDimension3D) +
        "\nmaxImageDimensionCube: " + std::to_string(properties.limits.maxImageDimensionCube) +
        "\nmaxImageArrayLayers: " + std::to_string(properties.limits.maxImageArrayLayers) +
        "\nmaxTexelBufferElements: " + std::to_string(properties.limits.maxTexelBufferElements) +
        "\nmaxUniformBufferRange: " + std::to_string(properties.limits.maxUniformBufferRange) +
        "\nmaxStorageBufferRange: " + std::to_string(properties.limits.maxStorageBufferRange) +
        "\nmaxPushConstantsSize: " + std::to_string(properties.limits.maxPushConstantsSize) +
        "\nmaxMemoryAllocationCount: " + std::to_string(properties.limits.maxMemoryAllocationCount) +
        "\nmaxSamplerAllocationCount: " + std::to_string(properties.limits.maxSamplerAllocationCount) +
        "\nbufferImageGranularity: " + std::to_string(properties.limits.bufferImageGranularity) +
        "\nsparseAddressSpaceSize: " + std::to_string(properties.limits.sparseAddressSpaceSize) +
        "\nmaxBoundDescriptorSets: " + std::to_string(properties.limits.maxBoundDescriptorSets) +
        "\nmaxPerStageDescriptorSamplers: " + std::to_string(properties.limits.maxPerStageDescriptorSamplers) +
        "\nmaxPerStageDescriptorUniformBuffers: " + std::to_string(properties.limits.maxPerStageDescriptorUniformBuffers) +
        "\nmaxPerStageDescriptorStorageBuffers: " + std::to_string(properties.limits.maxPerStageDescriptorStorageBuffers) +
        "\nmaxPerStageDescriptorSampledImages: " + std::to_string(properties.limits.maxPerStageDescriptorSampledImages) +
        "\nmaxPerStageDescriptorStorageImages: " + std::to_string(properties.limits.maxPerStageDescriptorStorageImages) +
        "\nmaxPerStageDescriptorInputAttachments: " + std::to_string(properties.limits.maxPerStageDescriptorInputAttachments) +
        "\nmaxPerStageResources: " + std::to_string(properties.limits.maxPerStageResources) +
        "\nmaxDescriptorSetSamplers: " + std::to_string(properties.limits.maxDescriptorSetSamplers) +
        "\nmaxDescriptorSetUniformBuffers: " + std::to_string(properties.limits.maxDescriptorSetUniformBuffers) +
        "\nmaxDescriptorSetUniformBuffersDynamic: " + std::to_string(properties.limits.maxDescriptorSetUniformBuffersDynamic) +
        "\nmaxDescriptorSetStorageBuffers: " + std::to_string(properties.limits.maxDescriptorSetStorageBuffers) +
        "\nmaxDescriptorSetStorageBuffersDynamic: " + std::to_string(properties.limits.maxDescriptorSetStorageBuffersDynamic) +
        "\nmaxDescriptorSetSampledImages: " + std::to_string(properties.limits.maxDescriptorSetSampledImages) +
        "\nmaxDescriptorSetStorageImages: " + std::to_string(properties.limits.maxDescriptorSetStorageImages) +
        "\nmaxDescriptorSetInputAttachments: " + std::to_string(properties.limits.maxDescriptorSetInputAttachments) +
        "\nmaxVertexInputAttributes: " + std::to_string(properties.limits.maxVertexInputAttributes) +
        "\nmaxVertexInputBindings: " + std::to_string(properties.limits.maxVertexInputBindings) +
        "\nmaxVertexInputAttributeOffset: " + std::to_string(properties.limits.maxVertexInputAttributeOffset) +
        "\nmaxVertexInputBindingStride: " + std::to_string(properties.limits.maxVertexInputBindingStride) +
        "\nmaxVertexOutputComponents: " + std::to_string(properties.limits.maxVertexOutputComponents) +
        "\nmaxTessellationGenerationLevel: " + std::to_string(properties.limits.maxTessellationGenerationLevel) +
        "\nmaxTessellationPatchSize: " + std::to_string(properties.limits.maxTessellationPatchSize) +
        "\nmaxTessellationControlPerVertexInputComponents: " + std::to_string(properties.limits.maxTessellationControlPerVertexInputComponents) +
        "\nmaxTessellationControlPerVertexOutputComponents: " + std::to_string(properties.limits.maxTessellationControlPerVertexOutputComponents) +
        "\nmaxTessellationControlPerPatchOutputComponents: " + std::to_string(properties.limits.maxTessellationControlPerPatchOutputComponents) +
        "\nmaxTessellationControlTotalOutputComponents: " + std::to_string(properties.limits.maxTessellationControlTotalOutputComponents) +
        "\nmaxTessellationEvaluationInputComponents: " + std::to_string(properties.limits.maxTessellationEvaluationInputComponents) +
        "\nmaxTessellationEvaluationOutputComponents: " + std::to_string(properties.limits.maxTessellationEvaluationOutputComponents) +
        "\nmaxGeometryShaderInvocations: " + std::to_string(properties.limits.maxGeometryShaderInvocations) +
        "\nmaxGeometryInputComponents: " + std::to_string(properties.limits.maxGeometryInputComponents) +
        "\nmaxGeometryOutputComponents: " + std::to_string(properties.limits.maxGeometryOutputComponents) +
        "\nmaxGeometryOutputVertices: " + std::to_string(properties.limits.maxGeometryOutputVertices) +
        "\nmaxGeometryTotalOutputComponents: " + std::to_string(properties.limits.maxGeometryTotalOutputComponents) +
        "\nmaxFragmentInputComponents: " + std::to_string(properties.limits.maxFragmentInputComponents) +
        "\nmaxFragmentOutputAttachments: " + std::to_string(properties.limits.maxFragmentOutputAttachments) +
        "\nmaxFragmentDualSrcAttachments: " + std::to_string(properties.limits.maxFragmentDualSrcAttachments) +
        "\nmaxFragmentCombinedOutputResources: " + std::to_string(properties.limits.maxFragmentCombinedOutputResources) +
        "\nmaxComputeSharedMemorySize: " + std::to_string(properties.limits.maxComputeSharedMemorySize) +
        "\nmaxComputeWorkGroupCount[3]: " + std::to_string(properties.limits.maxComputeWorkGroupCount[3]) +
        "\nmaxComputeWorkGroupInvocations: " + std::to_string(properties.limits.maxComputeWorkGroupInvocations) +
        "\nmaxComputeWorkGroupSize[3]: " + std::to_string(properties.limits.maxComputeWorkGroupSize[3]) +
        "\nsubPixelPrecisionBits: " + std::to_string(properties.limits.subPixelPrecisionBits) +
        "\nsubTexelPrecisionBits: " + std::to_string(properties.limits.subTexelPrecisionBits) +
        "\nmipmapPrecisionBits: " + std::to_string(properties.limits.mipmapPrecisionBits) +
        "\nmaxDrawIndexedIndexValue: " + std::to_string(properties.limits.maxDrawIndexedIndexValue) +
        "\nmaxDrawIndirectCount: " + std::to_string(properties.limits.maxDrawIndirectCount) +
        "\nmaxSamplerLodBias: " + std::to_string(properties.limits.maxSamplerLodBias) +
        "\nmaxSamplerAnisotropy: " + std::to_string(properties.limits.maxSamplerAnisotropy) +
        "\nmaxViewports: " + std::to_string(properties.limits.maxViewports) +
        "\nmaxViewportDimensions[2]: " + std::to_string(properties.limits.maxViewportDimensions[2]) +
        "\nviewportBoundsRange[2]: " + std::to_string(properties.limits.viewportBoundsRange[2]) +
        "\nviewportSubPixelBits: " + std::to_string(properties.limits.viewportSubPixelBits) +
        "\nminMemoryMapAlignment: " + std::to_string(properties.limits.minMemoryMapAlignment) +
        "\nminTexelBufferOffsetAlignment: " + std::to_string(properties.limits.minTexelBufferOffsetAlignment) +
        "\nminUniformBufferOffsetAlignment: " + std::to_string(properties.limits.minUniformBufferOffsetAlignment) +
        "\nminStorageBufferOffsetAlignment: " + std::to_string(properties.limits.minStorageBufferOffsetAlignment) +
        "\nminTexelOffset: " + std::to_string(properties.limits.minTexelOffset) +
        "\nmaxTexelOffset: " + std::to_string(properties.limits.maxTexelOffset) +
        "\nminTexelGatherOffset: " + std::to_string(properties.limits.minTexelGatherOffset) +
        "\nmaxTexelGatherOffset: " + std::to_string(properties.limits.maxTexelGatherOffset) +
        "\nminInterpolationOffset: " + std::to_string(properties.limits.minInterpolationOffset) +
        "\nmaxInterpolationOffset: " + std::to_string(properties.limits.maxInterpolationOffset) +
        "\nsubPixelInterpolationOffsetBits: " + std::to_string(properties.limits.subPixelInterpolationOffsetBits) +
        "\nmaxFramebufferWidth: " + std::to_string(properties.limits.maxFramebufferWidth) +
        "\nmaxFramebufferHeight: " + std::to_string(properties.limits.maxFramebufferHeight) +
        "\nmaxFramebufferLayers: " + std::to_string(properties.limits.maxFramebufferLayers) +
        "\nframebufferColorSampleCounts: " + std::to_string(properties.limits.framebufferColorSampleCounts) +
        "\nframebufferDepthSampleCounts: " + std::to_string(properties.limits.framebufferDepthSampleCounts) +
        "\nframebufferStencilSampleCounts: " + std::to_string(properties.limits.framebufferStencilSampleCounts) +
        "\nframebufferNoAttachmentsSampleCounts: " + std::to_string(properties.limits.framebufferNoAttachmentsSampleCounts) +
        "\nmaxColorAttachments: " + std::to_string(properties.limits.maxColorAttachments) +
        "\nsampledImageColorSampleCounts: " + std::to_string(properties.limits.sampledImageColorSampleCounts) +
        "\nsampledImageIntegerSampleCounts: " + std::to_string(properties.limits.sampledImageIntegerSampleCounts) +
        "\nsampledImageDepthSampleCounts: " + std::to_string(properties.limits.sampledImageDepthSampleCounts) +
        "\nsampledImageStencilSampleCounts: " + std::to_string(properties.limits.sampledImageStencilSampleCounts) +
        "\nstorageImageSampleCounts: " + std::to_string(properties.limits.storageImageSampleCounts) +
        "\nmaxSampleMaskWords: " + std::to_string(properties.limits.maxSampleMaskWords) +
        "\ntimestampComputeAndGraphics: " + std::to_string(properties.limits.timestampComputeAndGraphics) +
        "\ntimestampPeriod: " + std::to_string(properties.limits.timestampPeriod) +
        "\nmaxClipDistances: " + std::to_string(properties.limits.maxClipDistances) +
        "\nmaxCullDistances: " + std::to_string(properties.limits.maxCullDistances) +
        "\nmaxCombinedClipAndCullDistances: " + std::to_string(properties.limits.maxCombinedClipAndCullDistances) +
        "\ndiscreteQueuePriorities: " + std::to_string(properties.limits.discreteQueuePriorities) +
        "\npointSizeRange[2]: " + std::to_string(properties.limits.pointSizeRange[2]) +
        "\nlineWidthRange[2]: " + std::to_string(properties.limits.lineWidthRange[2]) +
        "\npointSizeGranularity: " + std::to_string(properties.limits.pointSizeGranularity) +
        "\nlineWidthGranularity: " + std::to_string(properties.limits.lineWidthGranularity) +
        "\nstrictLines: " + std::to_string(properties.limits.strictLines) +
        "\nstandardSampleLocations: " + std::to_string(properties.limits.standardSampleLocations) +
        "\noptimalBufferCopyOffsetAlignment: " + std::to_string(properties.limits.optimalBufferCopyOffsetAlignment) +
        "\noptimalBufferCopyRowPitchAlignment: " + std::to_string(properties.limits.optimalBufferCopyRowPitchAlignment) +
        "\nnonCoherentAtomSize: " + std::to_string(properties.limits.nonCoherentAtomSize);
    }


    VKAPI_ATTR VkBool32 VKAPI_CALL Window::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
    
        if(messageSeverity==VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            //LOG(pCallbackData->pMessage);
        } else if(messageSeverity==VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            INFO(pCallbackData->pMessage);
        } else if(messageSeverity==VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            WARNING(pCallbackData->pMessage);
        } else if(messageSeverity==VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            WARNING(pCallbackData->pMessage);
        }
    
        return VK_FALSE;
    }

    void Window::FramebufferResize(GLFWwindow* window, int width, int height) {
        auto windowPtr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        windowPtr->_framebufferResized = true;
    }

}
}