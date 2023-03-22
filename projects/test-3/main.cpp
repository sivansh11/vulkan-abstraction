#include "core/window.hpp"
#include "core/core.hpp"
#include "core/log.hpp"
#include "gfx/device.hpp"
#include "gfx/swapchain.hpp"
#include "gfx/pipeline.hpp"
#include "gfx/commandbuffer.hpp"
#include "renderer/renderer.hpp"
#include "gfx/buffer.hpp"

#include "glm/glm.hpp"

#include <memory>

// struct vec2 {
//     union {
//         struct {
//             float x, y;
//         };
//         float data[2];

//     };
// };

// struct vec3 {
//     union {
//         struct {
//             float x, y, z;
//         };
//         struct {
//             float r, g, b;
//         };
//         float data[3];
//     };
// };

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

    vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = vk::DescriptorSetLayoutBinding{}
        .setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex);
    
    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo{}
        .setBindingCount(1)
        .setPBindings(&descriptorSetLayoutBinding);

    vk::DescriptorSetLayout descriptorSetLayout;

    auto res = device->get().createDescriptorSetLayout(&descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);

    if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create uniform buffers!");
    }

    vk::DescriptorPoolSize descriptorPoolSize = vk::DescriptorPoolSize{}
        .setDescriptorCount(swapChain.MAX_FRAMES_IN_FLIGHT)
        .setType(vk::DescriptorType::eUniformBuffer);

    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo{}
        .setPoolSizeCount(1)
        .setPPoolSizes(&descriptorPoolSize)
        .setMaxSets(swapChain.MAX_FRAMES_IN_FLIGHT);
    
    vk::DescriptorPool descriptorPool;

    if (device->get().createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &descriptorPool) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(swapChain.MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo{}
        .setDescriptorPool(descriptorPool)
        .setDescriptorSetCount(swapChain.MAX_FRAMES_IN_FLIGHT)
        .setPSetLayouts(descriptorSetLayouts.data());

    std::vector<vk::DescriptorSet> descriptorSets;
    descriptorSets.resize(swapChain.MAX_FRAMES_IN_FLIGHT);
    if (device->get().allocateDescriptorSets(&descriptorSetAllocateInfo, descriptorSets.data()) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (int i = 0; i < swapChain.MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo descriptorBufferInfo = vk::DescriptorBufferInfo{}
            .setBuffer(uniformBuffers[i].get())
            .setOffset(0)
            .setRange(sizeof(UniformBufferObject));
        
        vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet{}
            .setDstSet(descriptorSets[i])
            .setDstBinding(0)
            .setDstArrayElement(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setPBufferInfo(&descriptorBufferInfo);
        
        device->get().updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
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
            commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.getPipelineLayout(), 0, 1, &descriptorSets[renderer.getCurrentFrameIndex()], 0, nullptr);

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