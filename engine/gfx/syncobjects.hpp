#ifndef GFX_SYNCOBJECTS_HPP
#define GFX_SYNCOBJECTS_HPP

#include "context.hpp"

namespace gfx {

class Semaphore {
public:
    struct Builder {
        Semaphore build(const Context *ctx);

        vk::SemaphoreCreateInfo m_semaphoreCreateInfo;
    };

    /**
     * @brief Used to synchronise GPU operations in same or different queues 
     * 2 states signaled and non-signaled
     * example: work A and B needs to be done in order
     * vkQueueSubmit(work: A, signal: S, wait: None)
     * vkQueueSubmit(work: B, signal: None, wait: S)
     * 
     * when A is done, it will signal semaphore S
     * B will wait for S to be signaled
     * 
     * @param ctx 
     */
    Semaphore() : m_ctx(nullptr), m_semaphore(VK_NULL_HANDLE) {}

    ~Semaphore();

    Semaphore(Semaphore&& semaphore);
    Semaphore(const Semaphore&) = delete;

    Semaphore& operator=(Semaphore&& semaphore);

    vk::Semaphore getSemaphore() const { return m_semaphore; }

private:
    Semaphore(const Context *ctx, vk::Semaphore semaphore);

private:
    const Context *m_ctx;
    vk::Semaphore m_semaphore;
};

class Fence {
public:
    struct Builder {
        Builder();

        Builder& setFlags(vk::FenceCreateFlags flags);
        Fence build(const Context *ctx);

        vk::FenceCreateInfo m_fenceCreateInfo;
    };

    /**
     * @brief Sync gpu and cpu
     * again 2 states signaled and non signaled
     * example:
     * cpu needs to wait for gpu to complete A
     * vkQueueSubmit(work: A, fence: F)
     * vkWaitForFence(F)
     * 
     * @param ctx 
     */
    
    Fence() : m_ctx(nullptr), m_fence(VK_NULL_HANDLE) {}

    ~Fence();
    
    Fence(Fence&& fence);
    Fence(const Fence&) = delete;

    Fence& operator=(Fence&& fence);

    void wait(uint64_t timeout = UINT64_MAX);
    void reset();

    vk::Fence getFence() const { return m_fence; }

private:
    Fence(const Context *ctx, vk::Fence fence);

private:
    const Context *m_ctx;
    vk::Fence m_fence;
};

} // namespace gfx

#endif