#include "Game.h"

namespace Engine {

int Game::Run() {
    OnStart();

    while(!_window.ShouldClose()) {
        _window.Update();
        _window.Draw();
    }

    return 0;
}

void Game::StopScene() {
    if (_scene) {
        _scene->OnSceneStop();
        delete _scene;
        _scene = nullptr;
    }

}

}