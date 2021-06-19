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
#include "SDL2/SDL_error.h"
#include "SDL2/SDL_stdinc.h"
#include "instance.hpp"
#include <SDL2/SDL_vulkan.h>
#include "../utils.hpp"
#include "../window.hpp"
#include "vulkan/vulkan_core.h"

// create surface for given window
void GameZero::Surface::Create(const Window &window){
    // sdl surface creation function does not accept cpp version of VkSurfaceKHR
    // therefore we create a temp surface and then later store it in cpp surface wrapper
    // SDL_Vulkan_CreateSurface(window.window, static_cast<VkInstance>(GetVulkanInstance()), reinterpret_cast<VkSurfaceKHR*>(&surface));
    VkSurfaceKHR tmpSurface;
    ASSERT(SDL_Vulkan_CreateSurface(window.window, GetVulkanInstance(), &tmpSurface) ==  SDL_TRUE, "Failed to create vulkan surface : %s", SDL_GetError());
    LOG(DEBUG, "DONE HERE");
    surface = tmpSurface;
    LOG(DEBUG, "DONE HERE");
    if(surface) LOG(DEBUG, "Surface created successfully");
LOG(DEBUG, "DONE HERE");
    // cache necessary handles
    extent = window.GetExtent();
    LOG(DEBUG, "DONE HERE");
}
