#include "window.hpp"

#include <iostream>

namespace core {

Window::Window(const uint32_t width, const uint32_t height, const std::string& title)
 : m_width(width), m_height(height), m_title(title) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize glfw!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_windowPtr = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
    if (!m_windowPtr) {
        throw std::runtime_error("Failed to create window!");
    }

}

Window::~Window() {
    glfwDestroyWindow(m_windowPtr);
    glfwTerminate();
}

} // namespace core
