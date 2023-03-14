#ifndef GFX_IMAGE_HPP
#define GFX_IMAGE_HPP

#include "context.hpp"

#include "../core/types.hpp"

namespace gfx {

class Image;

class ImageView {
public:
    struct Builder {
        /**
         * @brief Builder for creating image view objects
         * Default Subresource Range Base Mip Level = 0
         * Default Subresource Range Base Array Layer = 0
         * Default Subresource Range Level Count = 1
         * Default Subresource Range Layer Count = 1
         * Default Components = vk::ComponentMapping{ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA }
         * 
         */
        Builder();

        Builder& setFlags(vk::ImageViewCreateFlags flags);
        Builder& setImage(const Image& image);
        Builder& setImage(const vk::Image& image);
        Builder& setViewType(vk::ImageViewType imageViewType);
        Builder& setFormat(vk::Format format);
        Builder& setComponents(const vk::ComponentMapping& componentMapping);
        Builder& setSubresourceRangeAspectMask(vk::ImageAspectFlags imageAspectFlags);
        Builder& setSubresourceRangeBaseMipLevel(uint32_t baseMipLevel);
        Builder& setSubresourceRangeLevelCount(uint32_t mipLevel);
        Builder& setSubresourceRangeBaseArrayLayer(uint32_t baseArrayLayer);
        Builder& setSubresourceRangeLayerCount(uint32_t arrayLayer);

        ImageView build(const Context *ctx);

        vk::ImageViewCreateInfo m_imageViewCreateInfo;
    };

    ImageView() : m_ctx(nullptr), m_imageView(VK_NULL_HANDLE) {}

    ~ImageView();

    ImageView(ImageView&& imageView);
    ImageView(const ImageView&) = delete;

    vk::ImageView getImageView() const { return m_imageView; }

private:
    ImageView(const Context *ctx, vk::ImageView imageView);

private:
    const Context *m_ctx;
    vk::ImageView m_imageView;
};


class Image {
public:
    struct Builder {
        /**
         * @brief Builder for creating a Image Object
         * Default Sampling Count = vk::SampleCountFlagBits::e1
         * Default Tiling = vk::ImageTiling::eOptimal
         * Default Sharing Mode = vk::SharingMode::eExclusive
         * Default Initial Layout = vk::ImageLayout::eUndefined
         * Default Mip Level = 1
         * Default Array Layer = 1
         * 
         */
        Builder();
        Builder(const Builder&) = delete;
        Builder(const Builder&&) = delete;

        Builder& setCreateFlags(vk::ImageCreateFlags imageCreateFlags);
        Builder& setType(vk::ImageType type);
        Builder& setFormat(vk::Format format);
        Builder& setExtent(core::Dimensions dimensions);
        Builder& setMipLevels(uint32_t mipLevels);
        Builder& setArrayLayers(uint32_t arrayLayers);
        Builder& setSamples(vk::SampleCountFlagBits sampleCountFlags);
        Builder& setTiling(vk::ImageTiling imageTiling);
        Builder& setUsage(vk::ImageUsageFlags imageUsageFlags);
        Builder& addQueueFamilyIndex(uint32_t queueFamilyIndex);

        Image build(const Context *ctx);

        vk::ImageCreateInfo m_imageCreateInfo{};
        // Note To Self: this is volatile memory (memory may be reallocated, make sure that the memory is stationary while creating the image!!!)
        std::vector<uint32_t> m_queueFamilyIndices;
    };

    Image() : m_ctx(nullptr), m_image(VK_NULL_HANDLE), m_imageDeviceMemory(VK_NULL_HANDLE), m_format{} {}

    ~Image();

    Image(Image&& image);
    Image(const Image&) = delete;

    vk::Format getFormat() const { return m_format; }
    const vk::Image getImage() const { return m_image; }
    // ImageView createImageView() const;

private:
    Image(const Context *ctx, vk::Image image, vk::DeviceMemory imageDeviceMemory, vk::Format format);

private:
    const Context *m_ctx;
    vk::Image m_image;
    vk::DeviceMemory m_imageDeviceMemory;
    vk::Format m_format;
    
};

} // namespace gfx

#endif