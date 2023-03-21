#ifndef GFX_CONTEXT_HPP
#define GFX_CONTEXT_HPP

#include "../core/window.hpp"
#include "../core/log.hpp"

#include <vector>
#include <optional>

namespace gfx {

class Semaphore;
class CommandBuffer;
class Fence;
class SwapChain;

class Device {
public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilites;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };


    Device(core::Window& window, bool enableValidation);
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(const Device&&) = delete;
    Device& operator=(const Device&&) = delete;
    
    const core::Window& getWindow() const { return m_window; }
    SwapChainSupportDetails getSwapChainSupportDetails() const { return querySwapChainSupport(m_physicalDevice); }
    QueueFamilyIndices getQueueFamilyIndices() const { return findQueueFamilies(m_physicalDevice); }
    const vk::SurfaceKHR& getSurface() const { return m_surface; }
    const vk::Device& get() const { return m_device; }
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags memoryPropertyFlags) const;


    // might change in future, might relocate!
    struct QueueSubmitInfo {
        QueueSubmitInfo();

        QueueSubmitInfo& addWaitSemaphore(const Semaphore& semaphore);
        QueueSubmitInfo& addSignalSemaphore(const Semaphore& semaphore);
        QueueSubmitInfo& addCommandBuffer(const CommandBuffer& commandBuffer);
        QueueSubmitInfo& addWaitStage(vk::PipelineStageFlags flags);
        QueueSubmitInfo& setFence(const Fence& fence);

        std::vector<vk::Semaphore> m_waitSemaphores;
        std::vector<vk::Semaphore> m_signalSemaphores;
        std::vector<vk::CommandBuffer> m_commandBuffers;
        vk::PipelineStageFlags m_waitStages{};
        vk::Fence m_fence;
    };
    void submit(const QueueSubmitInfo& queueSubmitInfo);

    // bound to change
    struct PresentInfo {
        PresentInfo();

        PresentInfo& setImageIndex(uint32_t imageIndex);
        PresentInfo& addWaitSemaphore(const Semaphore& semaphore);
        PresentInfo& setSwapChain(const SwapChain& swapChain);

        std::vector<vk::Semaphore> m_waitSemaphores;
        uint32_t m_imageIndex;
        vk::SwapchainKHR m_swapChain;
    };
    vk::Result present(const PresentInfo& presentInfo);

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();

private:
    bool checkRequiredLayersSupport(const std::vector<const char *>& requiredLayers) noexcept;
    bool checkRequiredExtensionSupport(const vk::PhysicalDevice& physicalDevice) noexcept;
    void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCreateInfo);
    bool isDeviceSuitable(const vk::PhysicalDevice& physicalDevice);
    QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& physicalDevice) const;
    SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& physicalDevice) const;
    // vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    // vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    // vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites);

private:
    const core::Window& m_window;
    bool m_validations;

    std::vector<const char *> m_requiredLayers{};
    std::vector<const char *> m_requiredExtensions{};
    std::vector<const char *> m_requiredDeviceExtensions{};
    
    vk::Instance                           m_instance;
    vk::DispatchLoaderDynamic              m_dispatchLoaderDynamic;
    vk::DebugUtilsMessengerEXT             m_debugUtilsMessenger; 
    vk::SurfaceKHR                         m_surface;
    vk::PhysicalDevice                     m_physicalDevice;
    vk::Device                             m_device;
    vk::Queue                              m_graphicsQueue;
    vk::Queue                              m_presentQueue;
    vk::CommandPool                        m_commandPool;
    std::vector<vk::CommandBuffer>         m_commandBuffers;
};

} // namespace gfx

#endif