#ifndef GAMEZERO_SWAPCHAIN_HPP
#define GAMEZERO_SWAPCHAIN_HPP

#include "common.hpp"
#include "device.hpp"

namespace GameZero{

        struct Swapchain{
        /// swapchain handle
        VkSwapchainKHR swapchain;
 
        /// image format of images in swapchain
        VkFormat imageFormat;

        /// size of images in swapchain
        VkExtent2D imageExtent;

        /// image handles in swapchain
        std::vector<VkImage> images;

        /// image views of images in swapchain
        std::vector<VkImageView> imageViews;

        /// number of images in swapchain
        uint32_t imageCount;

        /// destroy swapchain and image views
        void Destroy(const Device& device){
            for(const auto& imageView : imageViews) vkDestroyImageView(device.logical, imageView, nullptr);
            vkDestroySwapchainKHR(device.logical, swapchain, nullptr);
        }
    };

}

#endif//GAMEZERO_SWAPCHAIN_HPP