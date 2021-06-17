/**
 * @file swapchain.cpp
 * @author Siddharth Mishra (bshock665@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-15
 * 
 * @copyright Copyright 2021 Siddharth Mishra. All Rights Reserved
 * 
 */

#include "swapchain.hpp"
#include <vulkan/vulkan.hpp>

using namespace GameZero;

// create swapchain and image views for images in swapchain
void GameZero::Swapchain::Create(Surface& surface, const Device &device){
    // unique queue indices
    std::vector<uint32_t> uniqueQueueIndices;

    // get best surface details
    surface.details = SurfaceDetails(surface.surface, device.physical);

    // get minimum image count for swapchain
    uint32_t minImageCount = 0;
    // clamp the value b/w min and max image count
    if(
        (surface.details.surfaceCapabilities.maxImageCount > 0) &&
        (minImageCount > surface.details.surfaceCapabilities.maxImageCount)
    ){
        minImageCount = surface.details.surfaceCapabilities.maxImageCount;
    }

    vk::SharingMode imageSharingMode;

    // check if graphics and present queue are same
    // on most of the devices they are same but it may be possible that they are different
    if(device.graphicsQueueIndex == device.presentQueueIndex){
        // if both graphics and presentation indices are same
        // then image will be not be shared
        imageSharingMode = vk::SharingMode::eExclusive;
    }
    else{
        // however if graphics and presentation indices are different
        // then image will be shared between two queue families
        // store unique queue indices
        uniqueQueueIndices.push_back(device.graphicsQueueIndex);
        uniqueQueueIndices.push_back(device.presentQueueIndex);
        // concurrent meaning that they will be shared between given queue families
        // more precisely : queue in from a queue family
        imageSharingMode = vk::SharingMode::eConcurrent;
    }

    // swapchain create info
    vk::SwapchainCreateInfoKHR swapchainInfo(
        {}, /* flags */
        surface.surface, /* surface handle */
        minImageCount, /* minimum number of images in swapchain */
        surface.details.surfaceFormat.format, /* selected surface format for images in swapchain */
        surface.details.surfaceFormat.colorSpace, /* color space for images in swapchain */
        surface.extent, /* image extent */
        1, /* image array layers*/ 
        vk::ImageUsageFlagBits::eColorAttachment, /* usage */
        imageSharingMode, /* is image shared between multiple queue families? */
        uniqueQueueIndices.size(), uniqueQueueIndices.data(), /* unique queue indices */
        vk::SurfaceTransformFlagBitsKHR::eIdentity, /* image transformation */
        vk::CompositeAlphaFlagBitsKHR::eOpaque, /* are images opaque */
        surface.details.presentMode, /* present mode */
        false, /* clipped */
        {} /* old swapchain */
    );

    // finally create swapchain
    swapchain = device.logical.createSwapchainKHR(swapchainInfo);

    // get image handles
    auto swapchainImages = device.logical.getSwapchainImagesKHR(swapchain);

    // create image views
    for(const auto& image : swapchainImages){
        vk::ImageViewCreateInfo imageViewInfo(
            {}, /*flags*/
            image, /* image handle */
            vk::ImageViewType::e2D, /* how to interpret this image ? */
            surface.details.surfaceFormat.format, /* image format */
            {}, /* default component mapping */
            vk::ImageSubresourceRange(
                vk::ImageAspectFlagBits::eColor, /* image aspect */
                0, 1, /* base mip level and level count */
                0, 1 /* base array layer and layer count */
            )
        );

        // add a new image to the list of images
        imageViews.emplace_back(device.logical.createImageView(imageViewInfo));
    }

    // cache necessary data
    imageExtent = surface.extent;
    imageFormat = surface.details.surfaceFormat.format;
    imageCount = swapchainImages.size();
}

// destroy
void GameZero::Swapchain::Destroy(const Device &device){
    /// destroy image views
    for(const auto& imageView : imageViews) device.logical.destroyImageView(imageView);

    /// destroy swapchain
    device.logical.destroySwapchainKHR(swapchain);
}