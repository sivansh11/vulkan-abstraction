#include "syncobjects.hpp"

namespace gfx {

// **********Semaphore::Builder**********
Semaphore Semaphore::Builder::build(std::shared_ptr<Device> device) {
    vk::Semaphore semaphore;
    if (device->getDevice().createSemaphore(&m_semaphoreCreateInfo, nullptr, &semaphore) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create semaphore!");
    }

    INFO("Created Semaphore!");

    return {device, semaphore};
}

// **********Semaphore**********
Semaphore::Semaphore(std::shared_ptr<Device> device, vk::Semaphore semaphore) : m_device(device), m_semaphore(semaphore) {

}

Semaphore::~Semaphore() {
    if (m_semaphore) m_device->getDevice().destroySemaphore(m_semaphore);
}

Semaphore::Semaphore(Semaphore&& semaphore) : m_device(semaphore.m_device), m_semaphore(semaphore.m_semaphore) {
    semaphore.m_semaphore = VK_NULL_HANDLE;
    semaphore.m_device = nullptr;
}

Semaphore& Semaphore::operator=(Semaphore&& semaphore) {
    m_device = semaphore.m_device;
    m_semaphore = semaphore.m_semaphore;
    semaphore.m_device = nullptr;
    semaphore.m_semaphore = VK_NULL_HANDLE;
    return *this;
}

// **********Fence::Builder**********
Fence::Builder::Builder() {}

Fence::Builder& Fence::Builder::setFlags(vk::FenceCreateFlags flags) {
    m_fenceCreateInfo.setFlags(flags);
    return *this;
}

Fence Fence::Builder::build(std::shared_ptr<Device> device) {
    vk::Fence fence;
    if (device->getDevice().createFence(&m_fenceCreateInfo, nullptr, &fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create fence!");
    }

    INFO("Created Fence!");

    return {device, fence};
}

// **********Fence**********
Fence::Fence(std::shared_ptr<Device> device, vk::Fence fence) : m_device(device), m_fence(fence) {

}

Fence::~Fence() {
    if (m_fence) m_device->getDevice().destroyFence(m_fence);
}

Fence::Fence(Fence&& fence) : m_device(fence.m_device), m_fence(fence.m_fence) {
    fence.m_fence = VK_NULL_HANDLE;
    fence.m_device = nullptr;
}

Fence& Fence::operator=(Fence&& fence) {
    m_device = fence.m_device;
    m_fence = fence.m_fence;
    fence.m_device = nullptr;
    fence.m_fence = VK_NULL_HANDLE;
    return *this;
}

void Fence::wait(uint64_t timeout) {
    if (m_device->getDevice().waitForFences(1, &m_fence, true, timeout) != vk::Result::eSuccess) {
        throw std::runtime_error("Wait for fence timed out!");
    }
}

void Fence::reset() {
    if (m_device->getDevice().resetFences(1, &m_fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to reset fence!");
    }
}

} // namespace gfx
