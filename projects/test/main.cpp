#include "core/window.hpp"
#include "core/core.hpp"

int main() {
    core::Window window{640, 420, "Test"};

    while (!window.shouldClose()) {
        core::Window::pollEvents();

        auto [width, height] = window.getDimensions();

    }
    

    return 0;
}