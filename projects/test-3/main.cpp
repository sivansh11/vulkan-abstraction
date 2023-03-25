#include "core/window.hpp"
#include "core/core.hpp"
#include "core/log.hpp"
#include "gfx/device.hpp"
#include "gfx/swapchain.hpp"
#include "gfx/pipeline.hpp"
#include "gfx/commandbuffer.hpp"
#include "gfx/buffer.hpp"
#include "gfx/descriptors.hpp"

#include "renderer/renderer.hpp"

#include "glm/glm.hpp"

#include <memory>

int main(int argc, char **argv) {
    if (!core::Log::init()) {
        throw std::runtime_error("Failed to initialize logger!");
    }

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            INFO("{}", argv[i]);
        }
    }

    core::Window window{640, 420, "Test"};

    std::shared_ptr<gfx::Device> device = std::make_shared<gfx::Device>(window, true);  
    gfx::SwapChain swapChain{device, 3};
    renderer::Renderer renderer = renderer::Renderer::Builder{}.build(device, swapChain);
    
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;

        static std::vector<vk::VertexInputBindingDescription> getBindingDescription() {
            std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescriptions{};
            vertexInputBindingDescriptions.resize(1);

            vertexInputBindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;
            vertexInputBindingDescriptions[0].binding = 0;
            vertexInputBindingDescriptions[0].stride = sizeof(Vertex);

            return vertexInputBindingDescriptions;
        }

        static std::vector<vk::VertexInputAttributeDescription> getAttributeDescription() {
            std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions{};
            vertexInputAttributeDescriptions.resize(2);

            vertexInputAttributeDescriptions[0].binding = 0;
            vertexInputAttributeDescriptions[0].location = 0;
            vertexInputAttributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
            vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, pos);

            vertexInputAttributeDescriptions[1].binding = 0;
            vertexInputAttributeDescriptions[1].location = 1;
            vertexInputAttributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
            vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, color);

            return vertexInputAttributeDescriptions;
        }
    };

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    struct UniformBufferObject {
        glm::vec3 color;
    };

    gfx::Buffer vertexBuffer = gfx::Buffer::Builder{}
        .setMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setSize(sizeof(Vertex) * vertices.size())
        .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
        .build(device);

    gfx::Buffer indexBuffer = gfx::Buffer::Builder{}
        .setMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setSize(sizeof(uint32_t) * indices.size())
        .setUsage(vk::BufferUsageFlagBits::eIndexBuffer)
        .build(device);

    std::vector<gfx::Buffer> uniformBuffers;
    uniformBuffers.reserve(swapChain.MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < swapChain.MAX_FRAMES_IN_FLIGHT; i++) {
        uniformBuffers.emplace_back(gfx::Buffer::Builder{}
            .setMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setSize(sizeof(UniformBufferObject))
            .setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
            .build(device));
    }

    vertexBuffer.map();
    std::memcpy(vertexBuffer.getMapped(), vertices.data(), sizeof(Vertex) * vertices.size());
    vertexBuffer.unmap();

    indexBuffer.map();
    std::memcpy(indexBuffer.getMapped(), indices.data(), sizeof(uint32_t) * indices.size());
    indexBuffer.unmap();

    
    gfx::DescriptorSetLayout descriptorSetLayout = gfx::DescriptorSetLayout::Builder{}
        .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
        .build(device);

    gfx::DescriptorPool descriptorPool = gfx::DescriptorPool::Builder{}
        .addPoolSize(vk::DescriptorType::eUniformBuffer, swapChain.MAX_FRAMES_IN_FLIGHT)
        .setMaxSets(swapChain.MAX_FRAMES_IN_FLIGHT)
        // .setMaxSets(1000) // might be worth to default this to 1000
        .build(device);

    auto descriptors = descriptorPool.allocate(gfx::DescriptorPool::SetAllocateInfo{}
        .addLayout(descriptorSetLayout)
        .addLayout(descriptorSetLayout)
        .addLayout(descriptorSetLayout));

    for (int i = 0; i < descriptors.size(); i++) {
        auto& descriptor = descriptors[i];
        descriptor.update(gfx::DescriptorSet::Update{}
            .addBuffer(0, uniformBuffers[i].getDescriptorBufferInfo()));
    }

    gfx::GraphicsPipeline pipeline = gfx::GraphicsPipeline::Builder{}
        .addShaderFromPath("../../../assets/shader/test-3.vert")
        .addShaderFromPath("../../../assets/shader/test.frag")
        .addDescriptorSetLayout(descriptorSetLayout)
        .addVertexInputBindingDescription(Vertex::getBindingDescription()[0])
        .addVertexInputAttributeDescription(Vertex::getAttributeDescription()[0])
        .addVertexInputAttributeDescription(Vertex::getAttributeDescription()[1])
        .setInputAssemblyTopology(vk::PrimitiveTopology::eTriangleList)
        .setInputAssemblyPrimitiveRestartEnable(false)
        .addViewport(vk::Viewport{}
            .setX(0.f)
            .setY(0.f)
            .setWidth(swapChain.getExtent().width)
            .setHeight(swapChain.getExtent().height)
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f))
        .addScissor(vk::Rect2D{}
            .setOffset(vk::Offset2D{ 0, 0 })
            .setExtent(swapChain.getExtent()))
        .setRasterizerDepthClampEnable(false)
        .setRasterizerDiscardEnable(false)
        .setRasterizerPolygonMode(vk::PolygonMode::eFill)
        .setRasterizerLineWidth(1)
        .setRasterizerCullMode(vk::CullModeFlagBits::eNone)
        .setRasterizerDepthBiasEnable(false)
        .setMultisamplingSampleShadingEnable(false)
        .setMultisamplinRasterizationSamples(vk::SampleCountFlagBits::e1)
        .addColorBlendAttachmentState(vk::PipelineColorBlendAttachmentState{}
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
            .setBlendEnable(vk::Bool32{ false }))
        .setColorBlendStateLogicOpEnable(false)
        .setRenderPass(renderer.getRenderPass())
        .build(device);

    auto update = [&]() {
        UniformBufferObject ubo;
        static float v = 0;
        v += 1.0f / 255.0f;
        if (v > 1) {
            v = 0;
        }
        ubo.color = {v, v, v};
        uniformBuffers[renderer.getCurrentFrameIndex()].map();
        std::memcpy(uniformBuffers[renderer.getCurrentFrameIndex()].getMapped(), &ubo, sizeof(ubo));
        uniformBuffers[renderer.getCurrentFrameIndex()].unmap();
    };

    while (!window.shouldClose()) {
        core::Window::pollEvents();

        if (auto res = renderer.begin()) {
            auto [commandBuffer, imageIndex] = res.value();

            renderer.beginSwapChainRenderPass();

            update();

            pipeline.bind(commandBuffer);
            commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.getPipelineLayout(), 0, 1, &descriptors[renderer.getCurrentFrameIndex()].get(), 0, nullptr);

            commandBuffer.get().bindVertexBuffers(0, {vertexBuffer.get()}, {0});
            commandBuffer.get().bindIndexBuffer(indexBuffer.get(), {0}, vk::IndexType::eUint32);
            commandBuffer.get().drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

            renderer.endSwapChainRenderPass();

            renderer.end();
        }

    }
    
    device->get().waitIdle();

    return 0;
}