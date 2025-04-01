#ifndef ENGINE_GAME_H
#define ENGINE_GAME_H

#include "core/PCH.h"
#include "core/Scene.h"

#include "renderer/Window.h"
#include "renderer/ImageLoader.h"
#include "renderer/TextLoader.h"

#define ENGINE_GAME_TEXTUREMAP_ID 0
#define ENGINE_SCENE_TEXTUREMAP_ID 1

namespace Engine {
    typedef Renderer::AssetID AssetID;

    class Game {
    public:

        Game();

        virtual void OnStart() {}
        virtual void OnSceneStart() {}
        virtual void LoadAssets() {}

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

    private:
        Scene* _scene;
        Renderer::Window _window;
    };

}

#endif