#include "context.hpp"

#include "../core/log.hpp"

#include "syncobjects.hpp"
#include "commandbuffer.hpp"
#include "swapchain.hpp"

#include <iostream>
#include <cassert>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>

namespace gfx {

Context::Context(core::Window& window, bool enableValidation) : m_window(window), m_validations(enableValidation) {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
}

Context::~Context() {
    m_device.destroy();
    m_instance.destroySurfaceKHR(m_surface);
    if (m_validations) {
        m_instance.destroyDebugUtilsMessengerEXT(m_debugUtilsMessenger, nullptr, m_dispatchLoaderDynamic);
    }
    m_instance.destroy();
}

bool Context::checkRequiredLayersSupport(const std::vector<const char *>& m_requiredLayers) noexcept {
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
            ERROR("Vulkan Validation Layer: \n{}", pCallbackData->pMessage);
            break;
        
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            INFO("Vulkan Validation Layer: \n{}", pCallbackData->pMessage);
            break;
        
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            WARN("Vulkan Validation Layer: \n{}", pCallbackData->pMessage);
            break;
        
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            TRACE("Vulkan Validation Layer: \n{}", pCallbackData->pMessage);
            break;
    }

    return VK_FALSE;

}

void Context::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCreateInfo) {
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

void Context::createInstance() {
    auto windowExtensions = m_window.getRequiredExtensions();

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

void Context::createSurface() {
    VkSurfaceKHR tempSurface{};
    if (m_window.createSurface(m_instance, nullptr, &tempSurface) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Failed to get window surface!");
    }
    m_surface = vk::SurfaceKHR{ tempSurface };

    INFO("Vulkan: Created Surface!");
}

void Context::setupDebugMessenger() {
    if (!m_validations) return;
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
    populateDebugMessengerCreateInfo(debugUtilsMessengerCreateInfo);
    if (m_instance.createDebugUtilsMessengerEXT(&debugUtilsMessengerCreateInfo, nullptr, &m_debugUtilsMessenger, m_dispatchLoaderDynamic) != vk::Result::eSuccess) {
        throw std::runtime_error("Vulkan: Failed to create debug messenger!");
    }

    INFO("Vulkan: Setup Debug Messenger!");
}

Context::QueueFamilyIndices Context::findQueueFamilies(const vk::PhysicalDevice& physicalDevice) const {
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

Context::SwapChainSupportDetails Context::querySwapChainSupport(const vk::PhysicalDevice& physicalDevice) const {
    SwapChainSupportDetails details;

    details.formats = physicalDevice.getSurfaceFormatsKHR(m_surface);

    details.capabilites = physicalDevice.getSurfaceCapabilitiesKHR(m_surface);

    details.presentModes = physicalDevice.getSurfacePresentModesKHR(m_surface);

    return details;
}

bool Context::checkRequiredExtensionSupport(const vk::PhysicalDevice& physicalDevice) noexcept {
    auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(m_requiredDeviceExtensions.begin(), m_requiredDeviceExtensions.end());

    for (auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool Context::isDeviceSuitable(const vk::PhysicalDevice& physicalDevice) {
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

void Context::pickPhysicalDevice() {
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

void Context::createLogicalDevice() {
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

uint32_t Context::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlagBits memoryPropertyFlags) const {
    auto physicalDeviceMemoryProperties = m_physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

Context::QueueSubmitInfo::QueueSubmitInfo() {}

Context::QueueSubmitInfo& Context::QueueSubmitInfo::addWaitSemaphore(const Semaphore& semaphore) {
    m_waitSemaphores.push_back(semaphore.getSemaphore());
    return *this;
}

Context::QueueSubmitInfo& Context::QueueSubmitInfo::addSignalSemaphore(const Semaphore& semaphore) {
    m_signalSemaphores.push_back(semaphore.getSemaphore());
    return *this;
}

Context::QueueSubmitInfo& Context::QueueSubmitInfo::addCommandBuffer(const CommandBuffer& commandBuffer) {
    m_commandBuffers.push_back(commandBuffer.getCommandBuffer());
    return *this;
}

Context::QueueSubmitInfo& Context::QueueSubmitInfo::addWaitStage(vk::PipelineStageFlags flags) {
    m_waitStages |= flags;
    return *this;
}

Context::QueueSubmitInfo& Context::QueueSubmitInfo::setFence(const Fence& fence) {
    m_fence = fence.getFence();
    return *this;
}

void Context::submit(const QueueSubmitInfo& queueSubmitInfo) {
    vk::SubmitInfo submitInfo = vk::SubmitInfo{}
        .setCommandBufferCount(queueSubmitInfo.m_commandBuffers.size())
        .setPCommandBuffers(queueSubmitInfo.m_commandBuffers.data())
        .setSignalSemaphoreCount(queueSubmitInfo.m_signalSemaphores.size())
        .setPSignalSemaphores(queueSubmitInfo.m_signalSemaphores.data())
        .setWaitSemaphoreCount(queueSubmitInfo.m_waitSemaphores.size())
        .setPWaitSemaphores(queueSubmitInfo.m_waitSemaphores.data())
        .setPWaitDstStageMask(&queueSubmitInfo.m_waitStages);

    if (m_graphicsQueue.submit(1, &submitInfo, queueSubmitInfo.m_fence) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }
}

Context::PresentInfo::PresentInfo() {}

Context::PresentInfo& Context::PresentInfo::setImageIndex(uint32_t imageIndex) {
    m_imageIndex = imageIndex;
    return *this;
}

Context::PresentInfo& Context::PresentInfo::addWaitSemaphore(const Semaphore& semaphore) {
    m_waitSemaphores.push_back(semaphore.getSemaphore());
    return *this;
} 

Context::PresentInfo& Context::PresentInfo::setSwapChain(const SwapChain& swapChain) {
    m_swapChain = swapChain.getSwapChain();
    return *this;
}

void Context::present(const PresentInfo& presentInfo) {
    vk::SwapchainKHR swapChains[] = { presentInfo.m_swapChain };
    vk::PresentInfoKHR vkPresentInfo = vk::PresentInfoKHR{}
        .setWaitSemaphoreCount(presentInfo.m_waitSemaphores.size())
        .setPWaitSemaphores(presentInfo.m_waitSemaphores.data())
        .setSwapchainCount(1)
        .setPSwapchains(swapChains)
        .setPResults(nullptr)
        .setPImageIndices(&presentInfo.m_imageIndex);

    m_presentQueue.presentKHR(&vkPresentInfo);
}

} // namespace gfx
