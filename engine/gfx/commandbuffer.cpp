#include "commandbuffer.hpp"

namespace gfx {

// **********CommandPool::Builder**********
CommandPool::Builder::Builder() {}

CommandPool::Builder& CommandPool::Builder::setFlags(vk::CommandPoolCreateFlags flags) {
    m_commandPoolCreateInfo.setFlags(flags);
    return *this;
}

CommandPool::Builder& CommandPool::Builder::setQueueFamilyIndex(uint32_t queueFamilyIndex) {
    m_commandPoolCreateInfo.setQueueFamilyIndex(queueFamilyIndex);
    return *this;
}

CommandPool CommandPool::Builder::build(const Context *ctx) {
    vk::CommandPool commandPool;
    if (ctx->getDevice().createCommandPool(&m_commandPoolCreateInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create command pool!");
    }
    return {ctx, commandPool};
}

// **********CommandPool**********
CommandPool::CommandPool(const Context *ctx, vk::CommandPool commandPool) : m_ctx(ctx), m_commandPool(commandPool) {}

CommandPool::~CommandPool() {
    if (m_commandPool) m_ctx->getDevice().destroyCommandPool(m_commandPool);
    m_commandPool = VK_NULL_HANDLE;
}

CommandPool::CommandPool(CommandPool&& commandPool) : m_ctx(commandPool.m_ctx), m_commandPool(commandPool.m_commandPool) {
    commandPool.m_commandPool = VK_NULL_HANDLE;
}

std::vector<CommandBuffer> CommandPool::createCommandBuffer(uint32_t commandBufferCount) {
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo{}
        .setCommandPool(m_commandPool)
        .setCommandBufferCount(commandBufferCount)
        .setLevel(vk::CommandBufferLevel::ePrimary);
    
    std::vector<vk::CommandBuffer> vkCommandBuffers;
    vkCommandBuffers.resize(commandBufferCount);

    if (m_ctx->getDevice().allocateCommandBuffers(&commandBufferAllocateInfo, vkCommandBuffers.data()) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create command buffer!");
    }

    std::vector<CommandBuffer> commandBuffers;
    commandBuffers.reserve(vkCommandBuffers.size());

    for (auto& vkCommandBuffer : vkCommandBuffers) {
        commandBuffers.emplace_back(vkCommandBuffer);
    }

    return commandBuffers;
}

// **********CommandBuffer::Builder**********


// **********CommandBuffer**********
CommandBuffer::CommandBuffer(vk::CommandBuffer commandBuffer) : m_commandBuffer(commandBuffer) {

}

CommandBuffer::~CommandBuffer() {
    m_commandBuffer = VK_NULL_HANDLE;
}

CommandBuffer::CommandBuffer(CommandBuffer&& commandBuffer) : m_commandBuffer(commandBuffer.m_commandBuffer) {
    commandBuffer.m_commandBuffer = VK_NULL_HANDLE;
}

void CommandBuffer::begin(vk::CommandBufferUsageFlags commandBufferUsageFlags, const vk::CommandBufferInheritanceInfo& commandBufferInheritanceInfo) {
    vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo{}
        .setFlags(commandBufferUsageFlags)
        .setPInheritanceInfo(&commandBufferInheritanceInfo);

    if (m_commandBuffer.begin(&commandBufferBeginInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
}

void CommandBuffer::end() {
    m_commandBuffer.end();
}

void CommandBuffer::reset(vk::CommandBufferResetFlags resetFlags) {
    m_commandBuffer.reset(resetFlags);
}

} // namespace gfx
