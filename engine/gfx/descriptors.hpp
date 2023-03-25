#ifndef GFX_DESCRIPTORS_HPP
#define GFX_DESCRIPTORS_HPP

#include "device.hpp"

#include <map>

namespace gfx {

using DescriptorBindingDescriptions = std::map<uint32_t, vk::DescriptorSetLayoutBinding>;

class DescriptorSetLayout {
public:
    struct Builder {
        Builder& addBinding(uint32_t binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags shaderStageFlags, uint32_t count);
        Builder& setFlags(vk::DescriptorSetLayoutCreateFlags flags);

        DescriptorSetLayout build(std::shared_ptr<Device> device);

        DescriptorBindingDescriptions m_descriptorBindingDescriptions;
        vk::DescriptorSetLayoutCreateInfo m_descriptorSetLayoutCreateInfo;

    };

    DescriptorSetLayout() : m_device(nullptr), m_descriptorSetLayout(VK_NULL_HANDLE) {}

    ~DescriptorSetLayout();

    DescriptorSetLayout(DescriptorSetLayout&& descriptorSetLayout);
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;

    DescriptorSetLayout& operator=(DescriptorSetLayout&& descriptorSetLayout);

    vk::DescriptorSetLayout get() const { return m_descriptorSetLayout; }
    const DescriptorBindingDescriptions& getDescriptorBindingDescriptions() const { return m_descriptorBindingDescriptions; }

private:
    DescriptorSetLayout(std::shared_ptr<Device> device, vk::DescriptorSetLayout descriptorSetLayout, const DescriptorBindingDescriptions& descriptorBindingDescriptions);

private:
    std::shared_ptr<Device>      m_device;
    vk::DescriptorSetLayout      m_descriptorSetLayout;
    DescriptorBindingDescriptions m_descriptorBindingDescriptions;
};

class DescriptorSet {
public:
    DescriptorSet() = delete;

    const vk::DescriptorSet& get() const { return m_descriptorSet; }

    struct Update {
        
        Update& addBuffer(uint32_t binding, const vk::DescriptorBufferInfo& descriptorBufferInfo);

        std::vector<vk::WriteDescriptorSet> m_writeDescriptorSets; 

    }; 

    void update(Update update);

private:
    friend class DescriptorPool;
    friend class std::vector<DescriptorSet>;

public: // todo remove this
    DescriptorSet(std::shared_ptr<Device> device, vk::DescriptorSet descriptorSet, const DescriptorSetLayout& descriptorSetLayout);

private:
    std::shared_ptr<Device> m_device;
    vk::DescriptorSet m_descriptorSet;
    const DescriptorSetLayout& m_descriptorSetLayout;
};

class DescriptorPool {
public:
    struct Builder {
        Builder& addPoolSize(vk::DescriptorType type, uint32_t count);
        Builder& setFlags(vk::DescriptorPoolCreateFlags flags);
        Builder& setMaxSets(uint32_t maxSets);

        DescriptorPool build(std::shared_ptr<Device> device);

        std::vector<vk::DescriptorPoolSize> m_descriptorPoolSizes;
        vk::DescriptorPoolCreateInfo m_descriptorPoolCreateInfo;
    };

    DescriptorPool() : m_device(nullptr), m_descriptorPool(VK_NULL_HANDLE) {}

    ~DescriptorPool();

    DescriptorPool(DescriptorPool&& descriptorPool);
    DescriptorPool(const DescriptorPool&) = delete;

    DescriptorPool& operator=(DescriptorPool&& descriptorPool);

    vk::DescriptorPool get() const { return m_descriptorPool; }

    struct SetAllocateInfo {

        SetAllocateInfo& addLayout(const DescriptorSetLayout& descriptorSetLayout);

        std::vector<const DescriptorSetLayout *> m_descriptorSetLayoutPtrs; 
    };

    std::vector<DescriptorSet> allocate(const SetAllocateInfo& setAllocateInfo);

private:
    DescriptorPool(std::shared_ptr<Device> device, vk::DescriptorPool descriptorPool);

private:
    std::shared_ptr<Device> m_device;
    vk::DescriptorPool      m_descriptorPool;
};

// class DescriptorSet;

// class DescriptorPool {
// public:
//     struct Builder {
//         Builder& addPoolSize(const vk::DescriptorPoolSize& descriptorPoolSize);
//         Builder& setFlags(vk::DescriptorPoolCreateFlags flags);
//         Builder& setMaxSets(uint32_t maxSets);
        
//         DescriptorPool build(std::shared_ptr<Device> device);

//         std::vector<vk::DescriptorPoolSize> m_descriptorPoolSizes;
//         vk::DescriptorPoolCreateInfo m_descriptorPoolCreateInfo;
//     };

//     DescriptorPool() : m_device(nullptr), m_descriptorPool(VK_NULL_HANDLE) {}

//     ~DescriptorPool();

//     DescriptorPool(DescriptorPool&& descriptorPool);
//     DescriptorPool(const DescriptorPool&) = delete;

//     DescriptorPool& operator=(DescriptorPool&& descriptorPool);

//     vk::DescriptorPool get() const { return m_descriptorPool; }

//     struct DescriptorAllocateInfo {
//         DescriptorAllocateInfo& addDescriptorSetLayout(const DescriptorSetLayout& descriptorSetLayout);

//         std::vector<DescriptorSetLayout> m_descriptorSetLayouts;
//     };

//     std::vector<DescriptorSet> allocateDescriptors(const DescriptorAllocateInfo& descriptorSetAllocateInfo);

// private:
//     DescriptorPool(std::shared_ptr<Device> device, vk::DescriptorPool descriptorPool);

// private:
//     std::shared_ptr<Device> m_device;
//     vk::DescriptorPool m_descriptorPool;
// };

// class DescriptorSet {
// friend class DescriptorPool;
// public:

//     DescriptorSet() : m_device(nullptr), m_descriptorSet(VK_NULL_HANDLE) {}

//     ~DescriptorSet();

//     DescriptorSet(DescriptorSet&& descriptorSet);
//     DescriptorSet(const DescriptorSet&) = delete;

//     DescriptorSet& operator=(DescriptorSet&& descriptorSet);

//     const vk::DescriptorSet& get() const { return m_descriptorSet; }

//     DescriptorSet(std::shared_ptr<Device> device, vk::DescriptorSet descriptorSet, vk::DescriptorType descriptorType);

//     struct WriteInfo {
//         WriteInfo& writeBuffer(uint32_t binding, const vk::DescriptorBufferInfo& descriptorBufferInfo);

//         // std::vector<vk::WriteDescriptorSet> m_writeDescriptorSets;
//         std::vector<std::pair<uint32_t, vk::DescriptorBufferInfo>> m_descriptorBufferInfos;
//     };

//     void write(const WriteInfo& writeInfo);

// private:
//     std::shared_ptr<Device> m_device;
//     vk::DescriptorSet m_descriptorSet;
//     vk::DescriptorType m_descriptorType;
// };

} // namespace gfx

#endif