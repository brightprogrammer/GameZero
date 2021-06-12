#ifndef GAMEZERO_VULKAN_INITIALIZERS_HPP
#define GAMEZERO_VULKAN_INITIALIZERS_HPP

#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>

namespace GameZero{
    template<typename T>
    inline T VulkanInitialize();

    // initialize command pool create info
    template<>
    inline VkCommandPoolCreateInfo VulkanInitialize<VkCommandPoolCreateInfo>(){
        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        return info;
    }

    // command buffer allocate info
    template<>
    inline VkCommandBufferAllocateInfo VulkanInitialize<VkCommandBufferAllocateInfo>(){
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        return info;
    }

    // renderpass create info
    template<>
    inline VkRenderPassCreateInfo VulkanInitialize<VkRenderPassCreateInfo>(){
        VkRenderPassCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        return info;
    }

    // framebuffer create info
    template<>
    inline VkFramebufferCreateInfo VulkanInitialize<VkFramebufferCreateInfo>(){
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        return info;
    }

    // fence create info
    template<>
    inline VkFenceCreateInfo VulkanInitialize<VkFenceCreateInfo>(){
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        return info;
    }

    // semaphore create info
    template<>
    inline VkSemaphoreCreateInfo VulkanInitialize<VkSemaphoreCreateInfo>(){
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        return info;
    }

    template<>
    inline VkCommandBufferBeginInfo VulkanInitialize<VkCommandBufferBeginInfo>(){
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        return info;
    }

    template<>
    inline VkRenderPassBeginInfo VulkanInitialize<VkRenderPassBeginInfo>(){
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        return info;
    }

    template<>
    inline VkSubmitInfo VulkanInitialize<VkSubmitInfo>(){
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        return info;
    }

    template<>
    inline VkPresentInfoKHR VulkanInitialize<VkPresentInfoKHR>(){
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        return info;
    }
}

#endif//GAMEZERO_VULKAN_INITIALIZERS_HPP