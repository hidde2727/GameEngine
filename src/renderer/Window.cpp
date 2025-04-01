#include "renderer/window.h"
    #include <bitset>

namespace Engine {
namespace Renderer {
    
    Window::Window(const uint32_t textureMapSlots) {
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

        _vkRenderPass.Init(_vkContext, _vkSwapchain, 2);

        _vkSwapchain.Resize(_vkContext, _vkRenderPass, width, height);

        // Rect pipeline
        Vulkan::PipelineCreator rectPipelineInfo;
        rectPipelineInfo.SetShaders({ "resources/engine/shaders/rect.vert", "resources/engine/shaders/rect.frag" });
        rectPipelineInfo.SetVertexInput({Vulkan::Vertex::Vec2 }, { Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec3, Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec2, Vulkan::Vertex::UInt });
        rectPipelineInfo.SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        rectPipelineInfo.SetDescriptorInfo(2, 16, 2, 0);
        rectPipelineInfo.SetPushConstantInput({ Vulkan::Vertex::Vec2 }, VK_SHADER_STAGE_VERTEX_BIT);
        _vkRectPipeline.Init(rectPipelineInfo, _vkContext, _vkRenderPass, 0);

        // Text pipeline
        Vulkan::PipelineCreator textPipelineInfo;
        textPipelineInfo.SetShaders({ "resources/engine/shaders/text.vert", "resources/engine/shaders/text.frag" });
        textPipelineInfo.SetVertexInput({Vulkan::Vertex::Vec2 }, { Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec3, Vulkan::Vertex::Vec2, Vulkan::Vertex::Vec2, Vulkan::Vertex::UInt, Vulkan::Vertex::Float });
        textPipelineInfo.SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        textPipelineInfo.SetDescriptorInfo(2, 16, 2, 0);
        textPipelineInfo.SetPushConstantInput({ Vulkan::Vertex::Vec2 }, VK_SHADER_STAGE_VERTEX_BIT);
        _vkTextPipeline.Init(textPipelineInfo, _vkContext, _vkRenderPass, 1);

        _vkCommandBuffer.Init(_vkContext, Vulkan::QueueType::GraphicsQueue, 2);
        _vkInFlightFence = _vkCommandBuffer.CreateFence(_vkContext, true);
        _vkImageAvailableSemaphore = _vkCommandBuffer.CreateSemaphore(_vkContext);
        _vkRenderFinishedSemaphore = _vkCommandBuffer.CreateSemaphore(_vkContext);

        Vertex rectangleData[] = { Vertex({0,0}), Vertex({1,0}), Vertex({1,1}), Vertex({0,1}) };
        _vkPerVertexBuffer.Init(_vkContext, sizeof(Vertex) * 4);
        _vkPerVertexBuffer.SetData(_vkContext, rectangleData);

        _vkRectVertexBuffer.Init(_vkContext, sizeof(InstanceDataRect) * 4);
        _vkTextVertexBuffer.Init(_vkContext, sizeof(InstanceDataText) * 4);

        Vulkan::TransferBuffer indexTransfer;
        indexTransfer.Init(_vkContext, 6*sizeof(uint16_t));
        _vkIndexBuffer.Init(_vkContext, 6*sizeof(uint16_t), true);
        indexTransfer.SetData<uint16_t>(_vkContext, {0, 1, 2, 0, 2, 3});
        indexTransfer.CopyTo(_vkContext, &_vkIndexBuffer);
        indexTransfer.Cleanup(_vkContext);
        
        _textureMaps.resize(textureMapSlots);
        
        _pixelSampler.Init(_vkContext, VK_FILTER_NEAREST, VK_FILTER_NEAREST);
        _vkRectPipeline.BindSamplerDescriptor(_vkContext, _pixelSampler, 0);
        _vkTextPipeline.BindSamplerDescriptor(_vkContext, _pixelSampler, 0);
        _linearSampler.Init(_vkContext, VK_FILTER_LINEAR, VK_FILTER_LINEAR);
        _vkRectPipeline.BindSamplerDescriptor(_vkContext, _linearSampler, 1);
        _vkTextPipeline.BindSamplerDescriptor(_vkContext, _linearSampler, 1);
    }
    Window::~Window() {
        _vkContext.WaitIdle();

        _pixelSampler.Cleanup(_vkContext);
        _linearSampler.Cleanup(_vkContext);

        for(TextureMap& textureMap : _textureMaps) {
            textureMap.Cleanup(_vkContext, { &_vkRectPipeline, &_vkTextPipeline });
        }
        _vkIndexBuffer.Cleanup(_vkContext);
        _vkPerVertexBuffer.Cleanup(_vkContext);
        _vkRectVertexBuffer.Cleanup(_vkContext);
        _vkTextVertexBuffer.Cleanup(_vkContext);

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
            if(width==0 && height==0) return;
            _vkSwapchain.Resize(_vkContext, _vkRenderPass, width, height);
            _framebufferSize.x = (float)width;
            _framebufferSize.y = (float)height;
        }
        _vkRectVertexBuffer.Resize(_vkContext, amountRectangles*sizeof(InstanceDataRect));
        _vkTextVertexBuffer.Resize(_vkContext, amountText*sizeof(InstanceDataText));

