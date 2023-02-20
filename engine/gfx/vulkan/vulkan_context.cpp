#include "vulkan_context.hpp"

#include "../../core/log.hpp"

#include <iostream>
#include <cassert>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>

namespace gfx {

namespace vulkan {

VulkanContext::VulkanContext(core::Window& window, bool enableValidation) : Context(VulkanContext::api), m_window(window), m_validations(enableValidation) {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
}

VulkanContext::~VulkanContext() {
    m_device.destroy();
    m_instance.destroySurfaceKHR(m_surface);
    if (m_validations) {
        m_instance.destroyDebugUtilsMessengerEXT(m_debugUtilsMessenger, nullptr, m_dispatchLoaderDynamic);
    }
    m_instance.destroy();
}

bool VulkanContext::checkRequiredLayersSupport(const std::vector<const char *>& m_requiredLayers) noexcept {
    auto  availableLayers = vk::enumerateInstanceLayerProperties();
    for (auto layerName : m_requiredLayers) {
        bool layerFound = false;
        for (auto &layerProperty : availableLayers) {
            if (std::strcmp(layerName, layerProperty.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
    
    switch (vk::DebugUtilsMessageSeverityFlagBitsEXT{ messageSeverity }) {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            ERROR("Validation Layer: {}", pCallbackData->pMessage);
            break;
        
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            INFO("Validation Layer: {}", pCallbackData->pMessage);
            break;
        
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            WARN("Validation Layer: {}", pCallbackData->pMessage);
            break;
        
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            TRACE("Validation Layer: {}", pCallbackData->pMessage);
            break;
    }

    return VK_FALSE;

}

void VulkanContext::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCreateInfo) {
    debugUtilsMessengerCreateInfo.setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError   |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning 
    );

    debugUtilsMessengerCreateInfo.setMessageType(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral     |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation  |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
    );

    debugUtilsMessengerCreateInfo.setPfnUserCallback(debugCallback);

    debugUtilsMessengerCreateInfo.setPNext(nullptr);
}

void VulkanContext::createInstance() {
    auto windowExtensions = reinterpret_cast<const VulkanWindow *>(m_window.getInternal())->getRequiredExtensions();

    m_requiredExtensions.insert(m_requiredExtensions.end(), windowExtensions.begin(), windowExtensions.end());
    
    if (m_validations) {
        m_requiredLayers.emplace_back("VK_LAYER_KHRONOS_validation");
        m_requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!checkRequiredLayersSupport(m_requiredLayers)) {
        throw std::runtime_error("Vulkan: Required Extensions not available!");
    }

    vk::ApplicationInfo applicationInfo = vk::ApplicationInfo{}
        .setPApplicationName(m_window.getTitle().c_str())
        .setPEngineName("Horizon")
        .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
        .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion(VK_API_VERSION_1_3);

    vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo{}
        .setPApplicationInfo(&applicationInfo)
        .setEnabledExtensionCount(static_cast<uint32_t>(m_requiredExtensions.size()))
        .setPpEnabledExtensionNames(m_requiredExtensions.data());

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
    if (m_validations) {
        populateDebugMessengerCreateInfo(debugUtilsMessengerCreateInfo);

        instanceCreateInfo.setEnabledLayerCount(static_cast<uint32_t>(m_requiredLayers.size()))
                          .setPpEnabledLayerNames(m_requiredLayers.data())
                          .setPNext(&debugUtilsMessengerCreateInfo);
    }

    if (vk::createInstance(&instanceCreateInfo, nullptr, &m_instance) != vk::Result::eSuccess) {
        throw std::runtime_error("Vulkan: Failed to create instance!");
    }

    m_dispatchLoaderDynamic = {reinterpret_cast<PFN_vkGetInstanceProcAddr>(m_instance.getProcAddr("vkGetInstanceProcAddr"))};
    m_dispatchLoaderDynamic.init(m_instance);

    INFO("Vulkan: Created Instance!");
}

vk::SurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    if (availableFormats.size() > 0) return availableFormats[0];

    throw std::runtime_error("Vulkan: No Available Surface formats!");
}

vk::PresentModeKHR VulkanContext::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanContext::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites) {
    if (capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilites.currentExtent;
    } else {
        auto [width, height] = reinterpret_cast<const VulkanWindow *>(m_window.getInternal())->getExtent();

        VkExtent2D actualExtent{ width, height };
        actualExtent.width = std::clamp(actualExtent.width, capabilites.minImageExtent.width, capabilites.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilites.minImageExtent.height, capabilites.maxImageExtent.height);
        
        return actualExtent;
    }
}   

void VulkanContext::createSurface() {
    VkSurfaceKHR tempSurface{};
    if (reinterpret_cast<const VulkanWindow *>(m_window.getInternal())->createSurface(m_instance, nullptr, &tempSurface) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Failed to get window surface!");
    }
    m_surface = vk::SurfaceKHR{ tempSurface };

    INFO("Vulkan: Created Surface!");
}

void VulkanContext::setupDebugMessenger() {
    if (!m_validations) return;
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
    populateDebugMessengerCreateInfo(debugUtilsMessengerCreateInfo);
    if (m_instance.createDebugUtilsMessengerEXT(&debugUtilsMessengerCreateInfo, nullptr, &m_debugUtilsMessenger, m_dispatchLoaderDynamic) != vk::Result::eSuccess) {
        throw std::runtime_error("Vulkan: Failed to create debug messenger!");
    }

    INFO("Vulkan: Setup Debug Messenger!");
}

VulkanContext::QueueFamilyIndices VulkanContext::findQueueFamilies(const vk::PhysicalDevice& physicalDevice) {
    QueueFamilyIndices indices;

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }
        if (physicalDevice.getSurfaceSupportKHR(i, m_surface)) {
            indices.presentFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

VulkanContext::SwapChainSupportDetails VulkanContext::querySwapChainSupport(const vk::PhysicalDevice& physicalDevice) {
    INFO("");
    INFO("");
    INFO("Current Device: {}", physicalDevice.getProperties().deviceName);
    SwapChainSupportDetails details;

    details.formats = physicalDevice.getSurfaceFormatsKHR(m_surface);

    details.capabilites = physicalDevice.getSurfaceCapabilitiesKHR(m_surface);

    details.presentModes = physicalDevice.getSurfacePresentModesKHR(m_surface);

    return details;
}

bool VulkanContext::checkRequiredExtensionSupport(const vk::PhysicalDevice& physicalDevice) noexcept {
    auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(m_requiredDeviceExtensions.begin(), m_requiredDeviceExtensions.end());

    for (auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool VulkanContext::isDeviceSuitable(const vk::PhysicalDevice& physicalDevice) {
    auto deviceProperties = physicalDevice.getProperties();
    auto deviceFeatures = physicalDevice.getFeatures();

    auto indices = findQueueFamilies(physicalDevice);

    bool swapChainAdequate = false;

    if (checkRequiredExtensionSupport(physicalDevice)) {
        auto swapChainSupportDetails = querySwapChainSupport(physicalDevice);
        swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
    }

    return deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && indices.isComplete() && swapChainAdequate;
}

void VulkanContext::pickPhysicalDevice() {
    m_requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    auto physicalDevices = m_instance.enumeratePhysicalDevices();
    if (physicalDevices.size() == 0) {
        throw std::runtime_error("Vulkan: Failed to find GPU with vulkan support!");
    }

    INFO("Vulkan: Physical Devices:");
    for (auto& device : physicalDevices) {
        auto deviceProperties = device.getProperties();
        INFO("Vulkan: Device Name: {} Type: {}", deviceProperties.deviceName, vk::to_string(deviceProperties.deviceType));
    }

    for (const auto& device : physicalDevices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }

    if (!m_physicalDevice) {
        throw std::runtime_error("Vulkan: Failed to find a suitable GPU!");
    }

    auto pickedPhysicalDeviceProperties = m_physicalDevice.getProperties();
    INFO("Picked Physical device {} of type {}", pickedPhysicalDeviceProperties.deviceName, vk::to_string(pickedPhysicalDeviceProperties.deviceType));
}

void VulkanContext::createLogicalDevice() {
    auto queueFamilyIndices = findQueueFamilies(m_physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo = vk::DeviceQueueCreateInfo{}
            .setQueueFamilyIndex(queueFamily)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);        
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};    
    vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo{}
        .setPQueueCreateInfos(deviceQueueCreateInfos.data())
        .setQueueCreateInfoCount(static_cast<uint32_t>(deviceQueueCreateInfos.size()))
        .setPEnabledFeatures(&deviceFeatures)
        .setPpEnabledExtensionNames(m_requiredDeviceExtensions.data())
        .setEnabledExtensionCount(m_requiredDeviceExtensions.size());

    if (m_validations) {
        deviceCreateInfo.setEnabledLayerCount(m_requiredLayers.size())
                        .setPpEnabledLayerNames(m_requiredLayers.data());
    } 

    if (m_physicalDevice.createDevice(&deviceCreateInfo, nullptr, &m_device) != vk::Result::eSuccess) {
        throw std::runtime_error("Vulkan: Failed to create logical device!");
    }

    m_graphicsQueue = m_device.getQueue(queueFamilyIndices.graphicsFamily.value(), 0);
    m_presentQueue = m_device.getQueue(queueFamilyIndices.presentFamily.value(), 0);
}

} // namespace vulkan

} // namespace gfx
