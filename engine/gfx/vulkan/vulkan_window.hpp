#ifndef GFX_VULKAN_WINDOW_HPP
#define GFX_VULKAN_WINDOW_HPP

#include "../apis.hpp"
#include "../../core/types.hpp"

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <array>

namespace gfx {

namespace vulkan {

class VulkanWindow {
public:
    VulkanWindow(GLFWwindow *window) : m_windowPtr(window) {}

    static constexpr Api api = Api::Vulkan;
    
    std::vector<const char *> getRequiredExtensions() const;

    VkBool32 createSurface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const;

    core::Extent2D getExtent() const;
private:
    GLFWwindow *m_windowPtr;
};

} // namespace vulkan  

} // namespace gfx

#endif