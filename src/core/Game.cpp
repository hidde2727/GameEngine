#include "Game.h"

namespace Engine {

    Game::Game() { }

    int Game::Run() {
        try {
            _webhandler.Start(
                // HTTP Handler
                [this] (Network::HTTP::RequestHeader& requestHeader, std::vector<uint8_t>& requestBody, Network::HTTP::Response& response) {
                    this->OnHTTPRequest(requestHeader, requestBody, response);
                    this->_scene->OnHTTPRequest(requestHeader, requestBody, response);
                },
                // Upgrade handler
                [this] (Network::HTTP::RequestHeader& requestHeader) -> bool {
                    if(!this->AllowWebsocketConnection(requestHeader)) return false;
                    if(!this->_scene->AllowWebsocketConnection(requestHeader)) return false;
                    return true;
                },
                [this] (Network::Websocket::Frame& frame, Network::WebHandler::WebsocketConnection& connection) {
                    this->OnWebsocketRequest(frame, connection);
                    this->_scene->OnWebsocketRequest(frame, connection);
                },
                [this] (Network::WebHandler::WebsocketConnection& connection, const size_t uuid, Network::HTTP::RequestHeader& requestHeader) {
                    this->OnWebsocketStart(connection, uuid, requestHeader);
                    this->_scene->OnWebsocketStart(connection, uuid, requestHeader);
                },
                [this] (Network::WebHandler::WebsocketConnection& connection, const size_t uuid) {
                    this->OnWebsocketStop(connection, uuid);
                    this->_scene->OnWebsocketStop(connection, uuid);
                }
            );
            _window.Init(2);

            _window.StartAssetLoading(ENGINE_GAME_TEXTUREMAP_ID);
            LoadAssets();
            _window.EndAssetLoading(ENGINE_GAME_TEXTUREMAP_ID);
            OnStart();


            while(!_window.ShouldClose()) {
                ASSERT(_scene==nullptr, "No scene bound, there should always be a scene bound")
                _webhandler.HandleRequests();
                _window.Update();
                _window.Draw(_scene->_entt, _scene->_textureComponents, _scene->_textComponents);
            }
            
            _webhandler.Stop();
            _window.Cleanup();
        } catch(std::exception exc) {
            try {
                INFO(_window.GetVulkanDeviceLimits())
            } catch(std::exception exc) {}
            _window.Cleanup();

            WARNING(exc.what())
            LOG("Press any key to continue . . .")
            std::cin.get();

            throw(std::runtime_error(exc.what()));
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