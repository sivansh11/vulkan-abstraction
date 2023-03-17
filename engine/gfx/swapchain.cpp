#include "swapchain.hpp"

#include <limits>

namespace gfx {

SwapChain::SwapChain(std::shared_ptr<Device> device, uint32_t maxFramesInFlight) : m_device(device), m_deviceRef(m_device->getDevice()), MAX_FRAMES_IN_FLIGHT(maxFramesInFlight) {
    init();
}

SwapChain::~SwapChain() {
    cleanup();
}

void SwapChain::cleanup() {
    m_deviceRef.destroySwapchainKHR(m_swapChain);
    m_swapChainFrameBuffer.clear();
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
    m_device->getDevice().waitIdle();
    cleanup();
    createSwapChain();
    createImageViews();
    createFrameBuffer();
}

void SwapChain::init() {
    createSwapChain();
    createImageViews();
    createRenderPass();
    createFrameBuffer();
    createSyncObjects();
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

    m_swapChainImages = m_deviceRef.getSwapchainImagesKHR(m_swapChain);
    m_swapChainImageFormat = m_surfaceFormat.format;
    m_swapChainExtent = m_extent;

    INFO("Created SwapChain!");
}

void SwapChain::createImageViews() {
    m_swapChainImageViews.reserve(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        m_swapChainImageViews.emplace_back(gfx::ImageView::Builder{}
            .setImage(m_swapChainImages[i])
            .setFormat(m_swapChainImageFormat)
            .setViewType(vk::ImageViewType::e2D)
            .setComponents(vk::ComponentMapping{}
                .setR(vk::ComponentSwizzle::eIdentity)
                .setG(vk::ComponentSwizzle::eIdentity)
                .setB(vk::ComponentSwizzle::eIdentity)
                .setA(vk::ComponentSwizzle::eIdentity))
            .setSubresourceRangeAspectMask(vk::ImageAspectFlagBits::eColor)
            .setSubresourceRangeBaseMipLevel(0)
            .setSubresourceRangeLevelCount(1)
            .setSubresourceRangeBaseArrayLayer(0)
            .setSubresourceRangeLayerCount(1)
            .build(m_device));
    }
}

void SwapChain::createRenderPass() {
    m_renderPass = RenderPass::Builder{}
            .addAttachmentDescription(vk::AttachmentDescription{}
                .setFormat(getSwapchainImageFormat())
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR))
            .addColorAttachmentRefrence(vk::AttachmentReference{}
                .setAttachment(0)
                .setLayout(vk::ImageLayout::eColorAttachmentOptimal))
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .build(m_device);
}

void SwapChain::createFrameBuffer() {
    m_swapChainFrameBuffer.reserve(m_swapChainImageViews.size());
    for (auto& imageView : m_swapChainImageViews) {
        m_swapChainFrameBuffer.emplace_back(gfx::FrameBuffer::Builder{}
            .addAttachment(imageView)
            .setDimensions({m_swapChainExtent.width, m_swapChainExtent.height, 1})
            .setRenderPass(m_renderPass)
            .build(m_device));
    }
}

void SwapChain::createSyncObjects() {
    m_imageAvailableSemaphore.reserve(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphore.reserve(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFence.reserve(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_imageAvailableSemaphore.emplace_back(gfx::Semaphore::Builder{}.build(m_device));
        m_renderFinishedSemaphore.emplace_back(gfx::Semaphore::Builder{}.build(m_device));
        m_inFlightFence.emplace_back(gfx::Fence::Builder{}
            .setFlags(vk::FenceCreateFlagBits::eSignaled)
            .build(m_device));
    }
}

std::optional<uint32_t> SwapChain::acquireNextImage(uint64_t timeout) {
    m_inFlightFence[m_currentFrame].wait(timeout);
    uint32_t imageIndex;
    auto res = m_device->getDevice().acquireNextImageKHR(m_swapChain, timeout, m_imageAvailableSemaphore[m_currentFrame].getSemaphore(), VK_NULL_HANDLE, &imageIndex);
    if (res == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return std::nullopt;
    }
    else if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire next image!");
    }

    m_inFlightFence[m_currentFrame].reset();

    return {imageIndex};
}

} // namespace gfx
