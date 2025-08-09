#include "core/Scene.h"
#include "core/Game.h"

namespace Engine{

    void Scene::SetAssetCacheName(const std::string name) {
        _window->SetAssetLoadingCacheName(ENGINE_SCENE_TEXTUREMAP_ID, name);
    }
    Renderer::AssetID Scene::LoadTextureFile(const std::string file) {
        return _window->AddAsset(ENGINE_SCENE_TEXTUREMAP_ID, std::make_unique<Renderer::ImageLoader>(file), ENGINE_RENDERER_ASSETTYPE_TEXTURE);
    }
    Renderer::AssetID Scene::LoadQrCode(const std::string file) {
        return 0;
    }
    Renderer::AssetID Scene::LoadTextFile(const std::string file, const Renderer::Characters characters, const std::initializer_list<uint32_t> sizes) {
        return _window->AddAsset(ENGINE_SCENE_TEXTUREMAP_ID, std::make_unique<Renderer::TextLoader>(file, characters, sizes), ENGINE_RENDERER_ASSETTYPE_TEXT);
    }

    void Scene::DebugLine(const Util::Vec2F start, const Util::Vec2F end, const Util::Vec3F color) {
        _window->AddDebugLine(start, end, color);
    }

}