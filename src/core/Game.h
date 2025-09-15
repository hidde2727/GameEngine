#ifndef ENGINE_GAME_H
#define ENGINE_GAME_H

#include "core/PCH.h"
#include "core/Scene.h"

#include "renderer/Window.h"
#include "renderer/ImageLoader.h"
#include "renderer/TextLoader.h"

#include "network/WebHandler.h"

#include "physics/Engine.h"

#define ENGINE_GAME_TEXTUREMAP_ID 0
#define ENGINE_SCENE_TEXTUREMAP_ID 1

namespace Engine {
    typedef Renderer::AssetID AssetID;
    typedef Network::HTTP::RequestHeader HTTPRequestHeader;
    typedef Network::HTTP::Response HTTPResponse;
    typedef Network::Websocket::Frame WebsocketFrame;
    typedef Network::WebHandler::WebsocketConnection WebsocketConnection;

    class Game {
    public:

        Game();

        virtual std::string GetEngineResourceDirectory() { return "resources/engine/"; }
        virtual void OnStart() {}
        virtual void OnSceneStart() {}
        virtual void LoadAssets() {}
        virtual void OnHTTPRequest(HTTPRequestHeader& requestHeader, std::vector<uint8_t>& requestBody, HTTPResponse& response) {}
        virtual bool AllowWebsocketConnection(HTTPRequestHeader& requestHeader) { return true; }
        virtual void OnWebsocketRequest(WebsocketFrame& frame, WebsocketConnection& connection) {}
        virtual void OnWebsocketStart(WebsocketConnection& connection, const size_t uuid, HTTPRequestHeader& requestHeader) {}
        virtual void OnWebsocketStop(WebsocketConnection& connection, const size_t uuid) {}

        int Run();

        template<class tScene>
        void StartScene() {
            StopScene();
            // Start a new scene by first creating one
            _scene = new tScene(this, &_window);
            _window.StartAssetLoading(ENGINE_SCENE_TEXTUREMAP_ID);
            _scene->LoadAssets();
            _window.EndAssetLoading(ENGINE_SCENE_TEXTUREMAP_ID);
            _scene->OnSceneStart();
            OnSceneStart();
        }
        void StopScene();

        // Should only be called inside the LoadAssets function
        // Set a unique name to be used all the times the exact same assets are loaded
        void SetAssetCacheName(const std::string name);
        // Should only be called inside the LoadAssets function
        AssetID LoadTextureFile(const std::string file);
        // Should only be called inside the LoadAssets function
        AssetID LoadQrCode(const std::string file);
        // Should only be called inside the LoadAssets function
        AssetID LoadTextFile(const std::string file, const Renderer::Characters characters, const std::initializer_list<uint32_t> sizes);

        void DebugLine(const Util::Vec2F start, const Util::Vec2F end, const Util::Vec3F color);

    private:
        Scene* _scene;
        Renderer::Window _window;
        Network::WebHandler _webhandler;
        Physics::Engine _physics;
    };

}

#endif