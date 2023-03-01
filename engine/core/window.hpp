#ifndef CORE_WINDOW_HPP
#define CORE_WINDOW_HPP

#include "../gfx/apis.hpp"
#include "types.hpp"

#define GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>

namespace core {

class Window {
public:

    Window(const uint32_t width, const uint32_t height, const std::string& title, gfx::Api api);
    ~Window();
 
    Window(const Window&) = delete;               // not copyable
    Window& operator=(const Window&) = delete;  
    Window(const Window&&) = delete;              // not moveable
    Window& operator=(const Window&&) = delete;

    static void pollEvents() { return glfwPollEvents(); }

    bool shouldClose() const { return glfwWindowShouldClose(m_windowPtr); }

    Extent2D getDimensions() const { return {m_width, m_height}; }
    float getAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
    const std::string& getTitle() const { return m_title; }

    const GLFWwindow *getWindowPtr() const { return m_windowPtr; }

    const void *getInternal() const { return m_internal; }

    gfx::Api getSelectedApi() const { return m_apiSelected; }

private:
    GLFWwindow *m_windowPtr;
    uint32_t m_width, m_height;
    std::string m_title;
    const gfx::Api m_apiSelected;
    void *m_internal;

};

} // namespace core

#endif