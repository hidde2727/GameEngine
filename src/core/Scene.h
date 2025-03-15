#ifndef ENGINE_CORE_SCENE_H
#define ENGINE_CORE_SCENE_H

#include "core/PCH.h"

namespace Engine{
    class Scene {
    public:

        virtual void OnSceneStart();
        virtual void OnSceneStop();

    private: 
        entt::registry _entt;
    };
}

#endif