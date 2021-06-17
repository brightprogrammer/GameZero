/**
 * @file device.hpp
 * @author Siddharth Mishra (bshock665@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-15
 * 
 * @copyright Copyright 2021 Siddharth Mishra. All Rights Reserved
 * 
 */

#ifndef GAMEZERO_DEVICE_HPP
#define GAMEZERO_DEVICE_HPP

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.hpp"
#include "surface.hpp"

namespace GameZero{

    /// device operations wrapper
    class Device{
        /// select best physical device based on properties, features etc
        void SelectBestPhysicalDevice(const Surface& surface);

        /// create a logical device
        void CreateDevice(const Surface& surface);

        /// create device memory allocator
        void CreateAllocator();
    public:
        /// default constructor
        Device() = default;

        /// constructor
        /// performs physical device selection, logical device creation
        /// device memory allocator creation 
        Device(const Surface& surface);

        /**
         * @brief Create Logical Device after selection of Physical Device.
         *        Also creates allocator.
         * @param surface 
         */
        void Create(const Surface& surface);

        // /// various device queue types
        // enum class QueueType {Graphics, Present, Compute, Transfer};

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

        /// destroy device and allocator
        void Destroy(){
            allocator.destroy();
            logical.destroy();
        }
    };

}

#endif//GAMEZERO_DEVICE_HPP