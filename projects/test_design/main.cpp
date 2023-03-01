#include "core/window.hpp"
#include "core/core.hpp"

#include "gfx/vulkan/vulkan_window.hpp"
#include "gfx/vulkan/vulkan_context.hpp"
#include "gfx/context.hpp"
#include "gfx/swapchain.hpp"
// #include "gfx/opengl/opengl_context.hpp"

#include "core/log.hpp"

int main() {
    
    if (!core::Log::init()) {
        throw std::runtime_error("Failed to initialize logger!");
    }

    core::Window window{640, 420, "Test", gfx::Api::Opengl};

    auto context = gfx::Context::createContext(true, window);
    auto swapChain = gfx::SwapChain::createSwapChain(context, {640, 420});
    
    // auto vulkanContext = CAST(gfx::vulkan::VulkanContext, context);

    // auto openglContext = CAST(gfx::opengl::OpenglContext, context);  // this will throw an error in debug mode but not in release mode as the current api is vulkan and are attempting to cast to a opengl context

    while (!window.shouldClose()) {
        core::Window::pollEvents();

        auto [width, height] = window.getDimensions();

    }
    
    gfx::SwapChain::destroySwapChain(swapChain);
    gfx::Context::destroyContext(context);

    return 0;
}