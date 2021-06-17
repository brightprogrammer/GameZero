#include "device.hpp"
#include "globals.hpp"
#include "../utils.hpp"
#include "vk_mem_alloc.hpp"
#include "vulkan/vulkan.hpp"
#include "../window.hpp"
#include "../app_state.hpp"
#include <set>

using namespace GameZero;

// get all device extensions
std::vector<const char*> GetPhysicalDeviceExtensionNames(const vk::PhysicalDevice &physicalDevice){
    // get device extensions
    auto extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();

    // create a vector and add the names to it
    std::vector<const char*> extensionNames(extensionProperties.size());
    for(uint i=0; i<extensionProperties.size(); i++){
        extensionNames[i] = extensionProperties[i].extensionName;
    }

    // return the names
    return extensionNames;
}

// rate a given physical device
uint32_t RatePhysicalDevice(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface){
    // device score
    uint32_t score = 0;

    // get physical device properties
    auto properties = physicalDevice.getProperties();
    // get physical device memory properties
    auto memoryProperties = physicalDevice.getMemoryProperties();
    // get physical device features
    auto features = physicalDevice.getFeatures();
    
    score += properties.limits.maxColorAttachments * 100;
    score += properties.limits.maxDescriptorSetInputAttachments * 100;
    score += properties.limits.maxImageDimension2D * 1000;
    score += properties.limits.maxImageArrayLayers * 10;
    score += properties.limits.maxViewports * 500;

    score += memoryProperties.memoryHeapCount * 1000;

    if(features.multiViewport == VK_TRUE)
        score += 500;

    // dedicated gpu is much better than an integrated one
    if(properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        score += 10000;
    else if(properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
        score += 5000;

    // get queue properties
    std::vector<vk::QueueFamilyProperties> queues = physicalDevice.getQueueFamilyProperties();
    
    // if device doesn't provide graphics queue then set score to 0
    for(const auto& queue : queues){
        // since graphics is most important queue
        if(queue.queueFlags & vk::QueueFlagBits::eGraphics){
            score += 100000; // 1L or 100 Th
        }
        // since compute is less important that graphics
        if(queue.queueFlags & vk::QueueFlagBits::eCompute){
            score += 50000; // 50 Th
        }
    }

    // check surface presentation support
    {
        uint i = 0;
        vk::Bool32 presentationSupported;
        for(const auto& queue : queues){
            presentationSupported = physicalDevice.getSurfaceSupportKHR(i, surface);
            if(presentationSupported) break; else i++;
        }

        // it may happen that we reached the end and we never got the queue
        // in that case we have to check it differently
        if(i+1 == queues.size() && !presentationSupported) score = 0;
        else score += 110000;
    }

    // get device extension names
    std::vector<const char*> extensions = GetPhysicalDeviceExtensionNames(physicalDevice);

    // check if swapchain extension is present or not
    bool swapchainExtensionAvailable = false;
    for(const auto& extension : extensions){
        if(strcmp(extension, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            swapchainExtensionAvailable = true;
    }

    // no swapchain means no multi-image rendering
    if(!swapchainExtensionAvailable)
        score = 0;

    return score;
}


// get queue family index for given physical device
int32_t GetPhysicalDeviceQueueFamilyIndex(const vk::PhysicalDevice &physicalDevice, const vk::QueueFlagBits& queueFamily){
    // get queue family properties
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
        
    // check for queue index
    // if no such queue is found, -1 is returned
    int32_t queueIdx = -1;
    for(size_t i = 0; i<queueFamilyProperties.size();  i++){
        if(queueFamilyProperties[i].queueFlags & queueFamily) queueIdx = i;
    }

    // return queue index
    return queueIdx;
}

// get queue family index that supports surface presentation
int32_t GetPhysicalDeviceSurfaceSupportQueueIndex(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface){
    // get queue family properties
    std::vector<vk::QueueFamilyProperties> queues = physicalDevice.getQueueFamilyProperties();

    // check which queue supports presentation
    for(uint32_t queueIdx = 0; queueIdx < queues.size(); queueIdx++){
        VkBool32 presentationSupported = physicalDevice.getSurfaceSupportKHR(queueIdx, surface);
        if(presentationSupported) return queueIdx;
    }

    // if not found
    return -1;
}

// select best physical device
void GameZero::Device::SelectBestPhysicalDevice(){
    // device list
    auto physicalDeviceList = GetVulkanInstance().enumeratePhysicalDevices();
            
    // start with score = 0
    // if we find a score greater than this score then
    // select that as score and that as selected physical devie
    uint32_t score = 0;

    // sort physical device
    for(const auto& gpu : physicalDeviceList){
        uint32_t maxScore = RatePhysicalDevice(gpu, GetMainWindow()->surface);
        if(score < maxScore){
            score = maxScore;
            physical = gpu; // selected device
        }
    }
}

// create logical device
void GameZero::Device::CreateDevice(){
    // get graphics queue index
    int32_t tmp = GetPhysicalDeviceQueueFamilyIndex(physical, vk::QueueFlagBits::eGraphics);
    if(tmp == -1) {
        ASSERT(false, "Failed to find graphics queue family on selected Physical Device");
    } else graphicsQueueIndex = static_cast<uint32_t>(tmp);

    // get presentation queue index
    tmp = GetPhysicalDeviceSurfaceSupportQueueIndex(physical, GetMainWindow()->surface);
    if(tmp == -1) {
        ASSERT(false, "Failed to find surface presentation supporting queue family on selected Physical Device");
    } else presentQueueIndex = static_cast<uint32_t>(tmp);

    // unique queue indices
    std::set<uint32_t> uniqueQueueIndices = {graphicsQueueIndex, presentQueueIndex};

    // queue priorities : since we are creating only one queue, only one value in vector
    std::vector<float> queuePriorities = {float{1.f}};

    // base queue create info struct
    // this is common for all queues that will be created
    vk::DeviceQueueCreateInfo deviceQueueInfo({}, 0, queuePriorities.size(), queuePriorities.data());

    // create queue create infos for devices with unique indices
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for(const auto& queueIdx : uniqueQueueIndices){
        deviceQueueInfo.queueFamilyIndex = queueIdx;
        queueCreateInfos.push_back(deviceQueueInfo);
    }

    // swapchain extension is already checked for a device
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
               
    // device create info
    vk::DeviceCreateInfo deviceCreateInfo(
        {}, /* flags */
        queueCreateInfos.size(), queueCreateInfos.data(),
        0, nullptr, /* layers */
        deviceExtensions.size(), deviceExtensions.data()
    );
    
    // create device
    logical = physical.createDevice(deviceCreateInfo);
   
    // get graphics queue handle
    graphicsQueue = logical.getQueue(graphicsQueueIndex, 0);

    // get present queue handle
    if(graphicsQueueIndex == presentQueueIndex) presentQueue = graphicsQueue;
    else presentQueue = logical.getQueue(presentQueueIndex, 0);
}

// create device memory allocator
void GameZero::Device::CreateAllocator(){
    vma::AllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.instance = GetVulkanInstance();
    allocatorInfo.physicalDevice = physical;
    allocatorInfo.device = logical;

    // create allocator
    allocator = vma::createAllocator(allocatorInfo);
}

// select best physical device
GameZero::Device::Device(){
    SelectBestPhysicalDevice();
    CreateDevice();
    CreateAllocator();
}