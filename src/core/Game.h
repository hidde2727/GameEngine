#ifndef ENGINE_GAME_H
#define ENGINE_GAME_H

#include "core/PCH.h"
#include "core/Scene.h"

#include "renderer/Window.h"
#include "renderer/ImageLoader.h"
#include "renderer/TextLoader.h"

#include "network/WebHandler.h"

#include "util/FileManager.h"

#define ENGINE_GAME_TEXTUREMAP_ID 0
#define ENGINE_SCENE_TEXTUREMAP_ID 1

namespace Engine {
    typedef Renderer::AssetID AssetID;

    class Game : public Network::HTTP::Router {
    public:

        Game();

        /**
         * Should return the directories where resources are located.
         * The first element in the list has the hightest priority.
         * If there are two files with the same name, the one in the folder with the highest priority will be choosen.
         */
        virtual std::vector<std::string> GetResourceDirectories() const { return { "resources/engine/" }; }
        /** 
         * Should return the directories where the chache should be located.
         * Will by default choose to create a /cache/ directory in the first resource directory (Please override this choise).
        */
        virtual std::string GetCacheDirectory() const { return *GetResourceDirectories().begin() + "./cache/"; }
        virtual void OnStart() {}
        virtual void OnSceneStart() {}
        virtual void LoadAssets() {}

        int Run();

        template<class tScene>
        void StartScene() {
            StopScene();
            // Start a new scene by first creating one
            _scene = std::static_pointer_cast<Scene>(std::make_shared<tScene>(this, &_window));
            _window.StartAssetLoading(ENGINE_SCENE_TEXTUREMAP_ID);
            _scene->LoadAssets();
            _window.EndAssetLoading(ENGINE_SCENE_TEXTUREMAP_ID);
            _scene->OnSceneStart();
            Route(_scene);
            OnSceneStart();
        }
        void StopScene();

        /// @name Asset loading
        ///@{
        /** 
         * Set a unique name to be used all the times the exact same assets are loaded.
         * This name will be used to check the correct cache location.
         * @warning Should only be called inside the LoadAssets function.
         */
        void SetAssetCacheName(const std::string name);
        /// @warning Should only be called inside the LoadAssets function
        AssetID LoadTextureFile(const std::string file);
        /// @warning Should only be called inside the LoadAssets function
        AssetID LoadQrCode(const std::string file);
        /// @warning Should only be called inside the LoadAssets function
        AssetID LoadTextFile(const std::string file, const Renderer::Characters characters, const std::initializer_list<uint32_t> sizes);
        ///@}

        void SetCameraPosition(const Util::Vec2F pos);
        void DebugLine(const Util::Vec2F start, const Util::Vec2F end, const Util::Vec3F color);

    private:
        void Start();
        void Cleanup();
        void OnError(const std::string& message);

        std::shared_ptr<Scene> _scene;
        Renderer::Window _window;
        std::shared_ptr<Network::WebHandler> _webhandler;

        std::chrono::steady_clock::time_point _previousFrame;
    };

}

#endif