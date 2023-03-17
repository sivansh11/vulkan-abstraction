#include "framebuffer.hpp"

#include "renderpass.hpp"
#include "../core/log.hpp"

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

FrameBuffer FrameBuffer::Builder::build(std::shared_ptr<Device> device) {
    m_frameBufferCreateInfo.setAttachmentCount(m_imageViews.size())
                           .setPAttachments(m_imageViews.data());
    
    vk::Framebuffer frameBuffer;

    if (device->getDevice().createFramebuffer(&m_frameBufferCreateInfo, nullptr, &frameBuffer) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create frame buffer!");
    }

    INFO("Created FrameBuffer!");

    return {device, frameBuffer};
}

// **********FrameBuffer**********
FrameBuffer::FrameBuffer(std::shared_ptr<Device> device, vk::Framebuffer frameBuffer) : m_device(device), m_frameBuffer(frameBuffer) {

}

FrameBuffer::~FrameBuffer() {
    if (m_frameBuffer) m_device->getDevice().destroyFramebuffer(m_frameBuffer);
    m_frameBuffer = VK_NULL_HANDLE;
}

FrameBuffer::FrameBuffer(FrameBuffer&& frameBuffer) : m_device(frameBuffer.m_device), m_frameBuffer(frameBuffer.m_frameBuffer) {
    frameBuffer.m_frameBuffer = VK_NULL_HANDLE;
}

} // namespace gfx
