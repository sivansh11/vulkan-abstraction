#ifndef GFX_COMMANDBUFFER_HPP
#define GFX_COMMANDBUFFER_HPP

#include "device.hpp"

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
    CommandBuffer(const CommandBuffer&) = default;

    vk::CommandBuffer get() const { return m_commandBuffer; }

private:
    vk::CommandBuffer m_commandBuffer;
};

class CommandPool {
public:
    struct Builder {
        Builder();

        Builder& setFlags(vk::CommandPoolCreateFlags flags);
        Builder& setQueueFamilyIndex(uint32_t queueFamilyIndex);
        CommandPool build(std::shared_ptr<Device> device);

        vk::CommandPoolCreateInfo m_commandPoolCreateInfo;
    };

    CommandPool() : m_device(nullptr), m_commandPool(VK_NULL_HANDLE) {}
    ~CommandPool();

    CommandPool(CommandPool&& commandPool);
    CommandPool(const CommandPool&) = delete;

    CommandPool& operator=(CommandPool&& commandPool);

    std::vector<CommandBuffer> createCommandBuffer(uint32_t commandBufferCount);

    vk::CommandPool get() const { return m_commandPool; }

private:
    CommandPool(std::shared_ptr<Device> device, vk::CommandPool commandPool);

private:    
    std::shared_ptr<Device> m_device;
    vk::CommandPool m_commandPool;
};

} // namespace gfx

#endif