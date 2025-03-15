#ifndef ENGINE_GAME_H
#define ENGINE_GAME_H

#include "core/PCH.h"
#include "core/Scene.h"
#include "renderer/Window.h"

namespace Engine {

    class Game {
    public:
        virtual void OnStart() {}
        virtual void OnSceneStart() {}

        int Run();

        template<class tScene>
        void StartScene() {
            StopScene();
            //start a new scene by first creating one
            _scene = new tScene(this);
            _scene->OnSceneStart();
            OnSceneStart();
        }
        void StopScene();

    private:
        Scene* _scene;
        Renderer::Window _window;
    };

}

#endif