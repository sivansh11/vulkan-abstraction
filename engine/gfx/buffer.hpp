#ifndef GFX_BUFFER_HPP
#define GFX_BUFFER_HPP

#include "device.hpp"

namespace gfx {

class Buffer {
public:
    // incomplete, need to add more sets for less used stuff like flags etc
    struct Builder {    
        Builder& setSize(vk::DeviceSize size);
        Builder& setUsage(vk::BufferUsageFlags usage);
        Builder& setSharingMode(vk::SharingMode sharingMode);
        Builder& setMemoryProperty(vk::MemoryPropertyFlags flags);

        Buffer build(std::shared_ptr<Device> device);

        vk::BufferCreateInfo m_bufferCreateInfo;
        vk::MemoryPropertyFlags m_memoryPropertyFlags;
    };

    Buffer() : m_device(nullptr), m_buffer(VK_NULL_HANDLE), m_deviceMemory(VK_NULL_HANDLE), m_bufferSize(0) {}
    
    ~Buffer();

    Buffer(Buffer&& buffer);
    Buffer& operator=(Buffer&& buffer);

    vk::Buffer getBuffer() const { return m_buffer; }

    // TODO: add copying of buffer
    // Buffer(const Buffer&);
    // Buffer& operator=(const Buffer& buffer);
    bool isMapped() const { return m_mapped; }
    void map(vk::DeviceSize offset = 0);
    void unmap();
    void *getMapped() { return m_mapped; }
    // void flush();
    // void invalidate();

private:
    Buffer(std::shared_ptr<Device> device, vk::Buffer buffer, vk::DeviceMemory deviceMemory, vk::DeviceSize bufferSize);

private:
    std::shared_ptr<Device> m_device;
    vk::Buffer m_buffer;
    vk::DeviceMemory m_deviceMemory;
    vk::DeviceSize m_bufferSize;
    void *m_mapped = nullptr;
};

} // namespace gfx

#endif