#include "core/Components.h"

#include "core/Scene.h"

namespace Engine {

    TextureComponent::TextureComponent(Scene* scene, Renderer::AssetID assetID) {
        std::pair<Utils::AreaF, uint32_t>* info = scene->_window->GetTextureInfo(assetID);
        _textureArea = info->first;
        _descriptorID = info->second;
    }

}