#include "Game.h"

namespace Engine {

    Game::Game() : _window(2) { }

    int Game::Run() {
        _window.StartAssetLoading(ENGINE_GAME_TEXTUREMAP_ID);
        LoadAssets();
        _window.EndAssetLoading(ENGINE_GAME_TEXTUREMAP_ID);
        OnStart();


        while(!_window.ShouldClose()) {
            ASSERT(_scene==nullptr, "No scene bound, there should always be a scene bound")
            _window.Update();
            _window.Draw(_scene->_entt, _scene->_textureComponents, _scene->_textComponents);
        }

        return 0;
    }

    void Game::StopScene() {
        if (_scene) {
            _scene->OnSceneStop();
            delete _scene;
            _scene = nullptr;
            _window.CleanupAssets(ENGINE_SCENE_TEXTUREMAP_ID);
        }

    }

    
    void Game::SetAssetCacheName(const std::string name) {
        _window.SetAssetLoadingCacheName(ENGINE_GAME_TEXTUREMAP_ID, name);
    }
    Renderer::AssetID Game::LoadTextureFile(const std::string file) {
        return _window.AddAsset(ENGINE_GAME_TEXTUREMAP_ID, std::make_unique<Renderer::ImageLoader>(file), ENGINE_RENDERER_ASSETTYPE_TEXTURE);
    }
    Renderer::AssetID Game::LoadQrCode(const std::string file) {
        return 0;
    }
    Renderer::AssetID Game::LoadTextFile(const std::string file, const Renderer::Characters characters, const std::initializer_list<uint32_t> sizes) {
        return _window.AddAsset(ENGINE_GAME_TEXTUREMAP_ID, std::make_unique<Renderer::TextLoader>(file, characters, sizes), ENGINE_RENDERER_ASSETTYPE_TEXT);
    }

}