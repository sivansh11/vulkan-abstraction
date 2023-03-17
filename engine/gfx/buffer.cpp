#include "buffer.hpp"

namespace gfx {

// **********Buffer::Builder**********
Buffer::Builder& Buffer::Builder::setSize(vk::DeviceSize size) {
    m_bufferCreateInfo.setSize(size);
    return *this;
}

Buffer::Builder& Buffer::Builder::setUsage(vk::BufferUsageFlags usage) {
    m_bufferCreateInfo.setUsage(usage);
    return *this;
}

Buffer::Builder& Buffer::Builder::setSharingMode(vk::SharingMode sharingMode) {
    m_bufferCreateInfo.setSharingMode(sharingMode);
    return *this;
}

Buffer::Builder& Buffer::Builder::setMemoryProperty(vk::MemoryPropertyFlags flags) {
    m_memoryPropertyFlags = flags;
    return *this;
}

Buffer Buffer::Builder::build(std::shared_ptr<Device> device) {
    assert(m_memoryPropertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent && "buffer must be host coherent atm, temporary!");
    vk::Buffer buffer;
    if (device->getDevice().createBuffer(&m_bufferCreateInfo, nullptr, &buffer) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create buffer!");
    }
    vk::MemoryRequirements memoryRequirements = device->getDevice().getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo memoryAllocateInfo = vk::MemoryAllocateInfo{}
        .setAllocationSize(memoryRequirements.size)
        .setMemoryTypeIndex(device->findMemoryType(memoryRequirements.memoryTypeBits, m_memoryPropertyFlags));

    vk::DeviceMemory deviceMemory;

    auto res = device->getDevice().allocateMemory(&memoryAllocateInfo, nullptr, &deviceMemory);

    if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to allocate device memory!");
    }

    device->getDevice().bindBufferMemory(buffer, deviceMemory, 0);

    return {device, buffer, deviceMemory, m_bufferCreateInfo.size};
}

// **********Buffer**********
Buffer::Buffer(std::shared_ptr<Device> device, vk::Buffer buffer, vk::DeviceMemory deviceMemory, uint64_t bufferSize)  
  : m_device(device), m_buffer(buffer), m_deviceMemory(deviceMemory), m_bufferSize(bufferSize) {

}

Buffer::~Buffer() {
    if (m_buffer) m_device->getDevice().destroyBuffer(m_buffer);
    if (m_deviceMemory) m_device->getDevice().freeMemory(m_deviceMemory);
    m_buffer = VK_NULL_HANDLE;
    m_deviceMemory = VK_NULL_HANDLE;
}

Buffer::Buffer(Buffer&& buffer) 
  : m_device(buffer.m_device), m_buffer(buffer.m_buffer), m_deviceMemory(buffer.m_deviceMemory) {
    buffer.m_device = nullptr;
    buffer.m_buffer = VK_NULL_HANDLE;
    buffer.m_deviceMemory = VK_NULL_HANDLE;
}

Buffer& Buffer::operator=(Buffer&& buffer) {
    m_device = buffer.m_device;
    m_buffer = buffer.m_buffer;
    m_deviceMemory = buffer.m_deviceMemory;
    buffer.m_device = nullptr;
    buffer.m_buffer = VK_NULL_HANDLE;
    buffer.m_deviceMemory = VK_NULL_HANDLE;
    return *this;
}

void Buffer::map(vk::DeviceSize offset) {
    assert(offset < m_bufferSize);
    m_mapped = m_device->getDevice().mapMemory(m_deviceMemory, offset, m_bufferSize, vk::MemoryMapFlagBits{});
}

void Buffer::unmap() {
    m_device->getDevice().unmapMemory(m_deviceMemory);
    m_mapped = nullptr;
}

} // namespace gfx