        {// Rectangle data
			auto group = registry.group<TextureComponent>(entt::get<AreaComponent>);
            _vkRectVertexBuffer.StartTransferingData(_vkContext);
			for (const auto [entity, texture, area] : group.each()) {
				_vkRectVertexBuffer.AddData(InstanceDataRect(
                    Utils::Vec2F(area.x , area.y),
                    Utils::Vec2F(area.w , area.h),
                    Utils::Vec3F(1.f, 1.f, 1.f),
                    Utils::Vec2F(texture._textureArea.x, texture._textureArea.y),
                    Utils::Vec2F(texture._textureArea.w, texture._textureArea.h),
                    texture._descriptorID
                ));
			}
            _vkRectVertexBuffer.EndTransferingData(_vkContext, _vkCommandBuffer);
		}
        {// Text data
			auto group = registry.group<TextComponent>(entt::get<AreaComponent>);
            _vkTextVertexBuffer.StartTransferingData(_vkContext);
			for (const auto [entity, text, area] : group.each()) {
                float x = area.x;
                float y = area.y;
            for(const auto renderInfo : text._renderInfo) {
                x = area.x + renderInfo._position.x;
                y = area.y + renderInfo._position.y;
                _vkTextVertexBuffer.AddData(InstanceDataText(
                    Utils::Vec2F(x , y),
                    Utils::Vec2F(renderInfo._position.w , renderInfo._position.h),
                    Utils::Vec3F(1.f, 1.f, 1.f),
                    Utils::Vec2F(renderInfo._textureArea.x, renderInfo._textureArea.y), 
                    Utils::Vec2F(renderInfo._textureArea.w, renderInfo._textureArea.h),
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
        _vkCommandBuffer.BindVertexBuffer(_vkPerVertexBuffer, 0);
        _vkCommandBuffer.BindVertexBuffer(_vkRectVertexBuffer, 1);
        _vkCommandBuffer.BindIndexBuffer(_vkIndexBuffer);
        _vkCommandBuffer.DrawIndexed(6, amountRectangles);

        _vkCommandBuffer.NextSubPass();

        _vkCommandBuffer.BindGraphicsPipeline(_vkTextPipeline);
        _vkCommandBuffer.SetPushConstantData(_vkTextPipeline, _framebufferSize, VK_SHADER_STAGE_VERTEX_BIT);
        _vkCommandBuffer.BindDescriptorSet(_vkTextPipeline);
        _vkCommandBuffer.BindVertexBuffer(_vkPerVertexBuffer, 0);
        _vkCommandBuffer.BindVertexBuffer(_vkTextVertexBuffer, 1);
        _vkCommandBuffer.BindIndexBuffer(_vkIndexBuffer);
        _vkCommandBuffer.DrawIndexed(6, amountText);

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
        _textureMaps[textureMapID].EndLoading(_vkContext, { &_vkRectPipeline, &_vkTextPipeline });
    }
    void Window::CleanupAssets(const size_t textureMapID) {
        _textureMaps[textureMapID].Cleanup(_vkContext, { &_vkRectPipeline, &_vkTextPipeline });
    }

    std::shared_ptr<ImageRenderInfo> Window::GetTextureInfo(AssetID asset) {
        if(Utils::GetBits<uint32_t>(asset, ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS, ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS) != ENGINE_RENDERER_ASSETTYPE_TEXTURE) 
            THROW("Cannot acces an asset that is not a texture with the GetTextureInfo function")
        return reinterpret_pointer_cast<ImageRenderInfo>(
            _textureMaps[asset >> ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS].GetRenderInfo(asset & (0xFFFFFFFF >> ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS))
        );
    }
    std::shared_ptr<TextRenderInfo> Window::GetTextInfo(AssetID asset) {
        if(Utils::GetBits<uint32_t>(asset, ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS, ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS) != ENGINE_RENDERER_ASSETTYPE_TEXT) 
            THROW("Cannot acces an asset that is not text with the GetTextInfo function")
        return reinterpret_pointer_cast<TextRenderInfo>(
            _textureMaps[asset >> ENGINE_RENDERER_ASSETID_TEXTUREMAPID_SHIFT_BITS].GetRenderInfo(asset & (0xFFFFFFFF >> ENGINE_RENDERER_ASSETTYPE_TEXTUREMAPID_SHIFT_BITS))
        );
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
            ERROR(pCallbackData->pMessage);
        } else if(messageSeverity==VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            ERROR(pCallbackData->pMessage);
        }
    
        return VK_FALSE;
    }

    void Window::FramebufferResize(GLFWwindow* window, int width, int height) {
        auto windowPtr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        windowPtr->_framebufferResized = true;
    }

}
}