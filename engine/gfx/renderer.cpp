#include "renderer.hpp"

namespace gfx {

// **********Renderer::Builder**********
Renderer Renderer::Builder::build(std::shared_ptr<Device> device, SwapChain& swapChain) {
    INFO("Created Renderer!");
    return {device, swapChain};
}

// **********Renderer**********
Renderer::Renderer(std::shared_ptr<Device> device, SwapChain& swapChain) : m_device(device), m_swapChain(swapChain) {
    m_commandPool = gfx::CommandPool::Builder{}
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(device->getQueueFamilyIndices().graphicsFamily.value())
        .build(m_device);

    m_commandBuffers = m_commandPool.createCommandBuffer(m_swapChain.MAX_FRAMES_IN_FLIGHT);
}

std::optional<Renderer::CommandBufferImageIndex> Renderer::begin() {
    auto res = m_swapChain.acquireNextImage();
    if (!res) 
        return std::nullopt;
    m_imageIndex = res.value();
    auto& commandBuffer = m_commandBuffers[m_swapChain.getCurrentFrameIndex()];
    commandBuffer.reset();
    commandBuffer.begin();
    return {{commandBuffer, m_imageIndex}};
}

void Renderer::end() {
    auto& commandBuffer = m_commandBuffers[m_swapChain.getCurrentFrameIndex()];
    commandBuffer.end();
    m_device->submit(
        gfx::Device::QueueSubmitInfo{}
            .addCommandBuffer(commandBuffer)
            .addWaitSemaphore(m_swapChain.getCurrentImageAvailableSemaphore())
            .addSignalSemaphore(m_swapChain.getCurrentRenderFinishedSemaphore())
            .setFence(m_swapChain.getCurrentInFlightFence())
            .addWaitStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    );
    auto res = m_device->present(
        gfx::Device::PresentInfo{}
            .addWaitSemaphore(m_swapChain.getCurrentRenderFinishedSemaphore())
            .setImageIndex(m_imageIndex)
            .setSwapChain(m_swapChain)
    );

    if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR) {
        m_swapChain.recreateSwapChain();
    } else if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    m_swapChain.advanceCurrentFrame();
}

void Renderer::beginSwapChainRenderPass() {
    auto& renderPass = m_swapChain.getSwapChainRenderPass();
    auto& commandBuffer = m_commandBuffers[m_swapChain.getCurrentFrameIndex()];
    renderPass.begin(commandBuffer, gfx::RenderPass::BeginInfo{}
            .setFrameBuffer(m_swapChain.getSwapChainFrameBuffers()[m_imageIndex])
            .setRenderArea(vk::Rect2D{}
                .setOffset({0, 0})
                .setExtent(m_swapChain.getSwapchainExtent()))            
            .addClearValue(vk::ClearValue{}
                .setColor(vk::ClearColorValue{std::array{0.f, 0.f, 0.f, 1.f}})));
}

void Renderer::endSwapChainRenderPass() {
    auto& renderPass = m_swapChain.getSwapChainRenderPass();
    auto& commandBuffer = m_commandBuffers[m_swapChain.getCurrentFrameIndex()];
    renderPass.end(commandBuffer);
}

} // namespace gfx
