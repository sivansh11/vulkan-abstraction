#include "pipeline.hpp"

#include "../core/log.hpp"

// #include <shaderc/libshaderc_util/include/libshaderc_util/file_finder.h>
// #include <shaderc/glslc/src/file_includer.h>
#include <shaderc/shaderc.hpp>

#include <fstream>

namespace gfx {

namespace utils {

static std::string readFile(const std::filesystem::path& path) {
    std::ifstream file{path};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to read file: " + path.string());
    }
    INFO("Read file {}", path.string());

    return std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
}

} // namespace utils

// **********************GraphicsPipeline::Builder*****************************
GraphicsPipeline::Builder::Builder() {}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::addShaderFromPath(const std::filesystem::path& shaderPath) {
    m_shaderPaths.emplace(shaderPath);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::addDynamicState(vk::DynamicState dynamicState) {
    m_dynamicStates.push_back(dynamicState);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setInputAssemblyTopology(vk::PrimitiveTopology topology) {
    m_pipelineInputAssemblyStateCreateInfo.setTopology(topology);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setInputAssemblyPrimitiveRestartEnable(bool enable) {
    m_pipelineInputAssemblyStateCreateInfo.setPrimitiveRestartEnable(enable);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::addViewport(const vk::Viewport& viewPort) {
    m_viewports.push_back(viewPort);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::addScissor(const vk::Rect2D& scissor) {
    m_scissors.push_back(scissor);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerDepthClampEnable(bool enable) {
    m_pipelineRasterizationStateCreateInfo.setDepthClampEnable(vk::Bool32{ enable });
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerDiscardEnable(bool enable) {
    m_pipelineRasterizationStateCreateInfo.setRasterizerDiscardEnable(vk::Bool32{ enable });
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerPolygonMode(vk::PolygonMode polygonMode) {
    m_pipelineRasterizationStateCreateInfo.setPolygonMode(polygonMode);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerLineWidth(float lineWidth) {
    m_pipelineRasterizationStateCreateInfo.setLineWidth(lineWidth);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerCullMode(vk::CullModeFlagBits cullMode) {
    m_pipelineRasterizationStateCreateInfo.setCullMode(cullMode);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerFrontFace(vk::FrontFace frontFace) {
    m_pipelineRasterizationStateCreateInfo.setFrontFace(frontFace);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerDepthBiasEnable(bool enable) {
    m_pipelineRasterizationStateCreateInfo.setDepthBiasEnable(vk::Bool32{ enable });
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerDepthBiasConstantFactor(float factor) {
    assert(m_pipelineRasterizationStateCreateInfo.depthBiasEnable && "Call setRasterizerDepthBiasEnable with true");
    m_pipelineRasterizationStateCreateInfo.setDepthBiasConstantFactor(factor);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerDepthBiasClamp(float factor) {
    assert(m_pipelineRasterizationStateCreateInfo.depthBiasEnable && "Call setRasterizerDepthBiasEnable with true");
    m_pipelineRasterizationStateCreateInfo.setDepthBiasClamp(factor);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRasterizerDepthBiasSlopeFactor(float factor) {
    assert(m_pipelineRasterizationStateCreateInfo.depthBiasEnable && "Call setRasterizerDepthBiasEnable with true");
    m_pipelineRasterizationStateCreateInfo.setDepthBiasSlopeFactor(factor);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setMultisamplingSampleShadingEnable(bool enable) {
    m_pipelineMultisampleStateCreateInfo.setSampleShadingEnable(vk::Bool32{ enable });
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setMultisamplinRasterizationSamples(vk::SampleCountFlagBits sampleCount) {
    m_pipelineMultisampleStateCreateInfo.setRasterizationSamples(sampleCount);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setMultisamplinMinSampleShading(float factor) {
    m_pipelineMultisampleStateCreateInfo.setMinSampleShading(factor);
    return *this;
}

// GraphicsPipeline::Builder& GraphicsPipeline::Builder::setMultisamplingSampleMask(vk::SampleMask sampleMask) {
//     m_sampleMask = sampleMask;
//     return *this;
// }

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setMultisamplingAlphaToCoverageEnable(bool enable) {
    m_pipelineMultisampleStateCreateInfo.setAlphaToCoverageEnable(vk::Bool32{ enable });
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setMultisamplingAlphaToOneEnable(bool enable) {
    m_pipelineMultisampleStateCreateInfo.setAlphaToOneEnable(vk::Bool32{ enable });
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::addColorBlendAttachmentState(const vk::PipelineColorBlendAttachmentState& pipelineColorBlendAttachmentState) {
    m_pipelineColorBlendAttachmentStates.push_back(pipelineColorBlendAttachmentState);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setColorBlendStateLogicOpEnable(bool enable) {
    m_pipelineColorBlendStateCreateInfo.setLogicOpEnable(vk::Bool32{ enable });
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setColorBlendStateLogicOp(vk::LogicOp logicOp) {
    m_pipelineColorBlendStateCreateInfo.setLogicOp(logicOp);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setColorBlendStateBlendConstantR(float factor) {
    m_pipelineColorBlendStateCreateInfo.blendConstants[0] = factor;
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setColorBlendStateBlendConstantG(float factor) {
    m_pipelineColorBlendStateCreateInfo.blendConstants[1] = factor;
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setColorBlendStateBlendConstantB(float factor) {
    m_pipelineColorBlendStateCreateInfo.blendConstants[2] = factor;
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setColorBlendStateBlendConstantA(float factor) {
    m_pipelineColorBlendStateCreateInfo.blendConstants[3] = factor;
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::addPushConstantRangeLayout(const vk::PushConstantRange& pushConstantRange) {
    m_pushConstantRanges.push_back(pushConstantRange);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::addDescriptorSetLayout(const DescriptorSetLayout& descriptorSetLayout) {
    m_descriptorSetLayout.push_back(descriptorSetLayout.get());
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::addVertexInputBindingDescription(const vk::VertexInputBindingDescription& vertexInputBindingDescription) {
    m_vertexInputBindingDescriptions.push_back(vertexInputBindingDescription);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::addVertexInputAttributeDescription(const vk::VertexInputAttributeDescription& vertexInputAttributeDescription) {
    m_vertexInputAttributeDescriptions.push_back(vertexInputAttributeDescription);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::setRenderPass(const RenderPass& renderPass) {
    m_renderPass = renderPass.get();
    return *this;
}

GraphicsPipeline GraphicsPipeline::Builder::build(std::shared_ptr<Device> device) {
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.setPushConstantRangeCount(m_pushConstantRanges.size())
                            .setPPushConstantRanges(m_pushConstantRanges.data())
                            .setSetLayoutCount(m_descriptorSetLayout.size())
                            .setPSetLayouts(m_descriptorSetLayout.data());

    vk::PipelineLayout pipelineLayout;
    if (device->get().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    // static shaderc_util::FileFinder fileFinder;
    static shaderc::Compiler compiler;
    static shaderc::CompileOptions options;
    // static bool once = []() { options.SetIncluder(std::make_unique<glslc::FileIncluder>(&fileFinder));  return true; }();

    std::vector<vk::ShaderModule> shaderModules;
    std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;

    shaderModules.reserve(m_shaderPaths.size());
    pipelineShaderStageCreateInfos.reserve(m_shaderPaths.size());

    for (auto& shaderPath : m_shaderPaths) {

        const auto source = utils::readFile(shaderPath);
        const auto name = shaderPath.filename().string().c_str();

        // TODO: potentially add options for setting optimization levels
        options.SetOptimizationLevel(shaderc_optimization_level_zero);

        shaderc_shader_kind kind;
        vk::ShaderStageFlagBits shaderStage;
        if (shaderPath.extension().string() == ".vert") {
            kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
            shaderStage = vk::ShaderStageFlagBits::eVertex;
        }
        if (shaderPath.extension().string() == ".frag") {
            kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
            shaderStage = vk::ShaderStageFlagBits::eFragment;
        }
        if (shaderPath.extension().string() == ".geom") {
            kind = shaderc_shader_kind::shaderc_glsl_geometry_shader;
            shaderStage = vk::ShaderStageFlagBits::eGeometry;
        }
        if (shaderPath.extension().string() == ".comp") {
            kind = shaderc_shader_kind::shaderc_glsl_compute_shader;
            shaderStage = vk::ShaderStageFlagBits::eCompute;
        }

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, kind, name, options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error("Failed to compile shader: " + shaderPath.string() + "\n\t" + module.GetErrorMessage());
        }

        INFO("Successfully compiled shader {}", shaderPath.filename().string());

        std::vector<uint32_t> code{ module.cbegin(), module.cend() };

        // std::string codeToBeSaved { reinterpret_cast<const char*>(code.data()), code.size() * sizeof(uint32_t) };
        // std::fstream file{"../../../assets/shader/" + shaderPath.filename().string() + ".spv", std::fstream::out | std::fstream::trunc | std::fstream::binary };
        // file.write(codeToBeSaved.c_str(), codeToBeSaved.size());
        // file.close();

        vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo{}
            .setCodeSize(code.size() * sizeof(uint32_t))
            .setPCode(code.data());

        vk::ShaderModule shaderModule;

        if (device->get().createShaderModule(&shaderModuleCreateInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create shader module!");
        }
        shaderModules.push_back(shaderModule);

        vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo{}
            .setModule(shaderModules.back())
            .setPName("main")
            .setStage(shaderStage);
        
        pipelineShaderStageCreateInfos.push_back(pipelineShaderStageCreateInfo);
    }

    // TODO: Do this correctly
    // hack
    vk::PipelineVertexInputStateCreateInfo vertexInput = vk::PipelineVertexInputStateCreateInfo{}
        .setVertexBindingDescriptionCount(m_vertexInputBindingDescriptions.size())
        .setPVertexBindingDescriptions(m_vertexInputBindingDescriptions.data())
        .setVertexAttributeDescriptionCount(m_vertexInputAttributeDescriptions.size())
        .setPVertexAttributeDescriptions(m_vertexInputAttributeDescriptions.data());

    vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo{}
        .setViewportCount(m_viewports.size())
        .setPViewports(m_viewports.data())
        .setScissorCount(m_scissors.size())
        .setPScissors(m_scissors.data());

    m_pipelineColorBlendStateCreateInfo.setAttachmentCount(m_pipelineColorBlendAttachmentStates.size())
                                       .setPAttachments(m_pipelineColorBlendAttachmentStates.data());

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo{}
        .setDynamicStateCount(m_dynamicStates.size())
        .setPDynamicStates(m_dynamicStates.data());

    m_pipelineMultisampleStateCreateInfo.setPSampleMask(nullptr);

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo{}
        .setStageCount(pipelineShaderStageCreateInfos.size())
        .setPStages(pipelineShaderStageCreateInfos.data())
        .setPVertexInputState(&vertexInput)
        .setPInputAssemblyState(&m_pipelineInputAssemblyStateCreateInfo)
        .setPViewportState(&viewportState)
        .setPRasterizationState(&m_pipelineRasterizationStateCreateInfo)
        .setPMultisampleState(&m_pipelineMultisampleStateCreateInfo)
        .setPDepthStencilState(nullptr)
        .setPColorBlendState(&m_pipelineColorBlendStateCreateInfo)
        .setPDynamicState(&dynamicStateCreateInfo)
        .setLayout(pipelineLayout)
        .setRenderPass(m_renderPass)
        .setSubpass(0)
        .setBasePipelineHandle(vk::Pipeline{ VK_NULL_HANDLE })
        .setBasePipelineIndex(-1);

    vk::Pipeline graphicsPipline;

    auto graphicsPipelineResult = device->get().createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineCreateInfo);

    if (graphicsPipelineResult.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    graphicsPipline = graphicsPipelineResult.value;

    INFO("Created a Graphics Pipeline!");

    return {device, graphicsPipline, pipelineLayout, shaderModules};
}

// **********************GRAPHICSPROGRAM*********************
GraphicsPipeline::GraphicsPipeline(std::shared_ptr<Device> device, const vk::Pipeline& pipeline, const vk::PipelineLayout& pipelineLayout, const std::vector<vk::ShaderModule>& shaderModules) : m_device(device), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout), m_shaderModules(shaderModules) {

}

GraphicsPipeline::~GraphicsPipeline() {
    for (auto& shaderModule : m_shaderModules) {
        m_device->get().destroyShaderModule(shaderModule);
    }
    m_device->get().destroyPipelineLayout(m_pipelineLayout);
    m_device->get().destroyPipeline(m_pipeline);
}

void GraphicsPipeline::bind(const CommandBuffer& commandBuffer) {
    commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
}

} // namespace gfx
