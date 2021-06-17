#ifndef GAMEZERO_SWAPCHAIN_HPP
#define GAMEZERO_SWAPCHAIN_HPP

#include "../common.hpp"
#include "device.hpp"

namespace GameZero{

    struct Swapchain{
        Swapchain() = default;

        /**
         * @brief Construct a new Swapchain object
         *
         * @param device
         */
        Swapchain(const Device& device){
            Create(device);
        }

        /**
         * @brief Create Swapchain
         *
         * @param device
         */
        void Create(const Device& device);

        /// pointer to device used for creation of swapchain
        const Device *usedDevice;

        /// swapchain handle
        vk::SwapchainKHR swapchain;
        /// image format of images in swapchain
        vk::Format imageFormat;
        /// size of images in swapchain
        vk::Extent2D imageExtent;
        /// image handles in swapchain
        std::vector<vk::Image> images;
        /// image views of images in swapchain
        std::vector<vk::ImageView> imageViews;
        /// number of images in swapchain
        uint32_t imageCount;

        /// destroy swapchain and image views
        void Destroy(){
            for(const auto& imageView : imageViews) vkDestroyImageView(usedDevice->logical, imageView, nullptr);
            vkDestroySwapchainKHR(usedDevice->logical, swapchain, nullptr);
        }
    };

}

#endif//GAMEZERO_SWAPCHAIN_HPP
