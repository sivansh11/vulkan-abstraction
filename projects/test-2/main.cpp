#include "core/window.hpp"
#include "core/core.hpp"
#include "core/log.hpp"
#include "gfx/device.hpp"
#include "gfx/swapchain.hpp"
#include "gfx/program.hpp"
#include "gfx/commandbuffer.hpp"
#include "gfx/renderer.hpp"
#include "gfx/buffer.hpp"

#include <memory>

struct vec2 {
    union {
        struct {
            float x, y;
        };
        float data[2];

    };
};

struct vec3 {
    union {
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };
        float data[3];
    };
};

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
    gfx::Renderer renderer = gfx::Renderer::Builder{}.build(device, swapChain);
    
    struct Vertex {
        vec2 pos;
        vec3 color;

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
        {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    gfx::Buffer buffer = gfx::Buffer::Builder{}
        .setMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setSize(sizeof(Vertex) * vertices.size())
        .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
        .build(device);

    buffer.map();
    std::memcpy(buffer.getMapped(), vertices.data(), sizeof(Vertex) * vertices.size());
    buffer.unmap();

    gfx::GraphicsProgram program = gfx::GraphicsProgram::Builder{}
        .addShaderFromPath("../../../assets/shader/test-2.vert")
        .addShaderFromPath("../../../assets/shader/test.frag")
        .addVertexInputBindingDescription(Vertex::getBindingDescription()[0])
        .addVertexInputAttributeDescription(Vertex::getAttributeDescription()[0])
        .addVertexInputAttributeDescription(Vertex::getAttributeDescription()[1])
        .setInputAssemblyTopology(vk::PrimitiveTopology::eTriangleList)
        .setInputAssemblyPrimitiveRestartEnable(false)
        .addViewport(vk::Viewport{}
            .setX(0.f)
            .setY(0.f)
            .setWidth(swapChain.getSwapchainExtent().width)
            .setHeight(swapChain.getSwapchainExtent().height)
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f))
        .addScissor(vk::Rect2D{}
            .setOffset(vk::Offset2D{ 0, 0 })
            .setExtent(swapChain.getSwapchainExtent()))
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
        .setRenderPass(swapChain.getSwapChainRenderPass())
        .build(device);

    while (!window.shouldClose()) {
        core::Window::pollEvents();

        if (auto res = renderer.begin()) {
            auto [commandBuffer, imageIndex] = res.value();

            renderer.beginSwapChainRenderPass();

            program.bind(commandBuffer);

            commandBuffer.getCommandBuffer().bindVertexBuffers(0, {buffer.getBuffer()}, {0});
            commandBuffer.getCommandBuffer().draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);

            renderer.endSwapChainRenderPass();

            renderer.end();
        }

    }

    device->getDevice().waitIdle();

    return 0;
}