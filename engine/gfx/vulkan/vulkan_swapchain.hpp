#ifndef GFX_VULKAN_SWAPCHAIN_HPP
#define GFX_VULKAN_SWAPCHAIN_HPP

#include "vulkan_context.hpp"
#include "../swapchain.hpp"

namespace gfx {

namespace vulkan {

class VulkanSwapChain : public SwapChain {
public:
    VulkanSwapChain(VulkanContext *vulkanCtx, const vk::Extent2D& windowExtent);

    ~VulkanSwapChain();

    VulkanSwapChain(const VulkanSwapChain&) = delete;
    VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;
    VulkanSwapChain(const VulkanSwapChain&&) = delete;
    VulkanSwapChain& operator=(const VulkanSwapChain&&) = delete;

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites);


private:
    void init();

private:
    void createSwapChain();
    void createImageViews();

private:
    VulkanContext *m_vulkanCtx;
    vk::Device& m_deviceRef;

    VulkanContext::SwapChainSupportDetails m_swapChainSupportDetails; 
    vk::SurfaceFormatKHR m_surfaceFormat;
    vk::PresentModeKHR m_presentMode;
    vk::Extent2D m_extent;
    vk::SwapchainKHR m_swapChain;
    std::vector<vk::Image> m_swapChainImages;
    vk::Format m_swapChainImageFormat;
    vk::Extent2D m_swapChainExtent;
    std::vector<vk::ImageView> m_swapChainImageViews;
};

} // namespace vulkan

} // namespace gfx

#endif
