#ifndef GAMEZERO_RENDERER_HPP
#define GAMEZERO_RENDERER_HPP

#include "common.hpp"
#include "vulkan/vulkan_core.h"
#include "window.hpp"
#include "swapchain.hpp"
#include "device.hpp"

namespace GameZero{

    struct RenderPass{
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> framebuffers;
    };

    class Renderer{
        /// initialize renderer
        void Initialize();
        /// init vulkan
        void InitVulkan();
        /// init command buffers
        void InitCommands();
        /// init renderpass
        void InitRenderPass();
        /// create framebuffers
        void InitFramebuffers();
        /// init sync structures
        void InitSyncStructures();
        /// init pipeline layouts
        void InitPipelineLayouts();
        /// init pipelines
        void InitPipelines();

        // keep track of how many renderer instances are present on system
        // if this is the last object then we need to destroy vulkan instance
        // not before that!
        static inline uint8_t objectCounter = 0;
        static inline VkInstance instance = VK_NULL_HANDLE;
        static inline VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    public:
        Window& window;
        Device device;
        Swapchain swapchain;

        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        RenderPass renderPass;

        VkFence renderFence;
        VkSemaphore renderSemaphore, presentSemaphore;

        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        /**
         * @brief Construct a new Renderer for a given Window
         * 
         * @param window to create renderer for
         */
        Renderer(Window& window);

        // main draw loop

        /**
         * @brief Destroy the Renderer object
         * 
         */
        ~Renderer();

        /// main draw loop
        void Draw();
    };
}


#endif//GAMEZERO_RENDERER_HPP