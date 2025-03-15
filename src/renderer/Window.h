#ifndef ENGINE_RENDERER_WINDOW_H
#define ENGINE_RENDERER_WINDOW_H

#include "core/PCH.h"

namespace Engine{
namespace Renderer{

    class Window {
    public:

        Window();
        ~Window();

        bool ShouldClose();
        void Update();

    private:
        GLFWwindow* _window = nullptr;
    };

}
}

#endif