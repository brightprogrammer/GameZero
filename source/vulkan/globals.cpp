#include "globals.hpp"
#include "SDL2/SDL_vulkan.h"
#include "../app_state.hpp"
#include <vulkan/vulkan.hpp>
#include "vulkan/vulkan_core.h"
#include <array>
#include "../window.hpp"
#include <SDL2/SDL_vulkan.h>
#include "../utils.hpp"

using namespace GameZero;

// vulkan instance
static vk::Instance instance;

// is vulkan instance created?
static bool isInstanceCreated  = false;

/* instance creation automatically creates a surface for the main window */

// create vulkan instance
void CreateInstance(){
    // application info
    vk::ApplicationInfo appInfo(
        GetApplicationName(),
        GetApplicationVersion(),
        "GameZero", /* engine name */
        0 /* engine version */,
        VK_API_VERSION_1_2
    );

    // request validation layers
    std::vector<const char*> layerNames = {
        VK_LAYER_KHRONOS_VALIDATION_LAYER_NAME
    };
    
    // get number of surface extension names 
    uint32_t count = 0;
    SDL_Vulkan_GetInstanceExtensions(GetMainWindow()->window, &count, nullptr); 

    // extension names
    std::vector<const char*> extensionNames(count);

    // get surface extension names
    SDL_Vulkan_GetInstanceExtensions(GetMainWindow()->window, &count, extensionNames.data());

    // instance create info
    vk::InstanceCreateInfo icInfo(
        {}, /* flags */
        &appInfo,
        layerNames.size(),
        layerNames.data(),
        extensionNames.size(),
        extensionNames.data()
    );

    // create instance
    instance = vk::createInstance(icInfo);
}

// return created instance
const vk::Instance& GameZero::GetVulkanInstance(){
    if(!isInstanceCreated) CreateInstance();
    return instance;
}