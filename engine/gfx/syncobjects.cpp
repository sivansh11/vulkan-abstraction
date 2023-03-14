#include "syncobjects.hpp"

namespace gfx {

// **********Semaphore::Builder**********
Semaphore Semaphore::Builder::build(const Context *ctx) {
    vk::Semaphore semaphore;
    if (ctx->getDevice().createSemaphore(&m_semaphoreCreateInfo, nullptr, &semaphore) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create semaphore!");
    }

    return {ctx, semaphore};
}

// **********Semaphore**********
Semaphore::Semaphore(const Context *ctx, vk::Semaphore semaphore) : m_ctx(ctx), m_semaphore(semaphore) {

}

Semaphore::~Semaphore() {
    if (m_semaphore) m_ctx->getDevice().destroySemaphore(m_semaphore);
}

Semaphore::Semaphore(Semaphore&& semaphore) : m_ctx(semaphore.m_ctx), m_semaphore(semaphore.m_semaphore) {
    semaphore.m_semaphore = VK_NULL_HANDLE;
}

Semaphore& Semaphore::operator=(Semaphore&& semaphore) {
    m_ctx = semaphore.m_ctx;
    m_semaphore = semaphore.m_semaphore;
    semaphore.m_ctx = nullptr;
    semaphore.m_semaphore = VK_NULL_HANDLE;
    return *this;
}

// **********Fence::Builder**********
Fence::Builder::Builder() {}

Fence::Builder& Fence::Builder::setFlags(vk::FenceCreateFlags flags) {
    m_fenceCreateInfo.setFlags(flags);
    return *this;
}

Fence Fence::Builder::build(const Context *ctx) {
    vk::Fence fence;
    if (ctx->getDevice().createFence(&m_fenceCreateInfo, nullptr, &fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create fence!");
    }

    return {ctx, fence};
}

// **********Fence**********
Fence::Fence(const Context *ctx, vk::Fence fence) : m_ctx(ctx), m_fence(fence) {

}

Fence::~Fence() {
    if (m_fence) m_ctx->getDevice().destroyFence(m_fence);
}

Fence::Fence(Fence&& fence) : m_ctx(fence.m_ctx), m_fence(fence.m_fence) {
    fence.m_fence = VK_NULL_HANDLE;
}

Fence& Fence::operator=(Fence&& fence) {
    m_ctx = fence.m_ctx;
    m_fence = fence.m_fence;
    fence.m_ctx = nullptr;
    fence.m_fence = VK_NULL_HANDLE;
    return *this;
}

void Fence::wait(uint64_t timeout) {
    if (m_ctx->getDevice().waitForFences(1, &m_fence, true, timeout) != vk::Result::eSuccess) {
        throw std::runtime_error("Wait for fence timed out!");
    }
}

void Fence::reset() {
    if (m_ctx->getDevice().resetFences(1, &m_fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to reset fence!");
    }
}

} // namespace gfx
