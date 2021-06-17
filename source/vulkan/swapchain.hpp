/**
 * @file swapchain.hpp
 * @author Siddharth Mishra (bshock665@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-15
 * 
 * @copyright Copyright 2021 Siddharth Mishra. All Rights Reserved
 * 
 */

#ifndef GAMEZERO_VULKAN_SWAPCHAIN_HPP
#define GAMEZERO_VULKAN_SWAPCHAIN_HPP

#include "../common.hpp"
#include "device.hpp"
#include "image.hpp"
#include "surface.hpp"

namespace GameZero{

    struct Swapchain{
        Swapchain() = default;

        /**
         * @brief Construct a new Swapchain object
         * 
         * @param surface 
         * @param device 
         */
        Swapchain(Surface& surface, const Device& device){
            Create(surface, device);
        }

        /**
         * @brief Create the surface
         * 
         * @param surface 
         * @param device 
         */
        void Create(Surface& surface, const Device& device);

        /// swapchain handle
        vk::SwapchainKHR swapchain;
        /// image handles in swapchain
        std::vector<vk::Image> images;
        /// image views for images in swapchain
        std::vector<vk::ImageView> imageViews;
        /// number of images in swapchain
        uint32_t imageCount;
        /// swapchain image extent
        vk::Extent2D imageExtent;
        /// swapchain image format
        vk::Format imageFormat;

        /// destroy swapchain and image views
        void Destroy(const Device& device);
    };

}

#endif//GAMEZERO_VULKAN_SWAPCHAIN_HPP
