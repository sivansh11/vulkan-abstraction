#ifndef GFX_RENDERER_HPP
#define GFX_RENDERER_HPP

#include "device.hpp"
#include "swapchain.hpp"
#include "commandbuffer.hpp"

namespace gfx {

class Renderer {
public:
    struct Builder {
        Renderer build(std::shared_ptr<Device> device, SwapChain& swapChain);
    };

    Renderer(const Renderer&) = delete;

    ~Renderer() = default;

    struct CommandBufferImageIndex {
        CommandBuffer commandBuffer;
        uint32_t imageIndex;
    };

    std::optional<CommandBufferImageIndex> begin();
    void end();
    void beginSwapChainRenderPass();
    void endSwapChainRenderPass();

private:
    Renderer(std::shared_ptr<Device> device, SwapChain& swapChain);

private:
    std::shared_ptr<Device> m_device;
    SwapChain&           m_swapChain;
    CommandPool                m_commandPool;
    std::vector<CommandBuffer> m_commandBuffers;
    uint32_t m_imageIndex;

};

} // namespace gfx

#endif