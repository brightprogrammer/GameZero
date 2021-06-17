/**
 * @file surface.hpp
 * @author Siddharth Mishra (bshock665@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-17
 * 
 * @copyright Copyright 2021 Siddharth Mishra. All Rights Reserved
 * 
 */

#ifndef GAMEZERO_VULKAN_SURFACE_HPP
#define GAMEZERO_VULKAN_SURFACE_HPP

#include <vulkan/vulkan.hpp>
#include "globals.hpp"
#include "surface_details.hpp"
#include "../window.hpp"

namespace GameZero{
    
    /// Vulkan Surface wrapper.
    /// Surface is created by the renderer that uses it
    struct Surface{
        /// default constructor
        Surface() = default;

        /**
         * @brief Construct a new Surface object
         * 
         * @param window : window that will use this surface
         */
        Surface(const Window& window){
            Create(window);
        }

        /**
         * @brief Create this surface for the given window
         * 
         * @param window 
         */
        void Create(const Window& window);

        /// destroy this surface
        inline void Destroy(){
            GetVulkanInstance().destroySurfaceKHR(surface);
        }

        /// vulkan surface handle
        vk::SurfaceKHR surface;
        /// surface size
        VkExtent2D extent;
        /// surface details
        SurfaceDetails details;
    };

}

#endif//GAMEZERO_VULKAN_SURFACE_HPP