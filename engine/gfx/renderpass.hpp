#ifndef GFX_RENDERPASS_HPP
#define GFX_RENDERPASS_HPP

#include "context.hpp"
#include "framebuffer.hpp"
#include "commandbuffer.hpp"

namespace gfx {

// NOTE: ATM HARDCODED ONLY 1 SUBPASS
class RenderPass {
public:
    struct Builder {
        Builder();
        Builder& addColorAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence);
        Builder& setDepthStencilAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence);
        Builder& addInputAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence);
        // Builder& addPreserveAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence);
        Builder& setResolveAttachmentRefrence(const vk::AttachmentReference& attachmentRefrence);
        Builder& addAttachmentDescription(const vk::AttachmentDescription& attachmentDescription);
        Builder& setPipelineBindPoint(vk::PipelineBindPoint pipelineBindPoint);
        Builder& setSubpassFlags(vk::SubpassDescriptionFlags subpassDescriptionFlags);
        Builder& setRenderpassFlags(vk::RenderPassCreateFlags renderpassCreateFlags);

        RenderPass build(const Context *ctx);
        
        std::vector<vk::AttachmentReference> m_colorAttachmentRefrences;
        vk::AttachmentReference m_depthStencilAttachmentRefrence;
        std::vector<vk::AttachmentReference> m_inputAttachmentRefrences;
        // std::vector<vk::AttachmentReference> m_preserveAttachmentRefrences;
        vk::AttachmentReference m_resolveAttachmentRefrence;
        std::vector<vk::AttachmentDescription> m_attachmentDescriptions;
        // hard coded 1 subpass and not a vector of subpasses
        vk::SubpassDescription m_subpassDescription;
        vk::RenderPassCreateInfo m_renderPassCreateInfo;
        bool setResolveAttachment = false, setDepthStencilAttachment = false;
    };
    
    RenderPass() : m_ctx(nullptr), m_renderPass(VK_NULL_HANDLE) {}
    
    ~RenderPass();

    RenderPass(RenderPass&& renderPass);
    RenderPass(const RenderPass&) = delete;

    RenderPass& operator=(RenderPass&& renderPass);

    vk::RenderPass getRenderPass() const { return m_renderPass; }

    struct BeginInfo {
        BeginInfo& setFrameBuffer(const FrameBuffer& frameBuffer);
        BeginInfo& setRenderArea(const vk::Rect2D& renderArea);
        BeginInfo& addClearValue(const vk::ClearValue& clearValue);
        
        vk::Framebuffer m_frameBuffer;
        vk::Rect2D m_renderArea;
        std::vector<vk::ClearValue> m_clearValues;
    };

    void begin(const CommandBuffer& commandBuffer, const BeginInfo& beginInfo);
    void end(const CommandBuffer& commandBuffer);

private:
    RenderPass(const Context *ctx, vk::RenderPass renderPass);

private:
    const Context *m_ctx;
    vk::RenderPass m_renderPass;
};

} // namespace gfx

#endif