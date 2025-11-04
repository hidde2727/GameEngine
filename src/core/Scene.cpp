#include "core/Scene.h"
#include "core/Game.h"

namespace Engine{

    Scene::Scene(Game* game, Renderer::Window* window) : _game(game), _window(window), _physics(GetSceneBounds()) {
        _fileManager = &game->GetFileManager();
    }
    Scene::~Scene() {
        _entt.clear();
    }

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

    #define BOUNDINGBOXWIDTH 10000
    Scene::BoundingboxID Scene::CreateBoundingBox(const Util::Vec2F pos, const float rotation, const Util::Vec2F dimensions, const Component::PhysicsMaterial mat) {
        BoundingboxID ret;
        ret.e1 = CreateEntity(
            Component::Position(pos+Util::Vec2F(0.5f*(-dimensions.x-BOUNDINGBOXWIDTH), 0).rotate(rotation), rotation), 
            Component::Collider::StaticRect(Util::Vec2F(BOUNDINGBOXWIDTH, dimensions.y), Component::ColliderFlags::NoMove, mat));
        ret.e2 = CreateEntity(
            Component::Position(pos+Util::Vec2F(0.5f*(dimensions.x+BOUNDINGBOXWIDTH), 0).rotate(rotation), rotation), 
            Component::Collider::StaticRect(Util::Vec2F(BOUNDINGBOXWIDTH, dimensions.y), Component::ColliderFlags::NoMove, mat));
        ret.e3 = CreateEntity(
            Component::Position(pos+Util::Vec2F(0, 0.5f*(-dimensions.y-BOUNDINGBOXWIDTH)).rotate(rotation), rotation), 
            Component::Collider::StaticRect(Util::Vec2F(dimensions.x, BOUNDINGBOXWIDTH), Component::ColliderFlags::NoMove, mat));
        ret.e4 = CreateEntity(
            Component::Position(pos+Util::Vec2F(0, 0.5f*(dimensions.y+BOUNDINGBOXWIDTH)).rotate(rotation), rotation), 
            Component::Collider::StaticRect(Util::Vec2F(dimensions.x, BOUNDINGBOXWIDTH), Component::ColliderFlags::NoMove, mat));       
        return ret;
    }
    void Scene::ModifyBoundingBox(const BoundingboxID id, const Util::Vec2F pos, const float rotation, const Util::Vec2F dimensions, const Component::PhysicsMaterial mat) {
        SetComponent<Component::Position>(id.e1, Component::Position(pos+Util::Vec2F(0.5f*(-dimensions.x-BOUNDINGBOXWIDTH), 0).rotate(rotation), rotation));
        SetComponent<Component::Collider>(id.e1, Component::Collider::StaticRect(Util::Vec2F(BOUNDINGBOXWIDTH, dimensions.y), Component::ColliderFlags::NoMove, mat));
        SetComponent<Component::Position>(id.e2, Component::Position(pos+Util::Vec2F(0.5f*(dimensions.x+BOUNDINGBOXWIDTH), 0).rotate(rotation), rotation));
        SetComponent<Component::Collider>(id.e2, Component::Collider::StaticRect(Util::Vec2F(BOUNDINGBOXWIDTH, dimensions.y), Component::ColliderFlags::NoMove, mat));
        SetComponent<Component::Position>(id.e3, Component::Position(pos+Util::Vec2F(0, 0.5f*(-dimensions.y-BOUNDINGBOXWIDTH)).rotate(rotation), rotation)); 
        SetComponent<Component::Collider>(id.e3, Component::Collider::StaticRect(Util::Vec2F(dimensions.x, BOUNDINGBOXWIDTH), Component::ColliderFlags::NoMove, mat));
        SetComponent<Component::Position>(id.e4, Component::Position(pos+Util::Vec2F(0, 0.5f*(dimensions.y+BOUNDINGBOXWIDTH)).rotate(rotation), rotation)); 
        SetComponent<Component::Collider>(id.e4, Component::Collider::StaticRect(Util::Vec2F(dimensions.x, BOUNDINGBOXWIDTH), Component::ColliderFlags::NoMove, mat));  
    }
    void Scene::DeleteBoundingBox(const BoundingboxID id) {
        DeleteEntity(id.e1);
        DeleteEntity(id.e2);
        DeleteEntity(id.e3);
        DeleteEntity(id.e4);
    }
}