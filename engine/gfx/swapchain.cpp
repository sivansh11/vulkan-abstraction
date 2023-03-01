#include "swapchain.hpp"

#include "vulkan/vulkan_swapchain.hpp"

namespace gfx {

SwapChain::SwapChain(Api selectedApi) : m_selectedApi(selectedApi) {}

SwapChain *SwapChain::createSwapChain(Context *ctx, const core::Extent2D windowExtent) {
    assert(ctx != nullptr);
    switch (ctx->m_selectedApi) {
        case Api::Vulkan:
            assert(ctx->m_selectedApi == Api::Vulkan);
            return new vulkan::VulkanSwapChain(reinterpret_cast<vulkan::VulkanContext *>(ctx), vk::Extent2D{windowExtent.width, windowExtent.height});
        
        default:
            throw std::runtime_error("Not Implemented!");
    }
}

void SwapChain::destroySwapChain(SwapChain *swapChain) {
    assert(swapChain != nullptr);
    delete swapChain;
}

} // namespace gfx
