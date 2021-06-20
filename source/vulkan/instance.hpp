/**
 * @file globals.hpp
 * @author Siddharth Mishra (bshock665@gmail.com)
 * @brief  This header file is responsible for handling global objects
 *         objects that almost all files might need access to in future
 *         for eg : a vulkan instance is a handle that needs to be created
 *         only once but is needed by most of the vulkan functions
 *         keeping this wrapped up in a singleton might make it tideous to work with.
 * @version 0.1
 * @date 2021-06-16
 * 
 * @copyright Copyright (c) 2021 Siddharth Mishra. All Rights Reserved.
 * 
 */


#ifndef GAMEZERO_GLOBALS_HPP
#define GAMEZERO_GLOBALS_HPP

#include <vulkan/vulkan.hpp>
#include "../utils.hpp"

namespace GameZero{

    /* vulkan validation layer names */
    #define VK_LAYER_KHRONOS_VALIDATION_LAYER_NAME        "VK_LAYER_KHRONOS_validation"
    #define VK_LAYER_LUNARG_API_DUMP_LAYER_NAME           "VK_LAYER_LUNARG_api_dump"       
    #define VK_LAYER_LUNARG_DEVICE_SIMULATION_LAYER_NAME  "VK_LAYER_LUNARG_device_simulation" 
    #define VK_LAYER_LUNARG_MONITOR_LAYER_NAME            "VK_LAYER_LUNARG_monitor"
    #define VK_LAYER_LUNARG_SCREENSHOT_LAYER_NAME         "VK_LAYER_LUNARG_screenshot"

    struct Instance : Singleton<Instance>{
	/// instance constructor
 	Instance();
	
	/// instance destructor
	~Instance(){
	    instance.destroy();
	}

	/// vulkan instance handle
	vk::Instance instance;
    };

    /// get global vulkan instance
    inline const vk::Instance& GetVulkanInstance(){
        return Instance::Get()->instance;
    }
}

#endif//GAMEZERO_GLOBALS_HPP
