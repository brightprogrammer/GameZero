#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>

#include <algorithm>
#include <array>

#include "instance.hpp"
#include "vulkan/vulkan_core.h"
#include "../app_state.hpp"
#include "../utils.hpp"
#include "../window.hpp"

using namespace GameZero;

// vulkan instance
static vk::Instance instance;

// is vulkan instance created?
static bool isInstanceCreated = false;

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
    std::vector<const char*> layerNames;
    if(GetApplicationState()->enableValidation) layerNames.push_back(VK_LAYER_KHRONOS_VALIDATION_LAYER_NAME);
    
    // get number of surface extension names 
    uint32_t extensionCount = 0;
    ASSERT(SDL_Vulkan_GetInstanceExtensions(GetMainWindow()->window, &extensionCount, nullptr) == SDL_TRUE, "Failed to get surface extensions for instance creation : %s", SDL_GetError());
    ASSERT(extensionCount > 0, "No surface extensions found on host")
    
    // get extension names
    std::vector<const char*> extensionNames(extensionCount);
    ASSERT(SDL_Vulkan_GetInstanceExtensions(GetMainWindow()->window, &extensionCount, extensionNames.data()) == SDL_TRUE, "Failed to get surface extensions for instance creation : %s", SDL_GetError());
    
    // enable debug utils extension name
    if(GetApplicationState()->enableValidation) extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    // instance create info
    vk::InstanceCreateInfo icInfo(
        {},
        &appInfo, /* application info */
        layerNames.size(), layerNames.data(), /* enabled layers */
        extensionNames.size(), extensionNames.data() /* enabled extensions */
    );

    // create instance
    instance = vk::createInstance(icInfo);
    // add to destruction
    GetDestructionQueue()->PushFunction([=](){
        instance.destroy();
    });
}

// return created instance
const vk::Instance& GameZero::GetVulkanInstance(){
    if(!isInstanceCreated){
        CreateInstance();
        isInstanceCreated = true;
    }

    return instance;
}