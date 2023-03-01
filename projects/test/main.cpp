#include "core/window.hpp"
#include "core/core.hpp"
#include "core/log.hpp"

int main() {
    if (!core::Log::init()) {
        throw std::runtime_error("Failed to initialize logger!");
    }

    core::Window window{640, 420, "Test", gfx::Api::Vulkan};

    while (!window.shouldClose()) {
        core::Window::pollEvents();

        auto [width, height] = window.getDimensions();

    }
    

    return 0;
}