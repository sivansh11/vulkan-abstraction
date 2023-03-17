#ifndef GFX_FRAMEBUFFER_HPP
#define GFX_FRAMEBUFFER_HPP

#include "device.hpp"
#include "../core/types.hpp"
#include "image.hpp"

namespace gfx {

class RenderPass;

class FrameBuffer {
public:
    struct Builder {
        Builder();

        Builder& setFlags(vk::FramebufferCreateFlags flags);
        // TODO: change this to take in renderpass abstraction class instance
        Builder& setRenderPass(const RenderPass& renderPass);
        Builder& addAttachment(const ImageView& imageView);
        Builder& setDimensions(const core::Dimensions& dimensions);

        FrameBuffer build(std::shared_ptr<Device> device);

        vk::FramebufferCreateInfo m_frameBufferCreateInfo;
        std::vector<vk::ImageView> m_imageViews;
    };

    FrameBuffer() : m_device(nullptr), m_frameBuffer(VK_NULL_HANDLE) {}

    ~FrameBuffer();
    FrameBuffer(FrameBuffer&& frameBuffer);

    FrameBuffer(const FrameBuffer&) = delete;

    vk::Framebuffer getFrameBuffer() const { return m_frameBuffer; }

private:
    FrameBuffer(std::shared_ptr<Device> device, vk::Framebuffer frameBuffer);

private:
    std::shared_ptr<Device> m_device;
    vk::Framebuffer m_frameBuffer;
    
};

} // namespace gfx

#endif