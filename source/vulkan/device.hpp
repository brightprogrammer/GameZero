#ifndef GAMEZERO_DEVICE_HPP
#define GAMEZERO_DEVICE_HPP

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.hpp"

namespace GameZero{

    /// device operations wrapper
    class Device{
        /// select best physical device based on properties, features etc
        void SelectBestPhysicalDevice();

        /// create a logical device
        void CreateDevice();

        /// create device memory allocator
        void CreateAllocator();
    public:
        /// constructor
        /// performs physical device selection, logical device creation
        /// device memory allocator creation 
        Device();

        /// various device queue types
        enum class QueueType {Graphics, Present, Compute, Transfer};

        /// vulkan physical device handle 
        vk::PhysicalDevice physical;
        /// vulkan logical device handle
        vk::Device logical;

        /// device memory allocator
        vma::Allocator allocator;

        /// graphics queue handle
        vk::Queue graphicsQueue;
        /// graphics queue index
        uint32_t graphicsQueueIndex;

        /// presentation queue handle
        vk::Queue presentQueue;
        /// presentation queue index
        uint32_t presentQueueIndex;

        /// destroy device
        void Destroy(){
            vkDestroyDevice(logical, nullptr);
        }
    };

}

#endif//GAMEZERO_DEVICE_HPP