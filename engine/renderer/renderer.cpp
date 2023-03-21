#include "renderer.hpp"

namespace renderer {

// **********Renderer::Builder**********
Renderer Renderer::Builder::build(std::shared_ptr<gfx::Device> device, gfx::SwapChain& swapChain) {
    INFO("Created Renderer!");
    return {device, swapChain};
}

// **********Renderer**********
Renderer::Renderer(std::shared_ptr<gfx::Device> device, gfx::SwapChain& swapChain) : m_device(device), m_swapChain(swapChain) {
    m_commandPool = gfx::CommandPool::Builder{}
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(device->getQueueFamilyIndices().graphicsFamily.value())
        .build(m_device);

    m_commandBuffers = m_commandPool.createCommandBuffer(m_swapChain.MAX_FRAMES_IN_FLIGHT);

    createRenderPass();
    createSyncObjects();
    createSwapChainImageViews();
    createSwapChainFrameBuffers();
}

Renderer::~Renderer() {

}

std::optional<Renderer::CommandBufferImageIndex> Renderer::begin() {
    assert(!didStartFrame && "cannot begin again while frame has already begin!");
    
    m_inFlightFence[m_currentFrame].wait();
    auto res = m_swapChain.acquireNextImage(m_imageAvailableSemaphore[m_currentFrame]);
    if (!res) {
        m_swapChain.recreateSwapChain();
        createSwapChainImageViews();
        createSwapChainFrameBuffers();
        return std::nullopt;
    } else {
        m_inFlightFence[m_currentFrame].reset();
    }
    m_imageIndex = res.value();
    auto& commandBuffer = m_commandBuffers[m_currentFrame];
    commandBuffer.reset();
    commandBuffer.begin();
    didStartFrame = true;
    return {{commandBuffer, m_imageIndex}};
}

void Renderer::end() {
    assert(didStartFrame && "cannot end frame when frame hasnt started!");
    
    auto& commandBuffer = m_commandBuffers[m_currentFrame];
    commandBuffer.end();
    m_device->submit(gfx::Device::QueueSubmitInfo{}
            .addCommandBuffer(commandBuffer)
            .addWaitSemaphore(m_imageAvailableSemaphore[m_currentFrame])
            .addSignalSemaphore(m_renderFinishedSemaphore[m_currentFrame])
            .setFence(m_inFlightFence[m_currentFrame])
            .addWaitStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    );
    auto res = m_device->present(gfx::Device::PresentInfo{}
            .addWaitSemaphore(m_renderFinishedSemaphore[m_currentFrame])
            .setImageIndex(m_imageIndex)
            .setSwapChain(m_swapChain)
    );

    if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR) {
        m_swapChain.recreateSwapChain();
        createSwapChainImageViews();
        createSwapChainFrameBuffers();
    } else if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    advanceCurrentFrame();
    didStartFrame = false;
}

void Renderer::beginSwapChainRenderPass() {
    assert(!didStartRenderPass && "cannot begin again while render pass has already begin!");

    auto& commandBuffer = m_commandBuffers[m_currentFrame];
    m_renderPass.begin(commandBuffer, gfx::RenderPass::BeginInfo{}
            .setFrameBuffer(m_swapChainFrameBuffers[m_imageIndex])
            .setRenderArea(vk::Rect2D{}
                .setOffset({0, 0})
                .setExtent(m_swapChain.getExtent()))            
            .addClearValue(vk::ClearValue{}
                .setColor(vk::ClearColorValue{std::array{0.f, 0.f, 0.f, 1.f}})));
    
    didStartRenderPass = true;
}

void Renderer::endSwapChainRenderPass() {
    assert(didStartRenderPass && "cannot end render pass when render pass hasnt started!");

    auto& commandBuffer = m_commandBuffers[m_currentFrame];
    m_renderPass.end(commandBuffer);
    didStartRenderPass = false;
}

void Renderer::createRenderPass() {
    m_renderPass = gfx::RenderPass::Builder{}
            .addAttachmentDescription(vk::AttachmentDescription{}
                .setFormat(m_swapChain.getImageFormat())
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

void Renderer::createSyncObjects() {
    m_imageAvailableSemaphore.reserve(m_swapChain.MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphore.reserve(m_swapChain.MAX_FRAMES_IN_FLIGHT);
    m_inFlightFence.reserve(m_swapChain.MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < m_swapChain.MAX_FRAMES_IN_FLIGHT; i++) {
        m_imageAvailableSemaphore.emplace_back(gfx::Semaphore::Builder{}.build(m_device));
        m_renderFinishedSemaphore.emplace_back(gfx::Semaphore::Builder{}.build(m_device));
        m_inFlightFence.emplace_back(gfx::Fence::Builder{}
            .setFlags(vk::FenceCreateFlagBits::eSignaled)
            .build(m_device));
    }
}

void Renderer::createSwapChainImageViews() {
    m_swapChainImageViews.clear();
    m_swapChainImageViews.reserve(m_swapChain.getImages().size());

    for (auto& image : m_swapChain.getImages()) {
        m_swapChainImageViews.emplace_back(gfx::ImageView::Builder{}
            .setImage(image)
            .setFormat(m_swapChain.getImageFormat())
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

void Renderer::createSwapChainFrameBuffers() {
    m_swapChainFrameBuffers.clear();
    m_swapChainFrameBuffers.reserve(m_swapChain.getImages().size());
    for (auto& imageView : m_swapChainImageViews) {
        m_swapChainFrameBuffers.emplace_back(gfx::FrameBuffer::Builder{}
            .addAttachment(imageView)
            .setDimensions({m_swapChain.getExtent().width, m_swapChain.getExtent().height, 1})
            .setRenderPass(m_renderPass)
            .build(m_device));
    }
}


} // namespace renderer
