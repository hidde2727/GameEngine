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

    #define BOUNDINGBOXWIDTH 10000
    Scene::BoundingboxID Scene::CreateBoundingBox(const Util::Vec2F pos, const float rotation, const Util::Vec2F dimensions) {
        BoundingboxID ret;
        ret.e1 = CreateEntity(
            PositionComponent(pos+Util::Vec2F(0.5f*(-dimensions.x-BOUNDINGBOXWIDTH), 0).rotate(rotation), rotation), 
            RectangleColliderComponent(Util::Vec2F(BOUNDINGBOXWIDTH, dimensions.y), ColliderFlags::NoMove));
        ret.e2 = CreateEntity(
            PositionComponent(pos+Util::Vec2F(0.5f*(dimensions.x+BOUNDINGBOXWIDTH), 0).rotate(rotation), rotation), 
            RectangleColliderComponent(Util::Vec2F(BOUNDINGBOXWIDTH, dimensions.y), ColliderFlags::NoMove));
        ret.e3 = CreateEntity(
            PositionComponent(pos+Util::Vec2F(0, 0.5f*(-dimensions.y-BOUNDINGBOXWIDTH)).rotate(rotation), rotation), 
            RectangleColliderComponent(Util::Vec2F(dimensions.x, BOUNDINGBOXWIDTH), ColliderFlags::NoMove));
        ret.e4 = CreateEntity(
            PositionComponent(pos+Util::Vec2F(0, 0.5f*(dimensions.y+BOUNDINGBOXWIDTH)).rotate(rotation), rotation), 
            RectangleColliderComponent(Util::Vec2F(dimensions.x, BOUNDINGBOXWIDTH), ColliderFlags::NoMove));       
        return ret;
    }
    void Scene::ModifyBoundingBox(const BoundingboxID id, const Util::Vec2F pos, const float rotation, const Util::Vec2F dimensions) {
        SetComponent<PositionComponent>(id.e1, PositionComponent(pos+Util::Vec2F(0.5f*(-dimensions.x-BOUNDINGBOXWIDTH), 0).rotate(rotation), rotation));
        SetComponent<RectangleColliderComponent>(id.e1, RectangleColliderComponent(Util::Vec2F(BOUNDINGBOXWIDTH, dimensions.y), ColliderFlags::NoMove));
        SetComponent<PositionComponent>(id.e2, PositionComponent(pos+Util::Vec2F(0.5f*(dimensions.x+BOUNDINGBOXWIDTH), 0).rotate(rotation), rotation));
        SetComponent<RectangleColliderComponent>(id.e2, RectangleColliderComponent(Util::Vec2F(BOUNDINGBOXWIDTH, dimensions.y), ColliderFlags::NoMove));
        SetComponent<PositionComponent>(id.e3, PositionComponent(pos+Util::Vec2F(0, 0.5f*(-dimensions.y-BOUNDINGBOXWIDTH)).rotate(rotation), rotation)); 
        SetComponent<RectangleColliderComponent>(id.e3, RectangleColliderComponent(Util::Vec2F(dimensions.x, BOUNDINGBOXWIDTH), ColliderFlags::NoMove));
        SetComponent<PositionComponent>(id.e4, PositionComponent(pos+Util::Vec2F(0, 0.5f*(dimensions.y+BOUNDINGBOXWIDTH)).rotate(rotation), rotation)); 
        SetComponent<RectangleColliderComponent>(id.e4, RectangleColliderComponent(Util::Vec2F(dimensions.x, BOUNDINGBOXWIDTH), ColliderFlags::NoMove));  
    }
    void Scene::DeleteBoundingBox(const BoundingboxID id) {
        DeleteEntity(id.e1);
        DeleteEntity(id.e2);
        DeleteEntity(id.e3);
        DeleteEntity(id.e4);
    }
}