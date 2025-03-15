#include "renderer/window.h"

namespace Engine{
namespace Renderer{
    
    Window::Window() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    }
    Window::~Window() {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(_window);
    }
    void Window::Update() {
        glfwPollEvents();
    }

}
}