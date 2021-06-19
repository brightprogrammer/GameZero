#ifndef GAMEZERO_VULKAN_TYPES_HPP
#define GAMEZERO_VULKAN_TYPES_HPP

#include <memory>
#include "../utils.hpp"
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.hpp"
#include "glm/ext/matrix_transform.hpp"


namespace GameZero{
    /// represents allocated buffer by vulkan memory allocator
    struct AllocatedBuffer{
        vk::Buffer buffer;
        vma::Allocation allocation;
    };

    // create buffer
    inline AllocatedBuffer CreateBuffer(const vma::Allocator& allocator, size_t allocSize, vk::BufferUsageFlags usage, vma::MemoryUsage memUsage){
        //allocate vertex buffer
        vk::BufferCreateInfo bufferInfo = {};
        bufferInfo.pNext = nullptr;
        bufferInfo.size = allocSize;
        bufferInfo.usage = usage;

        vma::AllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = memUsage;

        AllocatedBuffer newBuffer;

        //allocate the buffer
        CHECK_VK_RESULT(allocator.createBuffer(&bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, nullptr), "Failed to allocate Buffer");
        
        return newBuffer;
    }

    /// material
    struct Material{
        vk::DescriptorSet textureSet = VK_NULL_HANDLE;
        vk::Pipeline pipeline;
        vk::PipelineLayout pipelineLayout;
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
        vk::Semaphore renderSemaphore, presentSemaphore;
        vk::Fence renderFence;

        vk::CommandPool commandPool;
        vk::CommandBuffer commandBuffer;

        /// holds gpu camera data during a single renderpass
        AllocatedBuffer cameraBuffer;

        vk::DescriptorSet descriptorSet;
    };

    struct UploadContext{
        vk::Fence uploadFence;
        vk::CommandPool commandPool;
    };
}

#endif//GAMEZERO_VULKAN_TYPES_HPP