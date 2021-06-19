#include "texture.hpp"
#include "utils/assert.hpp"
#include "utils/destruction_queue.hpp"
#include "vulkan/vk_mem_alloc.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"

#include "renderer.hpp"
#include <functional>

bool GameZero::LoadImageFromFile(Renderer* renderer, const char *filename, AllocatedImage &outImage){
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if(!pixels){
        LOG(ERROR, "Failed to read texture from file [ %s ]", filename);
        return false;
    }

    void* pixel_ptr = pixels;
    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    // 8bit pixel format
    vk::Format imageFormat = vk::Format::eR8G8B8A8Srgb;

    // allocate temporary buffer for holding data in cpu
    AllocatedBuffer stagingBuffer = CreateBuffer(renderer->device.allocator, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuOnly);

    // copy image data to temp buffer
    void* dest;
    CHECK_VK_RESULT(renderer->device.allocator.mapMemory(stagingBuffer.allocation, &dest), "Failed to map memory correctly");
    memcpy(dest, pixel_ptr, static_cast<size_t>(imageSize));
    renderer->device.allocator.unmapMemory(stagingBuffer.allocation);

    // release occupied memory
    stbi_image_free(pixels);

    vk::Extent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(texWidth);
    imageExtent.height = static_cast<uint32_t>(texHeight);
    imageExtent.depth = 1;

    // image create info
    vk::ImageCreateInfo imageInfo = {};
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageFormat;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent = imageExtent;
    imageInfo.mipLevels = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
   
    // allocate new image
    AllocatedImage image;

    vma::AllocationCreateInfo allocInfo = {};
    allocInfo.usage = vma::MemoryUsage::eGpuOnly;

    // allocate
    CHECK_VK_RESULT(renderer->device.allocator.createImage(&imageInfo, &allocInfo, &image.image, &image.allocation, nullptr), "Failed to create Image")

    // submit for copy
    renderer->ImmediateSubmit([&](vk::CommandBuffer cmd){
        vk::ImageSubresourceRange range = {};
        range.aspectMask = vk::ImageAspectFlagBits::eColor;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        // barrier to change image layout to transfer dst bit
        vk::ImageMemoryBarrier imageBarrier_toTransfer = {};
        // initial layout of image is undefined
        imageBarrier_toTransfer.oldLayout = vk::ImageLayout::eUndefined;
        // final layout must be optimal for acting as a destination of a transfer op
        imageBarrier_toTransfer.newLayout = vk::ImageLayout::eTransferDstOptimal;
        // this is the image we want to convert
        imageBarrier_toTransfer.image = image.image;
        imageBarrier_toTransfer.subresourceRange = range;
        // source has no access
        imageBarrier_toTransfer.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
        // destination must be accesible for write operations
        imageBarrier_toTransfer.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        // image must be converted to given format before transfer ops
        // because at transfer we need to write and for that image mem must be accessible for writing
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

        // buffer copy region
        vk::BufferImageCopy copyRegion = {};
        copyRegion.bufferImageHeight = 0;
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageExtent = imageExtent;

        // copy image data to image
        cmd.copyBufferToImage(stagingBuffer.buffer, image.image, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);
    
        // barrier to change image to readeable optimal
        vk::ImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
        // old layout was optimal for acting as destination of a transfer op
        imageBarrier_toReadable.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        // new layout must be optimal to be readable from shaders
        imageBarrier_toReadable.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        // source's access was to be wriable
        imageBarrier_toReadable.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        // destination must be readble by shader
        imageBarrier_toReadable.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        // the image must be converted between transfer ops and fragment shader ops
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
    });

    outImage = image;
    LOG(INFO, "Texture image [ %s ] successfully loaded", filename);

    // deletor
    GetDestructionQueue()->PushFunction([=](){
        renderer->device.allocator.destroyImage(image.image, image.allocation);
    });

    // destroy temp buffer finally
    renderer->device.allocator.destroyBuffer(stagingBuffer.buffer, stagingBuffer.allocation);
    return true;
}