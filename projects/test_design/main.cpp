#include "core/window.hpp"
#include "core/core.hpp"

#include "gfx/context.hpp"
#include "gfx/vulkan/vulkan_context.hpp"
#include "gfx/opengl/opengl_context.hpp"

int main() {
    core::Window window{640, 420, "Test"};

    gfx::Context *context = gfx::Context::createContext(gfx::Api::Vulkan, window);
    auto vulkanContext = CAST(gfx::vulkan::VulkanContext, context);

    // auto vulkanContext = CAST(gfx::opengl::OpenglContext, context);  // this will throw an error in debug mode but not in release mode


    while (!window.shouldClose()) {
        core::Window::pollEvents();

        auto [width, height] = window.getDimensions();

    }
    

    return 0;
}