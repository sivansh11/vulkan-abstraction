#ifndef GFX_PROGRAM_HPP
#define GFX_PROGRAM_HPP

#include "device.hpp"
#include "renderpass.hpp"
#include "commandbuffer.hpp"

#include <filesystem>
#include <set>
#include <string>

namespace gfx {

class GraphicsPipeline {
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
        // Builder& setMultisamplingSampleMask(vk::SampleMask sampleMask);
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
        Builder& addPushConstantRangeLayout(const vk::PushConstantRange& pushConstantRange);
        Builder& addDescriptorSetLayout(const vk::DescriptorSetLayout& descriptorSetLayout);

        // TODO: add setVertexInput 
        Builder& addVertexInputBindingDescription(const vk::VertexInputBindingDescription& vertexInputBindingDescription);
        Builder& addVertexInputAttributeDescription(const vk::VertexInputAttributeDescription& vertexInputAttributeDescription);

        // renderpass
        Builder& setRenderPass(const RenderPass& renderPass);


        GraphicsPipeline build(std::shared_ptr<Device> device);
        // GraphicsProgram buildComputeProgram(const Context *device);

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
        std::vector<vk::VertexInputBindingDescription> m_vertexInputBindingDescriptions;
        std::vector<vk::VertexInputAttributeDescription> m_vertexInputAttributeDescriptions;

        vk::RenderPass m_renderPass;
    };

    GraphicsPipeline() : m_device(nullptr), m_pipeline(VK_NULL_HANDLE), m_pipelineLayout(VK_NULL_HANDLE), m_shaderModules{} {}

    ~GraphicsPipeline();

    // TODO: add direct shader string compilation
    // void addShader(const std::string& shaderName, const std::string& shaderSource);

    void bind(const CommandBuffer& commandBuffer);

    vk::PipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

private:
    GraphicsPipeline(std::shared_ptr<Device> device, const vk::Pipeline& pipeline, const vk::PipelineLayout& pipelineLayout, const std::vector<vk::ShaderModule>& shaderModules);

private:
    std::shared_ptr<Device> m_device;
    vk::Pipeline m_pipeline;
    vk::PipelineLayout m_pipelineLayout;
    std::vector<vk::ShaderModule> m_shaderModules;
};

} // namespace gfx

#endif