#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "../gfx/device.hpp"
#include "../gfx/swapchain.hpp"
#include "../gfx/commandbuffer.hpp"
#include "../gfx/renderpass.hpp"

namespace renderer {

class Renderer {
public:
    struct Builder {
        Renderer build(std::shared_ptr<gfx::Device> device, gfx::SwapChain& swapChain);
    };

    Renderer(const Renderer&) = delete;

    ~Renderer();

    struct CommandBufferImageIndex {
        gfx::CommandBuffer commandBuffer;
        uint32_t imageIndex;
    };

    std::optional<CommandBufferImageIndex> begin();
    void end();
    void beginSwapChainRenderPass();
    void endSwapChainRenderPass();

    const gfx::RenderPass& getRenderPass() const { return m_renderPass; }

    uint32_t getCurrentFrameIndex() { return m_currentFrame; }

private:
    Renderer(std::shared_ptr<gfx::Device> device, gfx::SwapChain& swapChain);

    void createRenderPass();
    void createSyncObjects();
    void createSwapChainImageViews();
    void createSwapChainFrameBuffers();

    void advanceCurrentFrame() { m_currentFrame = (m_currentFrame + 1) % m_swapChain.MAX_FRAMES_IN_FLIGHT; }

private:
    bool didStartFrame{false};
    bool didStartRenderPass{false};
    uint32_t                         m_currentFrame{0};
    uint32_t                         m_imageIndex;
    std::shared_ptr<gfx::Device>     m_device;
    gfx::SwapChain&                  m_swapChain;
    std::vector<gfx::ImageView>      m_swapChainImageViews;
    std::vector<gfx::FrameBuffer>    m_swapChainFrameBuffers;
    gfx::CommandPool                 m_commandPool;
    gfx::RenderPass                  m_renderPass;
    std::vector<gfx::CommandBuffer>  m_commandBuffers;
    std::vector<gfx::Semaphore>      m_imageAvailableSemaphore;
    std::vector<gfx::Semaphore>      m_renderFinishedSemaphore;
    std::vector<gfx::Fence>          m_inFlightFence;

};

} // namespace renderer

#endif