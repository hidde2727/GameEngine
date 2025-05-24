#ifndef ENGINE_CORE_SCENE_H
#define ENGINE_CORE_SCENE_H

#include "core/PCH.h"
#include "core/Components.h"

#include "renderer/Window.h"
#include "renderer/ImageLoader.h"
#include "renderer/TextLoader.h"

#include "network/WebHandler.h"

namespace Engine{

    typedef entt::entity Entity;

    class Game;
    class Scene {
    public:

        Scene(Game* game, Renderer::Window* window) : _game(game), _window(window) {}

        virtual void OnSceneStart() {}
        virtual void OnSceneStop() {}
        virtual void LoadAssets() {}
        virtual void OnHTTPRequest(Network::HTTP::RequestHeader& requestHeader, std::vector<uint8_t>& requestBody, Network::HTTP::Response& response) {}
        virtual bool AllowWebsocketConnection(Network::HTTP::RequestHeader& requestHeader) { return true; }
        virtual void OnWebsocketRequest(Network::Websocket::Frame& frame, Network::WebHandler::WebsocketConnection& connection) {}
        virtual void OnWebsocketStart(Network::WebHandler::WebsocketConnection& connection, const size_t uuid, Network::HTTP::RequestHeader& requestHeader) {}
        virtual void OnWebsocketStop(Network::WebHandler::WebsocketConnection& connection, const size_t uuid) {}

        // Should only be called inside the LoadAssets function
        // Set a unique name to be used all the times the exact same assets are loaded
        void SetAssetCacheName(const std::string name);
        // Should only be called inside the LoadAssets function
        Renderer::AssetID LoadTextureFile(const std::string file);
        // Should only be called inside the LoadAssets function
        Renderer::AssetID LoadQrCode(const std::string file);
        // Should only be called inside the LoadAssets function
        Renderer::AssetID LoadTextFile(const std::string file, const Renderer::Characters characters, const std::initializer_list<uint32_t> sizes);

        inline entt::entity CreateEntity() { return _entt.create(); }
		template<class ComponentType, class ... Ts>
		inline void AddComponent(entt::entity entity, Ts && ... inputs) {
            if(IsTextureComponent<ComponentType>()) _textureComponents++;
			_entt.emplace<ComponentType>(entity, inputs...);
            
            if(IsTextComponent<ComponentType>()) {
                _textComponents += (uint32_t)_entt.get<TextComponent>(entity)._renderInfo.size();
            }
		}
		template<class ComponentType>
		inline ComponentType& GetComponent(entt::entity entity) {
			return _entt.get<ComponentType>(entity);
		}

    private:
        friend struct TextureComponent;
        friend struct TextComponent;
        friend class Game;

        template <typename T>
		constexpr bool IsTextureComponent() { return std::is_same<T, TextureComponent>::value; }
        template <typename T>
		constexpr bool IsTextComponent() { return std::is_same<T, TextComponent>::value; }

        entt::registry _entt;
        Game* _game;
        Renderer::Window* _window;
        uint32_t _textureComponents = 0;
        uint32_t _textComponents = 0;
    };

}

#endif