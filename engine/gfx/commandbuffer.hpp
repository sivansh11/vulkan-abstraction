#ifndef GFX_COMMANDBUFFER_HPP
#define GFX_COMMANDBUFFER_HPP

#include "context.hpp"

namespace gfx {

class CommandBuffer {
public:
    CommandBuffer(vk::CommandBuffer commandBuffer);

    void reset(vk::CommandBufferResetFlags resetFlags = {});
    void begin(vk::CommandBufferUsageFlags commandBufferUsageFlags = {}, const vk::CommandBufferInheritanceInfo& commandBufferInheritanceInfo = {});
    void end();

    CommandBuffer() : m_commandBuffer(VK_NULL_HANDLE) {}
    ~CommandBuffer();

    CommandBuffer(CommandBuffer&& commandPool);
    CommandBuffer(const CommandBuffer&) = delete;

    vk::CommandBuffer getCommandBuffer() const { return m_commandBuffer; }

private:
    vk::CommandBuffer m_commandBuffer;
};

class CommandPool {
public:
    struct Builder {
        Builder();

        Builder& setFlags(vk::CommandPoolCreateFlags flags);
        Builder& setQueueFamilyIndex(uint32_t queueFamilyIndex);
        CommandPool build(const Context *ctx);

        vk::CommandPoolCreateInfo m_commandPoolCreateInfo;
    };

    CommandPool() : m_ctx(nullptr), m_commandPool(VK_NULL_HANDLE) {}
    ~CommandPool();

    CommandPool(CommandPool&& commandPool);
    CommandPool(const CommandPool&) = delete;

    std::vector<CommandBuffer> createCommandBuffer(uint32_t commandBufferCount);

private:
    CommandPool(const Context *ctx, vk::CommandPool commandPool);

private:    
    const Context *m_ctx;
    vk::CommandPool m_commandPool;
};

} // namespace gfx

#endif