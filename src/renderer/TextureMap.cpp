#include "renderer/TextureMap.h"

namespace Engine {
namespace Renderer {

    void TextureMap::Init() {

    }
    void TextureMap::Cleanup(Vulkan::Context& context, Vulkan::Pipeline& boundToPipeline) {
        for(Vulkan::Texture& texture : _textures) {
            boundToPipeline.UnbindTextureDescriptor(context, texture);
            texture.Cleanup(context);
        }
    }

    void TextureMap::StartLoading() {
        ASSERT(_amountTextures > 0, "Cannot start loading a second time on a TextureMap object");
    }
    uint32_t TextureMap::AddTextureLoader(std::unique_ptr<AssetLoader> assetLoader) {
        _amountTextures += assetLoader->GetAmountTextures();
        _assetLoaders.push_back(std::move(assetLoader));
        return (uint32_t)(_assetLoaders.size() - 1);
    }
    void TextureMap::SetCacheName(const std::string name) {
        _cacheName = name;
    }
    void TextureMap::EndLoading(Vulkan::Context& context, Vulkan::Pipeline& bindToPipeline) {
        if(_amountTextures == 0) return;
        // Check if the cache is usable
        
        // Retrieve needed texture sizes
        RectanglePacker packer;
        packer.SetAmountRectangles(_amountTextures);
        Utils::Vec3U32* inputPtr = packer.GetRectangleInputPtr();
        size_t currentTexture = 0;
        for(const auto& assetLoader : _assetLoaders) {
            assetLoader->_firstTexture = currentTexture;
            assetLoader->SetTextureSizes(inputPtr);
            inputPtr += assetLoader->GetAmountTextures();
            currentTexture += assetLoader->GetAmountTextures();
            assetLoader->_lastTexture = currentTexture-1;
        }

        // Pack
        packer.SetPackingAlgorithm(RectanglePacker::PackingAlgorithm::Shelf);
        packer.Pack();

        // Create and render the textures
        _textures.resize(packer.GetAmountBins());
        _renderInfos.resize(_amountTextures);
        Utils::Vec2U32* binSizePtr = packer.GetBinSizes();
        Vulkan::CommandBuffer commandBuffer;
        Vulkan::QueueType queueType = context.GetQueue(Vulkan::QueueType::TransferQueue)==VK_NULL_HANDLE? Vulkan::QueueType::GraphicsQueue : Vulkan::QueueType::TransferQueue;
        commandBuffer.Init(context, queueType, 1);
        for(size_t i = 0; i < _textures.size(); i++) {
            _textures[i].Init(context, *binSizePtr, VK_FORMAT_R8G8B8A8_SRGB);
            uint32_t descriptorBinding = bindToPipeline.BindTextureDescriptor(context, _textures[i]);
            _textures[i].StartTransferingData(context);
            // Go through all the textures and render the once with this bin
            // Going through them one-by-one to not overload the amount of transfer memory needed
            std::pair<uint32_t, Utils::AreaU32>* resultPtr = packer.GetResults();
            size_t currentAssetLoader = 0;
            Utils::AreaU8* texturePtr = reinterpret_cast<Utils::AreaU8*>(_textures[i].GetTransferLocation());
            for(size_t j = 0; j < _amountTextures; j++) {
                if(resultPtr->first != i) continue;
                _assetLoaders[j]->RenderTexture(texturePtr, resultPtr->second, j);
                _assetLoaders[j]->SetTextureRenderInfo(
                    Utils::AreaF((float)resultPtr->second.x/binSizePtr->x, (float)resultPtr->second.y/binSizePtr->y, (float)resultPtr->second.w/binSizePtr->x, (float)resultPtr->second.h/binSizePtr->y), 
                    descriptorBinding, 
                    j
                );

                if(_assetLoaders[j]->_lastTexture == j) currentAssetLoader++;
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

        // Create the cache

        // Remove the loaders
    }
    
    uint8_t* TextureMap::GetRenderInfo(const uint32_t id) {
        return _renderInfos[id].data();
    }



}
}