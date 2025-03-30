#ifndef ENGINE_CORE_SCENE_H
#define ENGINE_CORE_SCENE_H

#include "core/PCH.h"
#include "core/Components.h"

#include "renderer/Window.h"
#include "renderer/ImageLoader.h"

namespace Engine{

    typedef entt::entity Entity;

    class Game;
    class Scene {
    public:

        Scene(Game* game, Renderer::Window* window) : _game(game), _window(window) {}

        virtual void OnSceneStart() {}
        virtual void OnSceneStop() {}
        virtual void LoadAssets() {}

        // Should only be called inside the LoadAssets function
        // Set a unique name to be used all the times the exact same assets are loaded
        void SetAssetCacheName(const std::string name);
        // Should only be called inside the LoadAssets function
        Renderer::AssetID LoadTextureFile(const std::string file);
        // Should only be called inside the LoadAssets function
        Renderer::AssetID LoadQrCode(const std::string file);
        // Should only be called inside the LoadAssets function
        Renderer::AssetID LoadTextFile(const std::string file);

        inline entt::entity CreateEntity() { return _entt.create(); }
		template<class ComponentType, class ... Ts>
		inline void AddComponent(entt::entity entity, Ts && ... inputs) {
            if(IsDrawableObject<ComponentType>()) _drawableObjects++;
			_entt.emplace<ComponentType>(entity, inputs...);
		}
		template<class ComponentType>
		inline ComponentType& GetComponent(entt::entity entity) {
			return _entt.get<ComponentType>(entity);
		}

    private:
        friend struct TextureComponent;
        friend class Game;

        template <typename T>
		constexpr bool IsDrawableObject() { return std::is_same<T, TextureComponent>::value; }

        entt::registry _entt;
        Game* _game;
        Renderer::Window* _window;
        uint32_t _drawableObjects = 0;
    };

}

#endif