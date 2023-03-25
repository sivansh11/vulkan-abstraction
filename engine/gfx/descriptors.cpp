#include "descriptors.hpp"

namespace gfx {
    
// **********DescriptorSetLayout::Builder**********
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(uint32_t binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags shaderStageFlags, uint32_t count) {
    assert(!m_descriptorBindingDescriptions.contains(binding) && "Binding already in use!");
    m_descriptorBindingDescriptions[binding] = vk::DescriptorSetLayoutBinding{}
        .setBinding(binding)
        .setDescriptorCount(count)
        .setDescriptorType(descriptorType)
        .setStageFlags(shaderStageFlags);
    return *this;
}

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::setFlags(vk::DescriptorSetLayoutCreateFlags flags) {
    m_descriptorSetLayoutCreateInfo.setFlags(flags);
    return *this;  
}

DescriptorSetLayout DescriptorSetLayout::Builder::build(std::shared_ptr<Device> device) {
    std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    descriptorSetLayoutBindings.reserve(m_descriptorBindingDescriptions.size());
    for (auto& [binding, descriptorSetLayoutBinding] : m_descriptorBindingDescriptions) {
        descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
    }
    m_descriptorSetLayoutCreateInfo.setBindingCount(descriptorSetLayoutBindings.size())
                                   .setPBindings(descriptorSetLayoutBindings.data());
    vk::DescriptorSetLayout descriptorSetLayout;
    auto res = device->get().createDescriptorSetLayout(&m_descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
    if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create Descriptor Set Layout!");
    }
    return {device, descriptorSetLayout, m_descriptorBindingDescriptions};
}

// **********DescriptorSetLayout**********
DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> device, vk::DescriptorSetLayout descriptorSetLayout, const DescriptorBindingDescriptions& descriptorBindingDescriptions) 
  : m_device(device), m_descriptorSetLayout(descriptorSetLayout), m_descriptorBindingDescriptions(descriptorBindingDescriptions) {
    INFO("Created Descriptor Set Layout!");
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& descriptorSetLayout) 
  : m_device(descriptorSetLayout.m_device), m_descriptorSetLayout(descriptorSetLayout.m_descriptorSetLayout), m_descriptorBindingDescriptions(descriptorSetLayout.m_descriptorBindingDescriptions) {
    descriptorSetLayout.m_device = nullptr;
    descriptorSetLayout.m_descriptorSetLayout = VK_NULL_HANDLE;
    descriptorSetLayout.m_descriptorBindingDescriptions.clear();
}

DescriptorSetLayout::~DescriptorSetLayout() {
    if (m_descriptorSetLayout) m_device->get().destroyDescriptorSetLayout(m_descriptorSetLayout);
    m_device = nullptr;
    m_descriptorSetLayout = VK_NULL_HANDLE;
    m_descriptorBindingDescriptions.clear();
}

DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& descriptorSetLayout) {
    m_device = descriptorSetLayout.m_device;
    m_descriptorSetLayout = descriptorSetLayout.m_descriptorSetLayout;
    m_descriptorBindingDescriptions = descriptorSetLayout.m_descriptorBindingDescriptions;
    descriptorSetLayout.m_device = nullptr;
    descriptorSetLayout.m_descriptorSetLayout = VK_NULL_HANDLE;
    descriptorSetLayout.m_descriptorBindingDescriptions.clear();
    return *this;
}

// **********DescriptorSet**********
DescriptorSet::DescriptorSet(std::shared_ptr<Device> device, vk::DescriptorSet descriptorSet, const DescriptorSetLayout& descriptorSetLayout) 
  : m_device(device), m_descriptorSet(descriptorSet), m_descriptorSetLayout(descriptorSetLayout) {

}

DescriptorSet::Update& DescriptorSet::Update::addBuffer(uint32_t binding, const vk::DescriptorBufferInfo& descriptorBufferInfo) {
    vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet{}
        .setPBufferInfo(&descriptorBufferInfo)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDstArrayElement(0)
        .setDstBinding(binding);
        // .setDstSet();
    m_writeDescriptorSets.push_back(writeDescriptorSet);
    return *this;
}

void DescriptorSet::update(Update update) {
    for (auto& writeDescriptorSet : update.m_writeDescriptorSets) {
        assert(m_descriptorSetLayout.getDescriptorBindingDescriptions().contains(writeDescriptorSet.dstBinding));
        writeDescriptorSet.setDstSet(m_descriptorSet);
    }
    m_device->get().updateDescriptorSets(update.m_writeDescriptorSets.size(), update.m_writeDescriptorSets.data(), 0, nullptr);
}

