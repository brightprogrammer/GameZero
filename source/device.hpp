#ifndef GAMEZERO_DEVICE_HPP
#define GAMEZERO_DEVICE_HPP

#include "common.hpp"

namespace GameZero{

    struct Device{
        enum class QueueType {Graphics, Present, Compute, Transfer};

        VkPhysicalDevice physical;
        VkDevice logical;

        VkQueue graphicsQueue;
        uint32_t graphicsQueueIndex;

        VkQueue presentQueue;
        uint32_t presentQueueIndex;

        /// destroy device
        void Destroy(){
            vkDestroyDevice(logical, nullptr);
        }
    };


}

#endif//GAMEZERO_DEVICE_HPP