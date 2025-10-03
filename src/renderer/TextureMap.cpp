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
    }

    void TextureMap::StartLoading() {
        ASSERT(_amountTextures == 0, "[Renderer::TextureMap] Cannot start loading a second time on a TextureMap object");
    }
    uint32_t TextureMap::AddTextureLoader(std::unique_ptr<AssetLoader> assetLoader) {
        _amountTextures += assetLoader->GetAmountTextures();
        _assetLoaders.push_back(std::move(assetLoader));
        return (uint32_t)(_assetLoaders.size() - 1);
    }
    void TextureMap::SetCacheName(const std::string name) {
        _cacheName = name;
    }
    void TextureMap::EndLoading(Vulkan::Context& context, std::initializer_list<Vulkan::Pipeline*> bindToPipelines, const Util::FileManager& fileManager) {
        if(_amountTextures == 0) return;
        // Check if the cache is usable
        
        // Retrieve needed texture sizes
        RectanglePacker packer;
        packer.SetMaximumBinSize(Util::Vec2U32(1920, 1080));
        packer.SetAmountRectangles(_amountTextures);
        Util::Vec3U32* inputPtr = packer.GetRectangleInputPtr();
        size_t currentTexture = 0;
        for(const auto& assetLoader : _assetLoaders) {
            // Set all the variables
            assetLoader->_fileManager = &fileManager;
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
        packer.Pack();

        // Create and render the textures
        _textures.resize(packer.GetAmountBins());
        _renderInfos.resize(_amountTextures);
        Util::Vec2U32* binSizePtr = packer.GetBinSizes();
        Vulkan::CommandBuffer commandBuffer;
        Vulkan::QueueType queueType = context.GetQueue(Vulkan::QueueType::TransferQueue)==VK_NULL_HANDLE? Vulkan::QueueType::GraphicsQueue : Vulkan::QueueType::TransferQueue;
        commandBuffer.Init(context, queueType, 1);
        for(size_t i = 0; i < _textures.size(); i++) {
            _textures[i].Init(context, *binSizePtr, VK_FORMAT_R8G8B8A8_SRGB);
            uint32_t descriptorBinding = (*bindToPipelines.begin())->BindTextureDescriptor(context, _textures[i]);
            // It is ugly but it works
            for(Vulkan::Pipeline* const* p = bindToPipelines.begin()+1; p<bindToPipelines.end(); p++) {
                if(descriptorBinding!=(*p)->BindTextureDescriptor(context, _textures[i])) 
                    THROW("[Renderer::TextureMap] EndLoading should receive pipelines with equal amount of textures and with exclusive acces to the bindings (nothing else should bind textures)")
            }

            _textures[i].StartTransferingData(context);
            // Go through all the textures and render the once with this bin
            // Going through them one-by-one to not overload the amount of transfer memory needed
            std::pair<uint32_t, Util::AreaU32>* resultPtr = packer.GetResults();
            size_t currentAssetLoader = 0;
            Util::AreaU8* texturePtr = reinterpret_cast<Util::AreaU8*>(_textures[i].GetTransferLocation());
            for(size_t j = 0; j < _amountTextures; j++) {
                if(resultPtr->first != i) continue;
                _assetLoaders[currentAssetLoader]->RenderTexture(texturePtr, *binSizePtr, resultPtr->second, j-_assetLoaders[currentAssetLoader]->_firstTexture);
                _assetLoaders[currentAssetLoader]->SetTextureRenderInfo(
                    Util::AreaF((float)resultPtr->second.x/binSizePtr->x, (float)resultPtr->second.y/binSizePtr->y, (float)resultPtr->second.w/binSizePtr->x, (float)resultPtr->second.h/binSizePtr->y), 
                    descriptorBinding, 
                    j-_assetLoaders[currentAssetLoader]->_firstTexture
                );

                if(_assetLoaders[currentAssetLoader]->_lastTexture == j) currentAssetLoader++;
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
        _assetLoaders.clear();
    }
    
    std::shared_ptr<uint8_t> TextureMap::GetRenderInfo(const uint32_t id) {
        return _renderInfos[id];
    }



}
}