#include "framebuffer.hpp"

#include "renderpass.hpp"

namespace gfx {

// **********FrameBuffer::Builder**********
FrameBuffer::Builder::Builder() {}

FrameBuffer::Builder& FrameBuffer::Builder::setFlags(vk::FramebufferCreateFlags flags) {
    m_frameBufferCreateInfo.setFlags(flags);
    return *this;
}

FrameBuffer::Builder& FrameBuffer::Builder::setRenderPass(const RenderPass& renderPass) {
    m_frameBufferCreateInfo.setRenderPass(renderPass.getRenderPass());
    return *this;
}

FrameBuffer::Builder& FrameBuffer::Builder::addAttachment(const ImageView& imageView) {
    m_imageViews.push_back(imageView.getImageView());
    return *this;
}

FrameBuffer::Builder& FrameBuffer::Builder::setDimensions(const core::Dimensions& dimensions) {
    m_frameBufferCreateInfo.setWidth(dimensions.width);
    m_frameBufferCreateInfo.setHeight(dimensions.height);
    m_frameBufferCreateInfo.setLayers(dimensions.depth);
    return *this;
}

FrameBuffer FrameBuffer::Builder::build(const Context *ctx) {
    m_frameBufferCreateInfo.setAttachmentCount(m_imageViews.size())
                           .setPAttachments(m_imageViews.data());
    
    vk::Framebuffer frameBuffer;

    if (ctx->getDevice().createFramebuffer(&m_frameBufferCreateInfo, nullptr, &frameBuffer) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create frame buffer!");
    }

    return {ctx, frameBuffer};
}

// **********FrameBuffer**********
FrameBuffer::FrameBuffer(const Context *ctx, vk::Framebuffer frameBuffer) : m_ctx(ctx), m_frameBuffer(frameBuffer) {

}

FrameBuffer::~FrameBuffer() {
    if (m_frameBuffer) m_ctx->getDevice().destroyFramebuffer(m_frameBuffer);
    m_frameBuffer = VK_NULL_HANDLE;
}

FrameBuffer::FrameBuffer(FrameBuffer&& frameBuffer) : m_ctx(frameBuffer.m_ctx), m_frameBuffer(frameBuffer.m_frameBuffer) {
    frameBuffer.m_frameBuffer = VK_NULL_HANDLE;
}

} // namespace gfx