// **********DescriptorPool::Builder**********
DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(vk::DescriptorType type, uint32_t count) {
    m_descriptorPoolSizes.emplace_back(type, count);
    return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::setFlags(vk::DescriptorPoolCreateFlags flags) {
    m_descriptorPoolCreateInfo.setFlags(flags);
    return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t maxSets) {
    m_descriptorPoolCreateInfo.setMaxSets(maxSets);
    return *this;
}

DescriptorPool DescriptorPool::Builder::build(std::shared_ptr<Device> device) {
    m_descriptorPoolCreateInfo.setPoolSizeCount(m_descriptorPoolSizes.size())
                              .setPPoolSizes(m_descriptorPoolSizes.data());
    vk::DescriptorPool descriptorPool;
    auto res = device->get().createDescriptorPool(&m_descriptorPoolCreateInfo, nullptr, &descriptorPool);
    if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create Descriptor Pool!");
    }
    return {device, descriptorPool};
}

// **********DescriptorPool**********
DescriptorPool::DescriptorPool(std::shared_ptr<Device> device, vk::DescriptorPool descriptorPool)
  : m_device(device), m_descriptorPool(descriptorPool) {
    INFO("Created Descriptor Pool!");
}

DescriptorPool::DescriptorPool(DescriptorPool&& descriptorPool) 
  : m_device(descriptorPool.m_device), m_descriptorPool(descriptorPool.m_descriptorPool) {
    descriptorPool.m_device = nullptr;
    descriptorPool.m_descriptorPool = VK_NULL_HANDLE;
}

DescriptorPool::~DescriptorPool() {
    if (m_descriptorPool) m_device->get().destroyDescriptorPool(m_descriptorPool);
    m_device = nullptr;
    m_descriptorPool = VK_NULL_HANDLE;
}

DescriptorPool& DescriptorPool::operator=(DescriptorPool&& descriptorPool) {
    m_device = descriptorPool.m_device;
    m_descriptorPool = descriptorPool.m_descriptorPool;
    descriptorPool.m_device = nullptr;
    descriptorPool.m_descriptorPool = VK_NULL_HANDLE;
    return *this;
}

DescriptorPool::SetAllocateInfo& DescriptorPool::SetAllocateInfo::addLayout(const DescriptorSetLayout& descriptorSetLayout) {
    m_descriptorSetLayoutPtrs.push_back(&descriptorSetLayout);
    return *this;
}

std::vector<DescriptorSet> DescriptorPool::allocate(const SetAllocateInfo& setAllocateInfo) {
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.reserve(setAllocateInfo.m_descriptorSetLayoutPtrs.size());
    for (auto descriptorSetLayoutPtr : setAllocateInfo.m_descriptorSetLayoutPtrs) {
        descriptorSetLayouts.push_back(descriptorSetLayoutPtr->get());
    }
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo{}
        .setDescriptorSetCount(descriptorSetLayouts.size())
        .setPSetLayouts(descriptorSetLayouts.data())
        .setDescriptorPool(m_descriptorPool);
    std::vector<vk::DescriptorSet> vkDescriptorSets;
    vkDescriptorSets.resize(descriptorSetLayouts.size());
    auto res = m_device->get().allocateDescriptorSets(&descriptorSetAllocateInfo, vkDescriptorSets.data());
    if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to Allocate Descriptor Sets!");
    }
    std::vector<DescriptorSet> descriptorSets;
    descriptorSets.reserve(descriptorSetLayouts.size());
    for (int i = 0; i < descriptorSetLayouts.size(); i++) {
        descriptorSets.emplace_back(m_device, vkDescriptorSets[i], *setAllocateInfo.m_descriptorSetLayoutPtrs[i]);
    }

    return descriptorSets;
}


// // **********DescriptorPool::Builder**********
// DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(const vk::DescriptorPoolSize& descriptorPoolSize) {
//     m_descriptorPoolSizes.push_back(descriptorPoolSize);
//     return *this;
// }

// DescriptorPool::Builder& DescriptorPool::Builder::setFlags(vk::DescriptorPoolCreateFlags flags) {
//     m_descriptorPoolCreateInfo.setFlags(flags);
//     return *this;
// }

// DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t maxSets) {
//     m_descriptorPoolCreateInfo.setMaxSets(maxSets);
//     return *this;
// }

// DescriptorPool DescriptorPool::Builder::build(std::shared_ptr<Device> device) {
//     m_descriptorPoolCreateInfo.setPoolSizeCount(m_descriptorPoolSizes.size())
//                               .setPPoolSizes(m_descriptorPoolSizes.data());
//     vk::DescriptorPool descriptorPool;
//     auto res = device->get().createDescriptorPool(&m_descriptorPoolCreateInfo, nullptr, &descriptorPool);
//     if (res != vk::Result::eSuccess) {
//         throw std::runtime_error("Failed to create descriptor pool!");
//     }
//     return {device, descriptorPool};
// }

// // **********DescriptorPool**********
// DescriptorPool::DescriptorPool(std::shared_ptr<Device> device, vk::DescriptorPool descriptorPool) : m_device(device), m_descriptorPool(descriptorPool) {
//     INFO("Created descriptor Pool!");
// }

