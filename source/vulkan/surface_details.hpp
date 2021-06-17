/**
 * @file surface_details.hpp
 * @author Siddharth Mishra (bshock665@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-17
 * 
 * @copyright Copyright 2021 Siddharth Mishra. All Rights Reserved
 * 
 */

 
#ifndef GAMEZERO_VULKAN_SURFACE_DETAILS_HPP
#define GAMEZERO_VULKAN_SURFACE_DETAILS_HPP

#include <vulkan/vulkan.hpp>

namespace GameZero{
    
    /// details about surface
    struct SurfaceDetails{
        SurfaceDetails() = default;

        /**
         * @brief Construct a new Surface Details object
         * 
         * @param surface 
         * @param physicalDevice 
         */
        SurfaceDetails(
            const vk::SurfaceKHR& surface,
            const vk::PhysicalDevice& physicalDevice
        );

        /**
         * @brief Construct a new Surface Details object
         * 
         * @param surfaceCapabilities 
         * @param availableSurfaceFormats 
         * @param availablePresentModes 
         */
        SurfaceDetails(
            const vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
            const std::vector<vk::SurfaceFormatKHR>& availableSurfaceFormats,
            const std::vector<vk::PresentModeKHR>& availablePresentModes
        );

        /// surface capabilities
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        /// best/selected surface format
        vk::SurfaceFormatKHR surfaceFormat;
        /// best/selected surface present mode
        vk::PresentModeKHR presentMode;
    private:
        /// select best surface details from given details
        void SelectBestSurfaceDetails(
            const std::vector<vk::SurfaceFormatKHR>& availableSurfaceFormats,
            const std::vector<vk::PresentModeKHR>& availablePresentModes
        );
    };

}

#endif//GAMEZERO_VULKAN_SURFACE_DETAILS_HPP