#include "swapchain.hpp"

#include <limits>

namespace gfx {

SwapChain::SwapChain(std::shared_ptr<Device> device, uint32_t maxFramesInFlight) : m_device(device), m_deviceRef(m_device->get()), MAX_FRAMES_IN_FLIGHT(maxFramesInFlight) {
    init();
}

SwapChain::~SwapChain() {
    cleanup();
}

void SwapChain::cleanup() {
    m_deviceRef.destroySwapchainKHR(m_swapChain);
    m_swapChainImageViews.clear();
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    if (availableFormats.size() > 0) return availableFormats[0];

    throw std::runtime_error("Vulkan: No Available Surface formats!");
}

vk::PresentModeKHR SwapChain::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites) {
    if (capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilites.currentExtent;
    } else {
        vk::Extent2D actualExtent = m_device->getWindow().getDimensions();
        actualExtent.width = std::clamp(actualExtent.width, capabilites.minImageExtent.width, capabilites.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilites.minImageExtent.height, capabilites.maxImageExtent.height);
        
        return actualExtent;
    }
}   

void SwapChain::recreateSwapChain() {
    m_device->get().waitIdle();
    cleanup();
    createSwapChain();
    getSwapChainImages();
}

void SwapChain::init() {
    createSwapChain();
    getSwapChainImages();
}

void SwapChain::createSwapChain() {
    m_swapChainSupportDetails = m_device->getSwapChainSupportDetails();

    m_surfaceFormat = chooseSwapSurfaceFormat(m_swapChainSupportDetails.formats);
    m_presentMode = choosePresentMode(m_swapChainSupportDetails.presentModes);
    m_extent = chooseSwapExtent(m_swapChainSupportDetails.capabilites);

    uint32_t imageCount = m_swapChainSupportDetails.capabilites.minImageCount + 1;
    if (m_swapChainSupportDetails.capabilites.maxImageCount > 0 && imageCount > m_swapChainSupportDetails.capabilites.maxImageCount) {
        imageCount = m_swapChainSupportDetails.capabilites.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChainCreateInfo = vk::SwapchainCreateInfoKHR{}
        .setSurface(m_device->getSurface())
        .setMinImageCount(imageCount)
        .setImageFormat(m_surfaceFormat.format)
        .setImageColorSpace(m_surfaceFormat.colorSpace)
        .setImageExtent(m_extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    auto queueFamilyIndices = m_device->getQueueFamilyIndices();
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

    m_swapChainImageFormat = m_surfaceFormat.format;
    m_swapChainExtent = m_extent;

    INFO("Created SwapChain!");
}

void SwapChain::getSwapChainImages() {
    auto images = m_deviceRef.getSwapchainImagesKHR(m_swapChain);
    m_swapChainImages.reserve(images.size());
    for (auto image : images) {
        m_swapChainImages.emplace_back(m_device, image, m_swapChainImageFormat);
    }
}

std::optional<uint32_t> SwapChain::acquireNextImage(const Semaphore& imageAvailableSemaphore, const Fence& fence, uint64_t timeout) {
    uint32_t imageIndex;
    auto res = m_device->get().acquireNextImageKHR(m_swapChain, timeout, imageAvailableSemaphore.get(), fence.get(), &imageIndex);
    if (res == vk::Result::eErrorOutOfDateKHR) {
        return std::nullopt;
    }
    else if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire next image!");
    }

    return {imageIndex};
}

} // namespace gfx
