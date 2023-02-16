#ifndef GFX_VULKAN_CONTEXT_HPP
#define GFX_VULKAN_CONTEXT_HPP

#include "../context.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {

namespace vulkan {

class VulkanContext : public Context {
public:
    VulkanContext(core::Window& window);
    ~VulkanContext() override;

    static constexpr Api api = Api::Vulkan;

private:
    void createInstance();

private:
    core::Window& m_window;
    vk::Instance m_instance;  

};

} // namespace vulkan

} // namespace gfx

#endif