#include "texture.hpp"
#include "utils/assert.hpp"
#include "vulkan/vulkan_core.h"

#include "renderer.hpp"
#include <functional>

bool GameZero::LoadImageFromFile(Renderer* renderer, const char *filename, AllocatedImage &outImage){
    int texWidth, textHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename, &texWidth, &textHeight, &texChannels, STBI_rgb_alpha);

    if(!pixels){
        LOG(ERROR, "Failed to read texture from file [ %s ]", filename);
        return false;
    }

    void* pixel_ptr = pixels;
    VkDeviceSize imageSize = texWidth * textHeight * 4;

    // 8bit pixel format
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

    // allocate temporary buffer for holding data in cpu
    AllocatedBuffer stagingBuffer = CreateBuffer(renderer->allocator, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    // copy image data to temp buffer
    void* dest;
    vmaMapMemory(renderer->allocator, stagingBuffer.allocation, &dest);
    memcpy(dest, pixel_ptr, static_cast<size_t>(imageSize));
    vmaUnmapMemory(renderer->allocator, stagingBuffer.allocation);

    // release occupied memory
    stbi_image_free(pixels);

    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(texWidth);
    imageExtent.height = static_cast<uint32_t>(textHeight);
    imageExtent.depth = 1;
    // image create info
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = imageExtent;
    imageInfo.mipLevels = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   
    // allocate new image
    AllocatedImage image;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // allocate
    CHECK_VK_RESULT(vmaCreateImage(renderer->allocator, &imageInfo, &allocInfo, &image.image, &image.allocation, nullptr), "Failed to create Image")

    // submit for copy
    renderer->ImmediateSubmit([&](VkCommandBuffer cmd){
        VkImageSubresourceRange range = {};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        // barrier to change image layout to transfer dst bit
        VkImageMemoryBarrier imageBarrier_toTransfer = {};
        imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        // initial layout of image is undefined
        imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // final layout must be optimal for acting as a destination of a transfer op
        imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        // this is the image we want to convert
        imageBarrier_toTransfer.image = image.image;
        imageBarrier_toTransfer.subresourceRange = range;
        // source has no access
        imageBarrier_toTransfer.srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE_KHR;
        // destination must be accesible for write operations
        imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        // image must be converted to given format before transfer ops
        // because at transfer we need to write and for that image mem must be accessible for writing
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

        // buffer copy region
        VkBufferImageCopy copyRegion = {};
        copyRegion.bufferImageHeight = 0;
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageExtent = imageExtent;

        // copy image data to image
        vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    
        // barrier to change image to readeable optimal
        VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
        // old layout was optimal for acting as destination of a transfer op
        imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        // new layout must be optimal to be readable from shaders
        imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // source's access was to be wriable
        imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        // destination must be readble by shader
        imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // the image must be converted between transfer ops and fragment shader ops
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
    });

    outImage = image;
    LOG(INFO, "Texture image [ %s ] successfully loaded", filename);
    
    // deletor
    renderer->PushFunction([=](){
        vmaDestroyImage(renderer->allocator, image.image, image.allocation);
        LOG(INFO, "Destroyed Image");
    });


    // destroy temp buffer finally
    vmaDestroyBuffer(renderer->allocator, stagingBuffer.buffer, stagingBuffer.allocation);
    return true;
}