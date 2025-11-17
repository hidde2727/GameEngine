#include "renderer/TextureMap.h"

namespace Engine {
namespace Renderer {

    void TextureMap::Init() {

    }
    void TextureMap::Cleanup(Vulkan::Context& context, std::initializer_list<Vulkan::Pipeline*> boundToPipelines) {
        for(Vulkan::Texture& texture : _textures) {
            uint32_t slot = texture.GetBoundDescriptorSlot();
            for(Vulkan::Pipeline* pipeline : boundToPipelines) {
                pipeline->UnbindTextureDescriptor(context, slot, texture);
            }
            texture.Cleanup(context);
        }
        _textures.clear();
        _assetLoaders.clear();
        _renderInfos.clear();
        _amountTextures = 0;
        _cacheName.clear();
    }

    void TextureMap::StartLoading() {
        ASSERT(_amountTextures == 0, "[Renderer::TextureMap] Cannot start loading a second time on a TextureMap object");
    }
    uint32_t TextureMap::AddTextureLoader(std::shared_ptr<AssetLoader> assetLoader) {
        _amountTextures += assetLoader->GetAmountTextures();
        _assetLoaders.push_back(assetLoader);
        return (uint32_t)(_assetLoaders.size() - 1);
    }
    void TextureMap::SetCacheName(const std::string name) {
        _cacheName = name;
    }
    void TextureMap::EndLoading(Vulkan::Context& context, std::initializer_list<Vulkan::Pipeline*> bindToPipelines) {
        if(_amountTextures == 0) return;        
        // Retrieve needed texture sizes
        RectanglePacker packer;
        packer.SetMaximumBinSize(ENGINE_RENDERER_MAX_IMAGE_SIZE);
        packer.SetAmountRectangles(_amountTextures);
        Util::Vec3U32* inputPtr = packer.GetRectangleInputPtr();// Get the location where the input should go
        size_t currentTexture = 0;
        for(const auto& assetLoader : _assetLoaders) {
            // Set all the variables
            assetLoader->_firstTexture = currentTexture;
            currentTexture += assetLoader->GetAmountTextures();
            assetLoader->_lastTexture = currentTexture-1;
            assetLoader->Init();

            // Retrieve the texture sizes needed for the loader
            assetLoader->SetTextureSizes(inputPtr);
            inputPtr += assetLoader->GetAmountTextures();
        }

        // Pack
        packer.SetPackingAlgorithm(RectanglePacker::PackingAlgorithm::Shelf);
        packer.SetSortingAlgorithm(RectanglePacker::SortingAlgorithm::BigHeightFirst);
        packer.Pack();

        // Create and render the textures
        _textures.resize(packer.GetAmountBins());
        _renderInfos.resize(_amountTextures);
        Util::Vec2U32* binSizePtr = packer.GetBinSizes();
        Vulkan::CommandBuffer commandBuffer;
        Vulkan::QueueType queueType = context.GetQueue(Vulkan::QueueType::TransferQueue)==VK_NULL_HANDLE? Vulkan::QueueType::GraphicsQueue : Vulkan::QueueType::TransferQueue;
        commandBuffer.Init(context, queueType, 1);
        for(size_t i = 0; i < _textures.size(); i++) {// i = current packed bin/current texture
            _textures[i].Init(context, *binSizePtr, VK_FORMAT_R8G8B8A8_SRGB);
            // Make sure all the pipelines when binding this descriptor return the same DescriptorBindingID
            uint32_t descriptorBinding = (*bindToPipelines.begin())->BindTextureDescriptor(context, _textures[i]);
            for(Vulkan::Pipeline* const* p = bindToPipelines.begin()+1; p<bindToPipelines.end(); p++) {
                if(descriptorBinding!=(*p)->BindTextureDescriptor(context, _textures[i])) 
                    THROW("[Renderer::TextureMap] EndLoading should receive pipelines with equal amount of textures and with exclusive acces to the bindings (nothing else should bind textures)")
            }

            _textures[i].StartTransferingData(context);
            // Go through all the textures and render the once with this bin
            // Going through them one-by-one to not overload the amount of transfer memory needed
            RectanglePacker::ResultArea* resultPtr = packer.GetResults();
            Util::AreaU8* texturePtr = reinterpret_cast<Util::AreaU8*>(_textures[i].GetTransferLocation());
            for(size_t j = 0; j < _amountTextures; j++) {
                // Check if this rectangle should be rendered on texture with ID=i
                if(resultPtr->_bin != i) { resultPtr++; continue; }
                // Get the asset loader the area belongs to
                size_t assetLoader;
                for(assetLoader = 0; assetLoader < _assetLoaders.size(); assetLoader++) {
                    if(_assetLoaders[assetLoader]->_firstTexture <= resultPtr->_origID &&
                    _assetLoaders[assetLoader]->_lastTexture >= resultPtr->_origID) break;
                }
                ASSERT(assetLoader < _assetLoaders.size(), "[TextureMap::EndLoading] Failed to find the asset loader associated with a packed texture")
                // Render the texture with the asset loader
                _assetLoaders[assetLoader]->RenderTexture(texturePtr, *binSizePtr, resultPtr->_area, j-_assetLoaders[assetLoader]->_firstTexture);
                _assetLoaders[assetLoader]->SetTextureRenderInfo(
                    Util::AreaF((float)resultPtr->_area.x/binSizePtr->x, (float)resultPtr->_area.y/binSizePtr->y, (float)resultPtr->_area.w/binSizePtr->x, (float)resultPtr->_area.h/binSizePtr->y), 
                    descriptorBinding, 
                    j-_assetLoaders[assetLoader]->_firstTexture
                );

                resultPtr++;
            }
            binSizePtr++;
            if(i!=0) {
                context.WaitQueueIdle(queueType);
                _textures[i-1].TransferCompleteOnCommandBuffer(context);
            }
            commandBuffer.StartRecording(context, UINT32_MAX, true);
            _textures[i].EndTransferingData(context, commandBuffer);
            commandBuffer.EndRecording();
            commandBuffer.Submit(context);
            
        }
        context.WaitQueueIdle(queueType);
        _textures[_textures.size() - 1].TransferCompleteOnCommandBuffer(context);

        // Render infos
        for(size_t i = 0; i < _assetLoaders.size(); i++) {
            _renderInfos[i] = _assetLoaders[i]->GetRenderInfo();
        }

        // Remove the loaders
        _assetLoaders.clear();
    }
    
    std::shared_ptr<uint8_t> TextureMap::GetRenderInfo(const uint32_t id) {
        return _renderInfos[id];
    }



}
}