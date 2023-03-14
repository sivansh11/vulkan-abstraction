#include "image.hpp"

namespace gfx {

// **********Image::Builder**********
Image::Builder::Builder() {
    m_imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1)
                     .setTiling(vk::ImageTiling::eOptimal)
                     .setSharingMode(vk::SharingMode::eExclusive)
                     .setInitialLayout(vk::ImageLayout::eUndefined)
                     .setMipLevels(1)
                     .setArrayLayers(1);    
}

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

Image Image::Builder::build(const Context *ctx) {
    m_imageCreateInfo.setQueueFamilyIndexCount(m_queueFamilyIndices.size())
                     .setPQueueFamilyIndices(m_queueFamilyIndices.data());

    vk::Image image;

    if (ctx->getDevice().createImage(&m_imageCreateInfo, nullptr, &image) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create Image!");
    }

    vk::MemoryRequirements memoryRequirements = ctx->getDevice().getImageMemoryRequirements(image);
    
    vk::MemoryAllocateInfo memoryAllocateInfo = vk::MemoryAllocateInfo{} 
        .setAllocationSize(memoryRequirements.size)
        .setMemoryTypeIndex(ctx->findMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));

    vk::DeviceMemory deviceMemory;

    if (ctx->getDevice().allocateMemory(&memoryAllocateInfo, nullptr, &deviceMemory) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to allocate device memory!");
    }

    ctx->getDevice().bindImageMemory(image, deviceMemory, 0);

    return {ctx, image, deviceMemory, m_imageCreateInfo.format};
}

// **********Image**********
Image::Image(const Context *ctx, vk::Image image, vk::DeviceMemory imageDeviceMemory, vk::Format format) : m_ctx(ctx), m_image(image), m_imageDeviceMemory(imageDeviceMemory), m_format(format) {

}

Image::~Image() {
    if (m_image) m_ctx->getDevice().destroyImage(m_image);
    if (m_imageDeviceMemory) m_ctx->getDevice().freeMemory(m_imageDeviceMemory);
    m_image = VK_NULL_HANDLE;
    m_imageDeviceMemory = VK_NULL_HANDLE;
}

Image::Image(Image&& image) : m_ctx(image.m_ctx), m_image(image.m_image), m_imageDeviceMemory(image.m_imageDeviceMemory), m_format(image.m_format) {
    image.m_image = VK_NULL_HANDLE;
    image.m_imageDeviceMemory = VK_NULL_HANDLE;
}

// ImageView Image::createImageView() const {
//     return ImageView::Builder{}
//         .setFormat(getFormat())
//         .build(m_ctx);
// }

// **********ImageView::Builder*********
ImageView::Builder::Builder() {
    m_imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    m_imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    m_imageViewCreateInfo.subresourceRange.levelCount = 1;
    m_imageViewCreateInfo.subresourceRange.layerCount = 1;
    m_imageViewCreateInfo.components = vk::ComponentMapping{ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };

}

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

ImageView ImageView::Builder::build(const Context *ctx) {

    if (!m_imageViewCreateInfo.image) {
        throw std::runtime_error("Image not set, Didnt call .setImage(Image)");
    }   

    vk::ImageView imageView;

    if (ctx->getDevice().createImageView(&m_imageViewCreateInfo, nullptr, &imageView) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create image view!");
    }

    return {ctx, imageView};
}


// **********ImageView**********
ImageView::ImageView(const Context *ctx, vk::ImageView imageView) : m_ctx(ctx), m_imageView(imageView) {

}

ImageView::~ImageView() {
    if (m_imageView) m_ctx->getDevice().destroyImageView(m_imageView);
    m_imageView = VK_NULL_HANDLE;
}

ImageView::ImageView(ImageView&& imageView) : m_ctx(imageView.m_ctx), m_imageView(imageView.m_imageView) {
    imageView.m_imageView = { VK_NULL_HANDLE };
}

} // namespace gfx
