#include "core/window.hpp"
#include "core/core.hpp"

#include "gfx/context.hpp"
#include "gfx/swapchain.hpp"

#include "core/log.hpp"

int main() {
    
    if (!core::Log::init()) {
        throw std::runtime_error("Failed to initialize logger!");
    }

    core::Window window{640, 420, "Test"};

    gfx::Device device{window, true};

    gfx::SwapChain swapChain{&device, window.getDimensions()};
    
    while (!window.shouldClose()) {
        core::Window::pollEvents();

        auto [width, height] = window.getDimensions();

    }

    return 0;
}