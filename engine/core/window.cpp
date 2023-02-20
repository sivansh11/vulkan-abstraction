#include "window.hpp"

#include <iostream>

#include "../gfx/vulkan/vulkan_window.hpp"
// #include "../gfx/opengl/opengl_window.hpp"


namespace core {

Window::Window(const uint32_t width, const uint32_t height, const std::string& title, gfx::Api api)
 : m_width(width), m_height(height), m_title(title), m_apiSelected(api) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize glfw!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_windowPtr = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
    if (!m_windowPtr) {
        throw std::runtime_error("Failed to create window!");
    }

    switch (m_apiSelected) {
        case gfx::Api::Vulkan:
            m_internal = new gfx::vulkan::VulkanWindow{m_windowPtr};
            break;
        
        default:
            throw std::runtime_error("Not Implemented!");
            break;
    }
}

Window::~Window() {
    switch (m_apiSelected) {
        case gfx::Api::Vulkan:
            delete reinterpret_cast<gfx::vulkan::VulkanWindow *>(m_internal);
            break;
        
        default:
            std::cerr << "Not Implemented Api!\n";
            std::terminate();
    }
    glfwDestroyWindow(m_windowPtr);
    glfwTerminate();
}

} // namespace core
