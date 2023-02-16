#include "context.hpp"

#include "vulkan/vulkan_context.hpp"

namespace gfx {

Context::Context(Api selectedApi) : m_selectedApi(selectedApi) {}

Context *Context::createContext(Api selectedApi, core::Window& window) {
    switch (selectedApi) {
        case Api::Vulkan:
            return new vulkan::VulkanContext(window);
            break;
        
        default:
            throw std::runtime_error("Not Implemented!");
            break;
    }
}

} // namespace gfx
