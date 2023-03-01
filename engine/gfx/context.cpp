#include "context.hpp"

#include "vulkan/vulkan_context.hpp"
#include "opengl/opengl_context.hpp"

#include <iostream>

namespace gfx {

Context::Context(Api selectedApi) : m_selectedApi(selectedApi) {}

Context *Context::createContext(bool enableValidation, core::Window& window) {
    switch (window.getSelectedApi()) {
        case Api::Vulkan:
            return new vulkan::VulkanContext(window, enableValidation);
        
        case Api::Opengl:
            return new opengl::OpenglContext(window, enableValidation);
        
        default:
            throw std::runtime_error("Not Implemented!");
    }
}

void Context::destroyContext(Context *ctx) {
    assert(ctx != nullptr);
    delete ctx;
}

} // namespace gfx
