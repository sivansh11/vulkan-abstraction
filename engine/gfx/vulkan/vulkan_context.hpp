#ifndef GFX_VULKAN_CONTEXT_HPP
#define GFX_VULKAN_CONTEXT_HPP

#include "vulkan_window.hpp"
#include "../context.hpp"

#include <vector>
#include <optional>

namespace gfx {

namespace vulkan {

class VulkanContext : public Context {
public:
    VulkanContext(core::Window& window, bool enableValidation);
    ~VulkanContext() override;

    static constexpr Api api = Api::Vulkan;

private:

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
    QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& physicalDevice);
    SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& physicalDevice);
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites);

private:
    const core::Window& m_window;
    bool m_validations;

    std::vector<const char *> m_requiredLayers{};
    std::vector<const char *> m_requiredExtensions{};
    std::vector<const char *> m_requiredDeviceExtensions{};
    
    vk::Instance               m_instance;
    vk::DispatchLoaderDynamic  m_dispatchLoaderDynamic;
    vk::DebugUtilsMessengerEXT m_debugUtilsMessenger;  
    vk::SurfaceKHR             m_surface;
    vk::PhysicalDevice         m_physicalDevice;
    vk::Device                 m_device;
    vk::Queue                  m_graphicsQueue;
    vk::Queue                  m_presentQueue;

};

} // namespace vulkan

} // namespace gfx

#endif