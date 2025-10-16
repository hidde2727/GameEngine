#ifndef ENGINE_CORE_SCENE_H
#define ENGINE_CORE_SCENE_H

#include "core/PCH.h"
#include "core/Components.h"

#include "renderer/Window.h"
#include "renderer/ImageLoader.h"
#include "renderer/TextLoader.h"

#include "network/WebHandler.h"

#include "physics/Engine.h"
#include "physics/Components.h"

namespace Engine{

    typedef entt::entity Entity;

    class Game;
    class Scene {
    public:

        Scene(Game* game, Renderer::Window* window) : _game(game), _window(window) {}

        virtual void OnSceneStart() {}
        virtual void OnFrame() {}
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
        template<class ... Ts>
        inline entt::entity CreateEntity(Ts&&...components) {
            entt::entity entity = CreateEntity();
            AddComponents(entity, std::forward<Ts>(components)...);
            return entity;
        }
        // Has template overloaded for Component::Collider
		template<class ComponentType>
		inline void AddComponent(const entt::entity entity, const ComponentType component) {
#ifdef __DEBUG__
            ASSERT(!_entt.all_of<ComponentType>(entity), "[Scene] Cannot add a component to an entity that already has that component")
#endif
            if(IsTextureComponent<ComponentType>()) _textureComponents++;
			_entt.emplace<ComponentType>(entity, component);
            
            if(IsTextComponent<ComponentType>()) {
                _textComponents += (uint32_t)_entt.get<Component::Text>(entity)._renderInfo.size();
            }
		}
        template<class T>
        inline void AddComponents(const entt::entity entity, T&& c) {
            AddComponent<T>(entity, c);
        }
        template<class T, class ... Ts>
        inline void AddComponents(const entt::entity entity, T&& c, Ts&&...others) {
            AddComponent<T>(entity, c);
            AddComponents(entity, std::forward<Ts>(others)...);
        }
        // Has template overloaded for Component::Collider
		template<class ComponentType>
		inline void SetComponent(const entt::entity entity, const ComponentType component) {
#ifdef __DEBUG__
            ASSERT(HasComponent<ComponentType>(entity), "[Scene] Cannot set a component to an entity that doesnt't have that component")
#endif
			_entt.replace<ComponentType>(entity, component);
		}
        // Has template overloaded for Component::Collider
        template<class ComponentType>
        inline bool HasComponent(const entt::entity entity) {
            return _entt.all_of<ComponentType>(entity);
        }
        // Has template overloaded for Component::Collider
		template<class ComponentType>
		inline ComponentType& GetComponent(const entt::entity entity) {
			return _entt.get<ComponentType>(entity);
		}
        inline void DeleteEntity(const entt::entity entity) {
            if(HasComponent<Component::Texture>(entity)) _textureComponents--;
            if(HasComponent<Component::Text>(entity)) {
                _textComponents -= (uint32_t)GetComponent<Component::Text>(entity)._renderInfo.size();
            }
            if(_physics.HasCollider(_entt, entity)) {
                _physics.RemoveCollider(_entt, entity);
            }
            _entt.destroy(entity);
        }
        
        void DebugLine(const Util::Vec2F start, const Util::Vec2F end, const Util::Vec3F color);

        struct BoundingboxID {
            entt::entity e1, e2, e3, e4;
        };
        BoundingboxID CreateBoundingBox(const Util::Vec2F pos, const float rotation, const Util::Vec2F dimensions, const Component::PhysicsMaterial mat = Component::PhysicsMaterial::Rock());
        void ModifyBoundingBox(const BoundingboxID id, const Util::Vec2F pos, const float rotation, const Util::Vec2F dimensions, const Component::PhysicsMaterial mat = Component::PhysicsMaterial::Rock());
        void DeleteBoundingBox(const BoundingboxID id);

    private:
        friend struct Component::Texture;
        friend struct Component::Text;
        friend class Game;

        template <typename T>
		constexpr bool IsTextureComponent() { return std::is_same<T, Component::Texture>::value; }
        template <typename T>
		constexpr bool IsTextComponent() { return std::is_same<T, Component::Text>::value; }
        template <typename T>
		constexpr bool IsColliderComponent() { return std::is_same<T, Component::Collider>::value; }

        entt::registry _entt;
        Game* _game;
        Renderer::Window* _window;
        uint32_t _textureComponents = 0;
        uint32_t _textComponents = 0;
        Physics::PhysicsEngine _physics;
    };    
    
    // Template overload
    template<>
    inline void Scene::AddComponent<Component::Collider>(const entt::entity entity, const Component::Collider component) {
#ifdef __DEBUG__
        ASSERT(!_physics.HasCollider(_entt, entity), "[Scene] Cannot add a component to an entity that already has that component")
#endif
        _physics.AddCollider(_entt, entity, component);
    }
    // Template overload
    template<>
    inline void Scene::SetComponent<Component::Collider>(const entt::entity entity, const Component::Collider component) {
#ifdef __DEBUG__
        ASSERT(_physics.HasCollider(_entt, entity), "[Scene] Cannot set a component to an entity that doesnt't have that component")
#endif
        _physics.SetCollider(_entt, entity, component);
    }
    // Template overload
    template<>
    inline bool Scene::HasComponent<Component::Collider>(const entt::entity entity) {
        return _physics.HasCollider(_entt, entity);
    }
    // Template overload
    template<>
    inline Component::Collider& Scene::GetComponent<Component::Collider>(const entt::entity entity) {
        return _physics.GetCollider(_entt, entity);
    }

}

#endif