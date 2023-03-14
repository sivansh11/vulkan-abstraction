#include "renderpass.hpp"

namespace gfx {

// **********RenderPass::Builder**********
RenderPass::Builder::Builder() {}

RenderPass::Builder& RenderPass::Builder::addColorAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence) {
    m_colorAttachmentRefrences.push_back(attachmentRefrence);
    return *this;
}

RenderPass::Builder& RenderPass::Builder::setDepthStencilAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence) {
    m_depthStencilAttachmentRefrence = attachmentRefrence;
    setDepthStencilAttachment = true;
    return *this;
}

RenderPass::Builder& RenderPass::Builder::addInputAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence) {
    m_inputAttachmentRefrences.push_back(attachmentRefrence);
    return *this;
}

// RenderPass::Builder& RenderPass::Builder::addPreserveAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence) {
//     m_preserveAttachmentRefrences.push_back(attachmentRefrence);
//     return *this;
// }

RenderPass::Builder& RenderPass::Builder::setResolveAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence) {
    m_resolveAttachmentRefrence = attachmentRefrence;
    setResolveAttachment = true;
    return *this;
}

RenderPass::Builder& RenderPass::Builder::addAttachmentDescription(const vk::AttachmentDescription& attachmentDescription) {
    m_attachmentDescriptions.push_back(attachmentDescription);
    return *this;
}

RenderPass::Builder& RenderPass::Builder::setPipelineBindPoint(vk::PipelineBindPoint pipelineBindPoint) {
    m_subpassDescription.setPipelineBindPoint(pipelineBindPoint);
    return *this;
}

RenderPass::Builder& RenderPass::Builder::setSubpassFlags(vk::SubpassDescriptionFlags subpassDescriptionFlags) {
    m_subpassDescription.setFlags(subpassDescriptionFlags);
    return *this;
}

RenderPass::Builder& RenderPass::Builder::setRenderpassFlags(vk::RenderPassCreateFlags renderpassCreateFlags) {
    m_renderPassCreateInfo.setFlags(renderpassCreateFlags);
    return *this;
}

RenderPass RenderPass::Builder::build(const Context *ctx) {

    m_subpassDescription.setColorAttachmentCount(m_colorAttachmentRefrences.size())
                        .setPColorAttachments(m_colorAttachmentRefrences.data())
                        .setInputAttachmentCount(m_inputAttachmentRefrences.size())
                        .setPInputAttachments(m_inputAttachmentRefrences.data());
    if (setDepthStencilAttachment) m_subpassDescription.setPDepthStencilAttachment(&m_depthStencilAttachmentRefrence);
    if (setResolveAttachment) m_subpassDescription.setPResolveAttachments(&m_resolveAttachmentRefrence);

    m_renderPassCreateInfo.setAttachmentCount(m_attachmentDescriptions.size())
                          .setPAttachments(m_attachmentDescriptions.data())
                          .setPSubpasses(&m_subpassDescription)
                          .setSubpassCount(1);
    
    vk::RenderPass renderPass;
    if (ctx->getDevice().createRenderPass(&m_renderPassCreateInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create render pass!");
    }

    return {ctx, renderPass};
}

// **********RenderPass**********
RenderPass::RenderPass(const Context *ctx, vk::RenderPass renderPass) : m_ctx(ctx), m_renderPass(renderPass) {}

RenderPass::~RenderPass() {
    if (m_renderPass) m_ctx->getDevice().destroyRenderPass(m_renderPass);
    m_renderPass = VK_NULL_HANDLE;
}

RenderPass::RenderPass(RenderPass&& renderPass) : m_ctx(renderPass.m_ctx), m_renderPass(renderPass.m_renderPass) {
    renderPass.m_renderPass = VK_NULL_HANDLE;
}

RenderPass& RenderPass::operator=(RenderPass&& renderPass) {
    m_ctx = renderPass.m_ctx;
    m_renderPass = renderPass.m_renderPass;
    renderPass.m_ctx = nullptr;
    renderPass.m_renderPass = VK_NULL_HANDLE;
    return *this;
}

RenderPass::BeginInfo& RenderPass::BeginInfo::setFrameBuffer(const FrameBuffer& frameBuffer) {
    m_frameBuffer = frameBuffer.getFrameBuffer();
    return *this;
}

RenderPass::BeginInfo& RenderPass::BeginInfo::setRenderArea(const vk::Rect2D& renderArea) {
    m_renderArea = renderArea;
    return *this;
}

RenderPass::BeginInfo& RenderPass::BeginInfo::addClearValue(const vk::ClearValue& clearValue) {
    m_clearValues.push_back(clearValue);
    return *this;
}

void RenderPass::begin(const CommandBuffer& commandBuffer, const BeginInfo& beginInfo) {
    vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo{}
        .setRenderPass(m_renderPass)
        .setFramebuffer(beginInfo.m_frameBuffer)
        .setRenderArea(beginInfo.m_renderArea)
        .setClearValueCount(beginInfo.m_clearValues.size())
        .setPClearValues(beginInfo.m_clearValues.data());
    
    commandBuffer.getCommandBuffer().beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
}

void RenderPass::end(const CommandBuffer& commandBuffer) {
    commandBuffer.getCommandBuffer().endRenderPass();
}

} // namespace gfx
