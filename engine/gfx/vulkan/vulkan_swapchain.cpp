#include "vulkan_swapchain.hpp"

#include <limits>

namespace gfx {

namespace vulkan {

VulkanSwapChain::VulkanSwapChain(VulkanContext *vulkanCtx, const vk::Extent2D& windowExtent) : SwapChain(Api::Vulkan), m_vulkanCtx(vulkanCtx), m_deviceRef(m_vulkanCtx->getDevice()) {
    assert(vulkanCtx != nullptr);
    assert(vulkanCtx->m_selectedApi == Api::Vulkan);
    init();
}

VulkanSwapChain::~VulkanSwapChain() {
    for (auto imageView : m_swapChainImageViews) {
        m_deviceRef.destroyImageView(imageView);
    }
    m_deviceRef.destroySwapchainKHR(m_swapChain);
}

vk::SurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    if (availableFormats.size() > 0) return availableFormats[0];

    throw std::runtime_error("Vulkan: No Available Surface formats!");
}

vk::PresentModeKHR VulkanSwapChain::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanSwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites) {
    if (capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilites.currentExtent;
    } else {
        auto [width, height] = reinterpret_cast<const VulkanWindow *>(m_vulkanCtx->getWindow().getInternal())->getExtent();

        VkExtent2D actualExtent{ width, height };
        actualExtent.width = std::clamp(actualExtent.width, capabilites.minImageExtent.width, capabilites.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilites.minImageExtent.height, capabilites.maxImageExtent.height);
        
        return actualExtent;
    }
}   

void VulkanSwapChain::init() {
    createSwapChain();
}

void VulkanSwapChain::createSwapChain() {
    m_swapChainSupportDetails = m_vulkanCtx->getSwapChainSupportDetails();

    m_surfaceFormat = chooseSwapSurfaceFormat(m_swapChainSupportDetails.formats);
    m_presentMode = choosePresentMode(m_swapChainSupportDetails.presentModes);
    m_extent = chooseSwapExtent(m_swapChainSupportDetails.capabilites);

    uint32_t imageCount = m_swapChainSupportDetails.capabilites.minImageCount + 1;
    if (m_swapChainSupportDetails.capabilites.maxImageCount > 0 && imageCount > m_swapChainSupportDetails.capabilites.maxImageCount) {
        imageCount = m_swapChainSupportDetails.capabilites.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChainCreateInfo = vk::SwapchainCreateInfoKHR{}
        .setSurface(m_vulkanCtx->getSurface())
        .setMinImageCount(imageCount)
        .setImageFormat(m_surfaceFormat.format)
        .setImageColorSpace(m_surfaceFormat.colorSpace)
        .setImageExtent(m_extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    auto queueFamilyIndices = m_vulkanCtx->getQueueFamilyIndices();
    uint32_t queueIndices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
                           .setQueueFamilyIndexCount(2)
                           .setPQueueFamilyIndices(queueIndices);
    } else {
        swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    swapChainCreateInfo.setPreTransform(m_swapChainSupportDetails.capabilites.currentTransform)
                       .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                       .setPresentMode(m_presentMode)
                       .setClipped(vk::Bool32{VK_TRUE})
                       .setOldSwapchain({VK_NULL_HANDLE});
    
    if (m_deviceRef.createSwapchainKHR(&swapChainCreateInfo, nullptr, &m_swapChain) != vk::Result::eSuccess) {
        throw std::runtime_error("Vulkan: Failed to create SwapChain!");
    }

    m_swapChainImages = m_deviceRef.getSwapchainImagesKHR(m_swapChain);
    m_swapChainImageFormat = m_surfaceFormat.format;
    m_swapChainExtent = m_extent;
}

void VulkanSwapChain::createImageViews() {
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo{}
            .setImage(m_swapChainImages[i])
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(m_swapChainImageFormat)
            .setComponents(vk::ComponentMapping{}
                .setR(vk::ComponentSwizzle::eIdentity)
                .setG(vk::ComponentSwizzle::eIdentity)
                .setB(vk::ComponentSwizzle::eIdentity)
                .setA(vk::ComponentSwizzle::eIdentity))
            .setSubresourceRange(vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1));
        if (m_deviceRef.createImageView(&imageViewCreateInfo, nullptr, &m_swapChainImageViews[i]) != vk::Result::eSuccess) {
            throw std::runtime_error("Vulkan: Failed to create Image View!");
        }
    }
}

} // namespace vulkan

} // namespace gfx
