#include "Game.h"

#include "util/DebugGraphics.h"
#include "util/serialization/Binary.h"

namespace Engine {

    Game::Game() { }

    void Game::Start() {
        Util::FileManager::Init(GetResourceDirectories(), GetCacheDirectory());
        Util::SetDebugGraphicsTargetIfNull(this);
        Util::BinarySerializer serializer;
        struct Test {
            std::string f = "hello world";
            struct Nested {
                double a = 1;
                std::map<int, std::string> m = {{0, "hello"}, {2, "world"}};
            }n;
        }t;
        std::vector<char> serialized;
        serializer.Serialize(t, serialized, Util::BinarySerializer::OutputFlag::IncludeTypeInfo);
        t.f = "";
        t.n.a = 0;
        t.n.m[0] = "world";
        t.n.m[1] = "hello";
        Util::BinaryDeserializer deserializer;
        deserializer.Deserialize(t, serialized);

        _webhandler = Network::WebHandler::Create();
        _webhandler->Route("/", this);// Router all requests to this
        _webhandler->Start();
        _window.Init(2);

        _window.StartAssetLoading(ENGINE_GAME_TEXTUREMAP_ID);
        LoadAssets();
        _window.EndAssetLoading(ENGINE_GAME_TEXTUREMAP_ID);
        OnStart();
    }
    int Game::Run() {
        try {
            Start();

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
        } catch(std::runtime_error exc) {
            OnError("[Game] Caught std::runtime_error '" + std::string(exc.what()) + "'");
        } catch(std::exception exc) {
            OnError("[Game] Caught std::exception '" + std::string(exc.what()) + "'");
        } catch(...) {
            OnError("[Game] Caught unknown exception without a message");
        }
        Cleanup();

        return 0;
    }

    void Game::OnError(const std::string& message) {
#ifndef __DEBUG__
        try {
            INFO(_window.GetVulkanDeviceLimits())
        } catch(std::exception exc) {}
#endif
        WARNING(message)

        LOG("Press any key to continue . . .")
        std::cin.get();
    }
    void Game::Cleanup() {
        StopScene();
        _webhandler->Stop();
        _webhandler = nullptr;
        _window.Cleanup();
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