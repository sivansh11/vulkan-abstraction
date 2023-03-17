#ifndef GFX_SWAPCHAIN_HPP
#define GFX_SWAPCHAIN_HPP

#include "device.hpp"
#include "image.hpp"
#include "framebuffer.hpp"
#include "renderpass.hpp"
#include "syncobjects.hpp"

namespace gfx {

class SwapChain {
public:
    SwapChain(std::shared_ptr<Device> device, uint32_t maxFramesInFlight);

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
    std::optional<uint32_t> acquireNextImage(uint64_t timeout = UINT64_MAX);
    Semaphore& getCurrentImageAvailableSemaphore() { return m_imageAvailableSemaphore[m_currentFrame]; }
    Semaphore& getCurrentRenderFinishedSemaphore() { return m_renderFinishedSemaphore[m_currentFrame]; }
    Fence& getCurrentInFlightFence() { return m_inFlightFence[m_currentFrame]; }
    vk::SwapchainKHR getSwapChain() const { return m_swapChain; }
    uint32_t getCurrentFrameIndex() const { return m_currentFrame; }
    void advanceCurrentFrame() { m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; }
    void recreateSwapChain();

public:
    const uint32_t MAX_FRAMES_IN_FLIGHT;

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
    void cleanup();

private:
    std::shared_ptr<Device> m_device;
    const vk::Device& m_deviceRef;

    uint32_t                         m_currentFrame{0};

    Device::SwapChainSupportDetails  m_swapChainSupportDetails; 
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
    std::vector<gfx::Semaphore>      m_imageAvailableSemaphore;
    std::vector<gfx::Semaphore>      m_renderFinishedSemaphore;
    std::vector<gfx::Fence>          m_inFlightFence;
};

} // namespace gfx

#endif