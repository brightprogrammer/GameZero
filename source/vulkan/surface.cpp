/**
 * @file surface.cpp
 * @author Siddharth Mishra (bshock665@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-17
 * 
 * @copyright Copyright 2021 Siddharth Mishra. All Rights Reserved
 * 
 */

#include "surface.hpp"
#include "globals.hpp"
#include <SDL2/SDL_vulkan.h>
#include "../utils.hpp"
#include "../window.hpp"

// create surface for given window
void GameZero::Surface::Create(const Window &window){
    // sdl surface creation function does not accept cpp version of VkSurfaceKHR
    // therefore we create a temp surface and then later store it in cpp surface wrapper
    VkSurfaceKHR tempSurface;
    SDL_Vulkan_CreateSurface(window.window, GetVulkanInstance(), &tempSurface);
    surface = tempSurface;

    // cache necessary handles
    extent = window.GetExtent();
}
