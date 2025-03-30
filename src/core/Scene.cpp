#include "core/Scene.h"
#include "core/Game.h"

namespace Engine{

    void Scene::SetAssetCacheName(const std::string name) {
        _window->SetAssetLoadingCacheName(ENGINE_SCENE_TEXTUREMAP_ID, name);
    }
    Renderer::AssetID Scene::LoadTextureFile(const std::string file) {
        return _window->AddAsset(ENGINE_SCENE_TEXTUREMAP_ID, std::make_unique<Renderer::ImageLoader>(file));
    }
    Renderer::AssetID Scene::LoadQrCode(const std::string file) {
        return 0;
    }
    Renderer::AssetID Scene::LoadTextFile(const std::string file) {
        return 0;
    }

}