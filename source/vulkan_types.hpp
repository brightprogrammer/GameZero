#ifndef GAMEZERO_VULKAN_TYPES_HPP
#define GAMEZERO_VULKAN_TYPES_HPP

#include "common.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "utils.hpp"
#include "vulkan/vulkan_core.h"
#include <memory>
#include "vulkan/vk_mem_alloc.h"

namespace GameZero{
    /// represents allocated buffer by vulkan memory allocator
    struct AllocatedBuffer{
        VkBuffer buffer;
        VmaAllocation allocation;
    };

    /// represents an allocated image
    struct AllocatedImage{
        VkImage image;
        // VkImageView imageView;
        // VkFormat imageFormat;
        // VkImageUsageFlags imageUsage;
        // VkExtent2D imageExtent;
        VmaAllocation allocation;
    };

    // create buffer
    inline AllocatedBuffer CreateBuffer(VmaAllocator allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memUsage){
        //allocate vertex buffer
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.size = allocSize;
        bufferInfo.usage = usage;

        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = memUsage;

        AllocatedBuffer newBuffer;

        //allocate the buffer
        CHECK_VK_RESULT(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
            &newBuffer.buffer,
            &newBuffer.allocation,
            nullptr), "Failed to allocate Buffer");

        return newBuffer;
    }

    /// material
    struct Material{
        VkDescriptorSet textureSet = VK_NULL_HANDLE;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
    };

    /// render object
    struct RenderObject{
        struct Mesh* mesh;
        Material* material;

        /// transform or model matrix
        glm::mat4 transform = glm::mat4(1.0f);
    };

    // camera data
    struct GPUCameraData{
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    /// frame data
    struct FrameData{
        VkSemaphore renderSemaphore, presentSemaphore;
        VkFence renderFence;

        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        /// holds gpu camera data during a single renderpass
        AllocatedBuffer cameraBuffer;

        VkDescriptorSet descriptorSet;
    };

    struct UploadContext{
        VkFence uploadFence;
        VkCommandPool commandPool;
    };
}

#endif//GAMEZERO_VULKAN_TYPES_HPP