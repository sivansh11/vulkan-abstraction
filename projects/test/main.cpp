#include "core/window.hpp"
#include "core/core.hpp"
#include "core/log.hpp"
#include "gfx/context.hpp"
#include "gfx/swapchain.hpp"
#include "gfx/program.hpp"

#include "gfx/image.hpp"
#include "gfx/framebuffer.hpp"
#include "gfx/commandbuffer.hpp"
#include "gfx/syncobjects.hpp"

int main() {
    if (!core::Log::init()) {
        throw std::runtime_error("Failed to initialize logger!");
    }

    core::Window window{640, 420, "Test"};

    gfx::Context ctx{window, true};
    gfx::SwapChain swapChain{&ctx, {640, 420}};

    gfx::Image image = gfx::Image::Builder{}
        .setArrayLayers(1)
        .setExtent(core::Dimensions{250, 250, 1})
        .setFormat(vk::Format::eB8G8R8A8Srgb)
        .setMipLevels(1)
        .setType(vk::ImageType::e2D) 
        .setUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .build(&ctx);

    gfx::ImageView imageView = gfx::ImageView::Builder{}
        .setFormat(vk::Format::eB8G8R8A8Srgb)
        .setViewType(vk::ImageViewType::e2D)
        .setSubresourceRangeAspectMask(vk::ImageAspectFlagBits::eColor)
        .setImage(image)
        .build(&ctx);

    gfx::FrameBuffer frameBuffer = gfx::FrameBuffer::Builder{}
        .setDimensions({250, 250, 1})
        .setRenderPass(swapChain.getSwapChainRenderPass())
        .addAttachment(imageView)
        .build(&ctx);

    gfx::GraphicsProgram program = gfx::GraphicsProgram::Builder{}
        .addShaderFromPath("../../../assets/shader/test.vert")
        .addShaderFromPath("../../../assets/shader/test.frag")
        .addDynamicState(vk::DynamicState::eViewport)
        .addDynamicState(vk::DynamicState::eScissor)
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
        .addColorBlendAttachmentState(vk::PipelineColorBlendAttachmentState{}
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
            .setBlendEnable(vk::Bool32{ false }))
        .setRenderPass(swapChain.getSwapChainRenderPass())
        .build(&ctx);

    gfx::CommandPool commandPool = gfx::CommandPool::Builder{}
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(ctx.getQueueFamilyIndices().graphicsFamily.value())
        .build(&ctx);

    // gfx::Semaphore imageAvailableSemaphore = gfx::Semaphore::Builder{}.build(&ctx);
    // gfx::Semaphore renderFinishedSemaphore = gfx::Semaphore::Builder{}.build(&ctx);
    // gfx::Fence inFlightFence = gfx::Fence::Builder{}
    //     .setFlags(vk::FenceCreateFlagBits::eSignaled)
    //     .build(&ctx);

    auto commandBuffers = commandPool.createCommandBuffer(1);
    auto& commandBuffer = commandBuffers[0];

    auto& renderPass = swapChain.getSwapChainRenderPass();

    auto recordCommandBuffer = [&](uint32_t imageIndex) {
        commandBuffer.begin();
        renderPass.begin(commandBuffer, gfx::RenderPass::BeginInfo{}
            .setFrameBuffer(swapChain.getSwapChainFrameBuffers()[imageIndex])
            .setRenderArea(vk::Rect2D{}.setOffset({0, 0}).setExtent(swapChain.getSwapchainExtent()))            
            .addClearValue(vk::ClearValue{}.setColor(vk::ClearColorValue{std::array{0.f, 0.f, 0.f, 1.f}})));

        program.bind(commandBuffer);

        vk::Viewport viewPort;
        viewPort.x = viewPort.y = 0;
        viewPort.width = swapChain.getSwapchainExtent().width;
        viewPort.height = swapChain.getSwapchainExtent().height;
        viewPort.minDepth = 0;
        viewPort.maxDepth = 1;
        commandBuffer.getCommandBuffer().setViewport(0, { viewPort });
        
        vk::Rect2D scissor;
        scissor.offset = vk::Offset2D{0, 0};
        scissor.extent = swapChain.getSwapchainExtent();
        commandBuffer.getCommandBuffer().setScissor(0, { scissor });

        commandBuffer.getCommandBuffer().draw(3, 1, 0, 0);

        renderPass.end(commandBuffer);

        commandBuffer.end();
    };

    auto drawFrame = [&]() {
        uint32_t imageIndex = swapChain.acquireNextImage();
        recordCommandBuffer(imageIndex);
        swapChain.getInFlightFence().wait();
        swapChain.getInFlightFence().reset();
        ctx.submit(gfx::Context::QueueSubmitInfo{}
            .addCommandBuffer(commandBuffer)
            .addWaitSemaphore(swapChain.getImageAvailableSemaphore())
            .addSignalSemaphore(swapChain.getRenderFinishedSemaphore())
            .addWaitStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setFence(swapChain.getInFlightFence())
        );

        ctx.present(gfx::Context::PresentInfo{}
            .addWaitSemaphore(swapChain.getRenderFinishedSemaphore())
            .setImageIndex(imageIndex)
            .setSwapChain(swapChain)
        );
    };

    while (!window.shouldClose()) {
        core::Window::pollEvents();

        // auto [width, height] = window.getDimensions();
        drawFrame();
    }

    ctx.getDevice().waitIdle();

    return 0;
}