// DescriptorPool::~DescriptorPool() {
//     if (m_descriptorPool) m_device->get().destroyDescriptorPool(m_descriptorPool);
// }

// DescriptorPool::DescriptorPool(DescriptorPool&& descriptorPool) : m_device(descriptorPool.m_device), m_descriptorPool(descriptorPool.m_descriptorPool) {
//     descriptorPool.m_descriptorPool = VK_NULL_HANDLE;
//     descriptorPool.m_device = nullptr;
// }

// DescriptorPool& DescriptorPool::operator=(DescriptorPool&& descriptorPool) {
//     m_device = descriptorPool.m_device;
//     m_descriptorPool = descriptorPool.m_descriptorPool;
//     descriptorPool.m_device = nullptr;
//     descriptorPool.m_descriptorPool = VK_NULL_HANDLE;
//     return *this;
// }

// DescriptorPool::DescriptorAllocateInfo& DescriptorPool::DescriptorAllocateInfo::addDescriptorSetLayout(const DescriptorSetLayout& descriptorSetLayout) {
//     m_descriptorSetLayouts.push_back(descriptorSetLayout);
//     return *this;
// }

// std::vector<DescriptorSet> DescriptorPool::allocateDescriptors(const DescriptorAllocateInfo& descriptorSetAllocateInfo) {
//     vk::DescriptorSetAllocateInfo vkDescriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo{}
//         .setDescriptorPool(m_descriptorPool)
//         .setDescriptorSetCount(descriptorSetAllocateInfo.m_descriptorSetLayouts.size())
//         .setPSetLayouts(descriptorSetAllocateInfo.m_descriptorSetLayouts.data());
//     std::vector<vk::DescriptorSet> vkDescriptorSets;
//     vkDescriptorSets.resize(descriptorSetAllocateInfo.m_descriptorSetLayouts.size());
//     auto res = m_device->get().allocateDescriptorSets(&vkDescriptorSetAllocateInfo, vkDescriptorSets.data());
//     if (res != vk::Result::eSuccess) {
//         throw std::runtime_error("Failed to allocate descriptor sets!");
//     }
//     std::vector<DescriptorSet> descriptorSets;
//     descriptorSets.reserve(descriptorSetAllocateInfo.m_descriptorSetLayouts.size());
//     for (auto descriptorSet : vkDescriptorSets) {
//         descriptorSets.emplace_back(m_device, descriptorSet, vk::DescriptorType::eUniformBuffer);
//     }
//     return descriptorSets;
// }

// // **********DescriptorSet**********
// DescriptorSet::DescriptorSet(std::shared_ptr<Device> device, vk::DescriptorSet descriptorSet, vk::DescriptorType descriptorType) : m_device(device), m_descriptorSet(descriptorSet), m_descriptorType(descriptorType) {
//     INFO("Created descriptor Pool!");
// }

// DescriptorSet::~DescriptorSet() {
//     // if (m_descriptorSet) m_device->get().freeDescriptorSets();
// }

// DescriptorSet::DescriptorSet(DescriptorSet&& descriptorSet) : m_device(descriptorSet.m_device), m_descriptorSet(descriptorSet.m_descriptorSet), m_descriptorType(descriptorSet.m_descriptorType) {
//     descriptorSet.m_descriptorSet = VK_NULL_HANDLE;
//     descriptorSet.m_device = nullptr;
// }

// DescriptorSet& DescriptorSet::operator=(DescriptorSet&& descriptorSet) {
//     m_device = descriptorSet.m_device;
//     m_descriptorSet = descriptorSet.m_descriptorSet;
//     m_descriptorType = descriptorSet.m_descriptorType;
//     descriptorSet.m_device = nullptr;
//     descriptorSet.m_descriptorSet = VK_NULL_HANDLE;
//     return *this;
// }

// DescriptorSet::WriteInfo& DescriptorSet::WriteInfo::writeBuffer(uint32_t binding, const vk::DescriptorBufferInfo& descriptorBufferInfo) {
//     m_descriptorBufferInfos.emplace_back(binding, descriptorBufferInfo);
//     return *this;
// }

// void DescriptorSet::write(const WriteInfo& writeInfo) {
//     std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
//     writeDescriptorSets.reserve(writeInfo.m_descriptorBufferInfos.size());
//     for (auto& [binding, bufferInfo] : writeInfo.m_descriptorBufferInfos) {
//         vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet{}
//             .setDstSet(m_descriptorSet)
//             .setDstBinding(binding)
//             .setDstArrayElement(0)
//             .setDescriptorType(m_descriptorType)
//             .setDescriptorCount(1)
//             .setPBufferInfo(&bufferInfo);
//         writeDescriptorSets.push_back(writeDescriptorSet);
//     }

//     m_device->get().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
// }

} // namespace gfx 
