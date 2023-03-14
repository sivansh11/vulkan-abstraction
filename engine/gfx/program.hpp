#ifndef GFX_PROGRAM_HPP
#define GFX_PROGRAM_HPP

#include "context.hpp"
#include "renderpass.hpp"
#include "commandbuffer.hpp"

#include <filesystem>
#include <set>
#include <string>

namespace gfx {

class GraphicsProgram {
public:
    struct Builder {
        Builder();
        Builder& addShaderFromPath(const std::filesystem::path& shaderPath);
        
        // dynamic states
        Builder& addDynamicState(vk::DynamicState dynamicState);
        
        // pipeline input assembly
        Builder& setInputAssemblyTopology(vk::PrimitiveTopology topology);
        Builder& setInputAssemblyPrimitiveRestartEnable(bool enable);

        // viewport and scissor
        Builder& addViewport(const vk::Viewport& viewport);
        Builder& addScissor(const vk::Rect2D& scissor);

        // rasterizer
        Builder& setRasterizerDepthClampEnable(bool enable);
        Builder& setRasterizerDiscardEnable(bool enable);
        Builder& setRasterizerPolygonMode(vk::PolygonMode polygonMode);
        Builder& setRasterizerLineWidth(float lineWidth);
        Builder& setRasterizerCullMode(vk::CullModeFlagBits cullMode);
        Builder& setRasterizerFrontFace(vk::FrontFace frontFace);
        Builder& setRasterizerDepthBiasEnable(bool enable);
        Builder& setRasterizerDepthBiasConstantFactor(float factor);
        Builder& setRasterizerDepthBiasClamp(float factor);
        Builder& setRasterizerDepthBiasSlopeFactor(float factor);

        // multisampling
        Builder& setMultisamplingSampleShadingEnable(bool enable);
        Builder& setMultisamplinRasterizationSamples(vk::SampleCountFlagBits sampleCount);
        Builder& setMultisamplinMinSampleShading(float factor);
        Builder& setMultisamplingSampleMask(vk::SampleMask sampleMask);
        Builder& setMultisamplingAlphaToCoverageEnable(bool enable);
        Builder& setMultisamplingAlphaToOneEnable(bool enable);

        // color blend attachment state
        Builder& addColorBlendAttachmentState(const vk::PipelineColorBlendAttachmentState& pipelineColorBlendAttachmentState);
        
        // color blend state create info
        Builder& setColorBlendStateLogicOpEnable(bool enable);
        Builder& setColorBlendStateLogicOp(vk::LogicOp logicOp);
        Builder& setColorBlendStateBlendConstantR(float factor);
        Builder& setColorBlendStateBlendConstantG(float factor);
        Builder& setColorBlendStateBlendConstantB(float factor);
        Builder& setColorBlendStateBlendConstantA(float factor);

        // pipeline layout create info
        Builder& addLayoutPushConstantRange(const vk::PushConstantRange& pushConstantRange);
        Builder& addLayoutDescriptorSet(const vk::DescriptorSetLayout& descriptorSetLayout);

        // TODO: add setVertexInput 
        // Builder& setVertexInput();

        // renderpass
        Builder& setRenderPass(const RenderPass& renderPass);


        GraphicsProgram build(const Context *ctx);
        // GraphicsProgram buildComputeProgram(const Context *ctx);

        // TODO: add depth stencil state

        std::set<std::filesystem::path> m_shaderPaths;
        std::vector<vk::DynamicState> m_dynamicStates;
        vk::PipelineInputAssemblyStateCreateInfo m_pipelineInputAssemblyStateCreateInfo;
        std::vector<vk::Viewport> m_viewports;
        std::vector<vk::Rect2D> m_scissors;
        vk::PipelineRasterizationStateCreateInfo m_pipelineRasterizationStateCreateInfo;
        vk::SampleMask m_sampleMask;
        vk::PipelineMultisampleStateCreateInfo m_pipelineMultisampleStateCreateInfo;
        std::vector<vk::PipelineColorBlendAttachmentState> m_pipelineColorBlendAttachmentStates;
        vk::PipelineColorBlendStateCreateInfo m_pipelineColorBlendStateCreateInfo;
        std::vector<vk::PushConstantRange> m_pushConstantRanges;
        std::vector<vk::DescriptorSetLayout> m_descriptorSetLayout;
        std::vector<vk::SubpassDescription> m_subpassDescriptions;
        std::vector<vk::AttachmentDescription> m_attachmentDescriptions;

        vk::RenderPass m_renderPass;
    };

    GraphicsProgram() : m_ctx(nullptr), m_pipeline(VK_NULL_HANDLE), m_pipelineLayout(VK_NULL_HANDLE), m_shaderModules{} {}

    ~GraphicsProgram();

    // TODO: add direct shader string compilation
    // void addShader(const std::string& shaderName, const std::string& shaderSource);

    void bind(const CommandBuffer& commandBuffer);

private:
    GraphicsProgram(const Context *ctx, const vk::Pipeline& pipeline, const vk::PipelineLayout& pipelineLayout, const std::vector<vk::ShaderModule>& shaderModules);

private:
    const Context *m_ctx;
    vk::Pipeline m_pipeline;
    vk::PipelineLayout m_pipelineLayout;
    std::vector<vk::ShaderModule> m_shaderModules;
};

} // namespace gfx

#endif