#include "vulkan_context.hpp"

#include <cassert>

namespace gfx {

namespace vulkan {

VulkanContext::VulkanContext(core::Window& window) : Context(VulkanContext::api), m_window(window) {
    createInstance();
}

VulkanContext::~VulkanContext() {

}

void VulkanContext::createInstance() {

}

} // namespace vulkan

} // namespace gfx
