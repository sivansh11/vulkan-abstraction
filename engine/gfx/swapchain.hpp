#ifndef GFX_SWAPCHAIN_HPP
#define GFX_SWAPCHAIN_HPP

#include "context.hpp"
#include "image.hpp"
#include "framebuffer.hpp"
#include "renderpass.hpp"
#include "syncobjects.hpp"

namespace gfx {

class SwapChain {
public:
    SwapChain(const Context *ctx, const vk::Extent2D& windowExtent);

    ~SwapChain();

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain(const SwapChain&&) = delete;
    SwapChain& operator=(const SwapChain&&) = delete;

    vk::Extent2D getSwapchainExtent() const { return m_swapChainExtent; }
    vk::Format getSwapchainImageFormat() const { return m_swapChainImageFormat; }
    RenderPass& getSwapChainRenderPass() { return m_renderPass; }
    std::vector<FrameBuffer>& getSwapChainFrameBuffers() { return m_swapChainFrameBuffer; }
    // currently hardcoding sync objects in swapchain
    uint32_t acquireNextImage(uint64_t timeout = UINT64_MAX);
    Semaphore& getImageAvailableSemaphore() { return m_imageAvailableSemaphore; }
    Semaphore& getRenderFinishedSemaphore() { return m_renderFinishedSemaphore; }
    Fence& getInFlightFence() { return m_inFlightFence; }
    vk::SwapchainKHR getSwapChain() const { return m_swapChain; }

private:
    void init();

private:
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFrameBuffer();
    void createSyncObjects();
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites);

private:
    const Context *m_ctx;
    const vk::Device& m_deviceRef;

    Context::SwapChainSupportDetails m_swapChainSupportDetails; 
    vk::SurfaceFormatKHR             m_surfaceFormat;
    vk::PresentModeKHR               m_presentMode;
    vk::Extent2D                     m_extent;
    vk::SwapchainKHR                 m_swapChain;
    std::vector<vk::Image>           m_swapChainImages;
    vk::Format                       m_swapChainImageFormat;
    vk::Extent2D                     m_swapChainExtent;
    std::vector<ImageView>           m_swapChainImageViews;
    RenderPass                       m_renderPass;
    std::vector<FrameBuffer>         m_swapChainFrameBuffer;
    gfx::Semaphore                   m_imageAvailableSemaphore;
    gfx::Semaphore                   m_renderFinishedSemaphore;
    gfx::Fence                       m_inFlightFence;
};

} // namespace gfx

#endif