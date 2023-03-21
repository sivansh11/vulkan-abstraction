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

    vk::Extent2D getExtent() const { return m_swapChainExtent; }
    vk::Format getImageFormat() const { return m_swapChainImageFormat; }
    const std::vector<Image>& getImages() const { return m_swapChainImages; }
    std::optional<uint32_t> acquireNextImage(const Semaphore& imageAvailableSemaphore, const Fence& fence = {}, uint64_t timeout = UINT64_MAX);
    vk::SwapchainKHR get() const { return m_swapChain; }
    void recreateSwapChain();

public:
    const uint32_t MAX_FRAMES_IN_FLIGHT;

private:
    void init();

private:
    void createSwapChain();
    void getSwapChainImages();
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites);
    void cleanup();

private:
    std::shared_ptr<Device> m_device;
    const vk::Device& m_deviceRef;
    Device::SwapChainSupportDetails  m_swapChainSupportDetails; 
    vk::SurfaceFormatKHR             m_surfaceFormat;
    vk::PresentModeKHR               m_presentMode;
    vk::Extent2D                     m_extent;
    vk::SwapchainKHR                 m_swapChain;
    std::vector<Image>               m_swapChainImages;
    vk::Format                       m_swapChainImageFormat;
    vk::Extent2D                     m_swapChainExtent;
    std::vector<ImageView>           m_swapChainImageViews;
    // RenderPass                       m_renderPass;
    // std::vector<FrameBuffer>         m_swapChainFrameBuffer;
};

} // namespace gfx

#endif