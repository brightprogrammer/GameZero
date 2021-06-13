#ifndef GAMEZERO_RENDERER_HPP
#define GAMEZERO_RENDERER_HPP

#include "common.hpp"
#include "vulkan/vulkan_core.h"
#include "window.hpp"
#include "utils/utils.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "mesh.hpp"
#include <string>
#include <unordered_map>

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
        // init depth image
        void InitDepthImage();
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
        /// init mesh
        void InitMesh();
        /// Init Descriptors
        void InitDescriptors();

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

        RenderPass renderPass;

        size_t frameNumber = 0;
        FrameData frames[FrameOverlapCount];

        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        VmaAllocator allocator;

        Mesh mesh;

        AllocatedImage depthImage;
        VkImageView depthImageView;

        std::unordered_map<std::string, Material> materials;
        std::unordered_map<std::string, Mesh> meshes;

        std::vector<RenderObject> renderables;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;

        GPUCameraData cameraData;

        /// create material and add it to material map
        Material* CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

        /// get material using name. returns nullptr if not found
        Material* GetMaterial(const std::string& name);

        /// get mesh using given name, return nullptr if not found
        Mesh* GetMesh(const std::string& name);

        // get current frame
        FrameData& GetCurrentFrame(){
            return frames[frameNumber % FrameOverlapCount];
        }


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

        /// initialize scene
        void InitScene();

        /**
         * @brief draw count number of given objects
         * 
         * @param cmd 
         * @param firstObject 
         * @param count 
         */
        void DrawObjects(VkCommandBuffer cmd, RenderObject* firstObject, uint32_t count);
    };
}


#endif//GAMEZERO_RENDERER_HPP