#ifndef CORE_WINDOW_HPP
#define CORE_WINDOW_HPP

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace core {

class Window {
public:

    Window(const uint32_t width, const uint32_t height, const std::string& title);
    ~Window();
 
    Window(const Window&) = delete;               // not copyable
    Window& operator=(const Window&) = delete;  
    Window(const Window&&) = delete;              // not moveable
    Window& operator=(const Window&&) = delete;

    static void pollEvents() { return glfwPollEvents(); }

    bool shouldClose() const { return glfwWindowShouldClose(m_windowPtr); }

    vk::Extent2D getDimensions() const { return {m_width, m_height}; }
    float getAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
    const std::string& getTitle() const { return m_title; }

    const GLFWwindow *getWindowPtr() const { return m_windowPtr; }

    std::vector<const char *> getRequiredExtensions() const;

    VkBool32 createSurface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const;

private:
    GLFWwindow *m_windowPtr;
    uint32_t m_width, m_height;
    std::string m_title;
};

} // namespace core

#endif