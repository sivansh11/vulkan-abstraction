#include "context.hpp"

#include "vulkan/vulkan_context.hpp"

#include <iostream>

namespace gfx {

Context::Context(Api selectedApi) : m_selectedApi(selectedApi) {}

Context *Context::createContext(bool enableValidation, core::Window& window) {
    switch (window.getSelectedApi()) {
        case Api::Vulkan:
            return new vulkan::VulkanContext(window, enableValidation);
            break;
        
        default:
            throw std::runtime_error("Not Implemented!");
            break;
    }
}

} // namespace gfx
