#include "Game.h"

#include "util/DebugGraphics.h"

namespace Engine {

    Game::Game() { }

    int Game::Run() {
        try {
            Util::FileManager::Init(GetResourceDirectories(), GetCacheDirectory());
            Util::SetDebugGraphicsTargetIfNull(this);
            
            _webhandler = Network::WebHandler::Create();
            _webhandler->Route("/", this);// Router all requests to this
            _webhandler->Start();
            _window.Init(2);

            _window.StartAssetLoading(ENGINE_GAME_TEXTUREMAP_ID);
            LoadAssets();
            _window.EndAssetLoading(ENGINE_GAME_TEXTUREMAP_ID);
            OnStart();

            _previousFrame = std::chrono::steady_clock::now();
            while(!_window.ShouldClose()) {
                ASSERT(_scene!=nullptr, "[Game] No scene bound, there should always be a scene bound")
                _webhandler->Update();
                _window.Update();

                auto now = std::chrono::steady_clock::now();
                float dt = (float)(((double)std::chrono::nanoseconds(now - _previousFrame).count()) / 1000000000);
                // TODO: Use the framerate of the device to skip frames
                if(dt > (1/30.f)) {
                    dt = 1/30.f;
                    INFO("[Game] Skipping frames, previous took too long")
                }
                _previousFrame = now;
                
                _scene->OnFrame(dt);
                _scene->_physics.Update(_scene->_entt, dt);
                _window.Draw(_scene->_entt, _scene->_textureComponents, _scene->_textComponents);
            }
            
            StopScene();
            _webhandler->Stop();
            _webhandler = nullptr;
            _window.Cleanup();
        } catch(std::exception exc) {
#ifndef __DEBUG__
            WARNING(exc.what())
            try {
                INFO(_window.GetVulkanDeviceLimits())
            } catch(std::exception exc) {}
#endif
            WARNING(exc.what())
            StopScene();
            _webhandler->Stop();
            _webhandler = nullptr;
            _window.Cleanup();

            LOG("Press any key to continue . . .")
            std::cin.get();
        }

        return 0;
    }

    void Game::StopScene() {
        if (!_scene) return;// There is no scene bound
        _scene->OnSceneStop();
        Route(nullptr);
        _scene = nullptr;
        _window.CleanupAssets(ENGINE_SCENE_TEXTUREMAP_ID);
    }

    
    void Game::SetAssetCacheName(const std::string name) {
        _window.SetAssetLoadingCacheName(ENGINE_GAME_TEXTUREMAP_ID, name);
    }
    Renderer::AssetID Game::LoadTextureFile(const std::string file) {
        return _window.AddAsset(
            ENGINE_GAME_TEXTUREMAP_ID, 
            std::static_pointer_cast<Renderer::AssetLoader>(std::make_shared<Renderer::ImageLoader>(file)), 
            ENGINE_RENDERER_ASSETTYPE_TEXTURE
        );
    }
    Renderer::AssetID Game::LoadQrCode(const std::string file) {
        return 0;
    }
    Renderer::AssetID Game::LoadTextFile(const std::string file, const Renderer::Characters characters, const std::initializer_list<uint32_t> sizes) {
        return _window.AddAsset(
            ENGINE_GAME_TEXTUREMAP_ID, 
            std::static_pointer_cast<Renderer::AssetLoader>(std::make_shared<Renderer::TextLoader>(file, characters, sizes)), 
            ENGINE_RENDERER_ASSETTYPE_TEXT
        );
    }

    void Game::SetCameraPosition(const Util::Vec2F pos) {
        _window.SetCameraPosition(pos);
    }
    
    void Game::DebugLine(const Util::Vec2F start, const Util::Vec2F end, const Util::Vec3F color) {
        _window.AddDebugLine(start, end, color);
    }

}