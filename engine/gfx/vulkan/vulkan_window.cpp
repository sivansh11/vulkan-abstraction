#include "vulkan_window.hpp"

namespace gfx {

namespace vulkan {

std::vector<const char *> VulkanWindow::getRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char ** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
}

VkBool32 VulkanWindow::createSurface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const {
    return glfwCreateWindowSurface(instance, m_windowPtr, allocator, surface);
}

core::Extent2D VulkanWindow::getExtent() const {
    int width, height;
    glfwGetFramebufferSize(m_windowPtr, &width, &height);
    return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}


} // namespace vulkan

} // namespace gfx
