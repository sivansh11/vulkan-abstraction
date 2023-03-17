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

int main() {
    if (!core::Log::init()) {
        throw std::runtime_error("Failed to initialize logger!");
    }

    core::Window window{640, 420, "Test"};

    std::shared_ptr<gfx::Device> device = std::make_shared<gfx::Device>(window, true);  
    gfx::SwapChain swapChain{device, 3};
    gfx::Renderer renderer = gfx::Renderer::Builder{}.build(device, swapChain);

    gfx::GraphicsProgram program = gfx::GraphicsProgram::Builder{}
        .addShaderFromPath("../../../assets/shader/test.vert")
        .addShaderFromPath("../../../assets/shader/test.frag")
        .addDynamicState(vk::DynamicState::eViewport)
        .addDynamicState(vk::DynamicState::eScissor)
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

    gfx::Buffer buffer = gfx::Buffer::Builder{}
        .setMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setSize(25)
        .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
        .build(device);

    while (!window.shouldClose()) {
        core::Window::pollEvents();

        if (auto res = renderer.begin()) {
            auto [commandBuffer, imageIndex] = res.value();

            renderer.beginSwapChainRenderPass();

            program.bind(commandBuffer);

            vk::Viewport viewPort;
            viewPort.x = viewPort.y = 0;
            viewPort.width = swapChain.getSwapchainExtent().width;
            viewPort.height = swapChain.getSwapchainExtent().height;
            viewPort.minDepth = 0;
            viewPort.maxDepth = 1;
            commandBuffer.getCommandBuffer().setViewport(0, 1, &viewPort);
            
            vk::Rect2D scissor;
            scissor.offset = vk::Offset2D{0, 0};
            scissor.extent = swapChain.getSwapchainExtent();
            commandBuffer.getCommandBuffer().setScissor(0, 1, &scissor);

            commandBuffer.getCommandBuffer().draw(3, 1, 0, 0);

            renderer.endSwapChainRenderPass();

            renderer.end();
        }
    }

device->getDevice().waitIdle();

    return 0;
}