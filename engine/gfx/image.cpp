#include "image.hpp"

#include "../core/log.hpp"

namespace gfx {

// **********Image::Builder**********
Image::Builder::Builder() {}

Image::Builder& Image::Builder::setCreateFlags(vk::ImageCreateFlags imageCreateFlags) {
    m_imageCreateInfo.setFlags(imageCreateFlags);
    return *this;
}

Image::Builder& Image::Builder::setType(vk::ImageType type) {
    m_imageCreateInfo.setImageType(type);
    return *this;
}

Image::Builder& Image::Builder::setFormat(vk::Format format) {
    m_imageCreateInfo.setFormat(format);
    return *this;
}

Image::Builder& Image::Builder::setExtent(core::Dimensions dimensions) {
    m_imageCreateInfo.setExtent({static_cast<uint32_t>(dimensions.x), static_cast<uint32_t>(dimensions.y), static_cast<uint32_t>(dimensions.z)});
    return *this;
}

Image::Builder& Image::Builder::setMipLevels(uint32_t mipLevels) {
    m_imageCreateInfo.setMipLevels(mipLevels);
    return *this;
}

Image::Builder& Image::Builder::setArrayLayers(uint32_t arrayLayers) {
    m_imageCreateInfo.setArrayLayers(arrayLayers);
    return *this;
}

Image::Builder& Image::Builder::setSamples(vk::SampleCountFlagBits sampleCountFlags) {
    m_imageCreateInfo.setSamples(sampleCountFlags);
    return *this;
}

Image::Builder& Image::Builder::setTiling(vk::ImageTiling imageTiling) {
    m_imageCreateInfo.setTiling(imageTiling);
    return *this;
}

Image::Builder& Image::Builder::setUsage(vk::ImageUsageFlags imageUsageFlags) {
    m_imageCreateInfo.setUsage(imageUsageFlags);
    return *this;
}

Image::Builder& Image::Builder::addQueueFamilyIndex(uint32_t queueFamilyIndex) {
    m_queueFamilyIndices.push_back(queueFamilyIndex);
    return *this;
}

Image Image::Builder::build(std::shared_ptr<Device> device) {
    m_imageCreateInfo.setQueueFamilyIndexCount(m_queueFamilyIndices.size())
                     .setPQueueFamilyIndices(m_queueFamilyIndices.data());

    vk::Image image;

    if (device->getDevice().createImage(&m_imageCreateInfo, nullptr, &image) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create Image!");
    }

    vk::MemoryRequirements memoryRequirements = device->getDevice().getImageMemoryRequirements(image);
    
    vk::MemoryAllocateInfo memoryAllocateInfo = vk::MemoryAllocateInfo{} 
        .setAllocationSize(memoryRequirements.size)
        .setMemoryTypeIndex(device->findMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));

    vk::DeviceMemory deviceMemory;

    if (device->getDevice().allocateMemory(&memoryAllocateInfo, nullptr, &deviceMemory) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to allocate device memory!");
    }

    device->getDevice().bindImageMemory(image, deviceMemory, 0);

    INFO("Created Image!");

    return {device, image, deviceMemory, m_imageCreateInfo.format};
}

// **********Image**********
Image::Image(std::shared_ptr<Device> device, vk::Image image, vk::DeviceMemory imageDeviceMemory, vk::Format format) : m_device(device), m_image(image), m_imageDeviceMemory(imageDeviceMemory), m_format(format) {

}

Image::~Image() {
    if (m_image) m_device->getDevice().destroyImage(m_image);
    if (m_imageDeviceMemory) m_device->getDevice().freeMemory(m_imageDeviceMemory);
    m_image = VK_NULL_HANDLE;
    m_imageDeviceMemory = VK_NULL_HANDLE;
}

Image::Image(Image&& image) : m_device(image.m_device), m_image(image.m_image), m_imageDeviceMemory(image.m_imageDeviceMemory), m_format(image.m_format) {
    image.m_image = VK_NULL_HANDLE;
    image.m_imageDeviceMemory = VK_NULL_HANDLE;
}

// **********ImageView::Builder*********
ImageView::Builder::Builder() {}

ImageView::Builder& ImageView::Builder::setFlags(vk::ImageViewCreateFlags flags) {
    m_imageViewCreateInfo.setFlags(flags);
    return *this;
}

ImageView::Builder& ImageView::Builder::setImage(const Image& image) {
    m_imageViewCreateInfo.setImage(image.getImage());
    return *this;
}

ImageView::Builder& ImageView::Builder::setImage(const vk::Image& image) {
    m_imageViewCreateInfo.setImage(image);
    return *this;
}

ImageView::Builder& ImageView::Builder::setViewType(vk::ImageViewType imageViewType) {
    m_imageViewCreateInfo.setViewType(imageViewType);
    return *this;
}

ImageView::Builder& ImageView::Builder::setFormat(vk::Format format) {
    m_imageViewCreateInfo.setFormat(format);
    return *this;
}

ImageView::Builder& ImageView::Builder::setComponents(const vk::ComponentMapping& componentMapping) {
    m_imageViewCreateInfo.setComponents(componentMapping);
    return *this;
}

ImageView::Builder& ImageView::Builder::setSubresourceRangeAspectMask(vk::ImageAspectFlags imageAspectFlags) {
    m_imageViewCreateInfo.subresourceRange.setAspectMask(imageAspectFlags);
    return *this;
}

ImageView::Builder& ImageView::Builder::setSubresourceRangeBaseMipLevel(uint32_t baseMipLevel) {
    m_imageViewCreateInfo.subresourceRange.setBaseMipLevel(baseMipLevel);
    return *this;
}

ImageView::Builder& ImageView::Builder::setSubresourceRangeLevelCount(uint32_t mipLevel) {
    m_imageViewCreateInfo.subresourceRange.setLevelCount(mipLevel);
    return *this;
}

ImageView::Builder& ImageView::Builder::setSubresourceRangeBaseArrayLayer(uint32_t baseArrayLayer) {
    m_imageViewCreateInfo.subresourceRange.setBaseArrayLayer(baseArrayLayer);
    return *this;
}

ImageView::Builder& ImageView::Builder::setSubresourceRangeLayerCount(uint32_t arrayLayer) {
    m_imageViewCreateInfo.subresourceRange.setLayerCount(arrayLayer);
    return *this;
}

ImageView ImageView::Builder::build(std::shared_ptr<Device> device) {

    if (!m_imageViewCreateInfo.image) {
        throw std::runtime_error("Image not set, Didnt call .setImage(Image)");
    }   

    vk::ImageView imageView;

    if (device->getDevice().createImageView(&m_imageViewCreateInfo, nullptr, &imageView) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create image view!");
    }

    INFO("Created Image View!");

    return {device, imageView};
}


// **********ImageView**********
ImageView::ImageView(std::shared_ptr<Device> device, vk::ImageView imageView) : m_device(device), m_imageView(imageView) {

}

ImageView::~ImageView() {
    if (m_imageView) m_device->getDevice().destroyImageView(m_imageView);
    m_imageView = VK_NULL_HANDLE;
}

ImageView::ImageView(ImageView&& imageView) : m_device(imageView.m_device), m_imageView(imageView.m_imageView) {
    imageView.m_imageView = { VK_NULL_HANDLE };
}

} // namespace gfx
