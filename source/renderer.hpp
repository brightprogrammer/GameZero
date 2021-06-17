#ifndef GAMEZERO_RENDERER_HPP
#define GAMEZERO_RENDERER_HPP

#include "common.hpp"
#include "vulkan/vulkan_core.h"
#include "window.hpp"
#include "utils.hpp"
#include "vulkan/device.hpp"
#include "vulkan/swapchain.hpp"
#include "mesh.hpp"
#include <string>
#include <unordered_map>
#include <functional>
#include "texture.hpp"
#include <deque>

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
        /// load images
        void LoadImages();

        // keep track of how many renderer instances are present on system
        // if this is the last object then we need to destroy vulkan instance
        // not before that!
        static inline uint8_t objectCounter = 0;
        static inline VkInstance instance = VK_NULL_HANDLE;
        static inline VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        
        /// execte deletors
        void FlushDeletors();
    public:
        /// window that this renderer renders to
        Window& window;

        /// device used by the renderer
        Device device;

        /// swapchain of the window
        Swapchain swapchain;

        /// default renderpass
        RenderPass renderPass;

        /// current frame number
        size_t frameNumber = 0;

        /// frame data for multiple buffering
        /// while gpu renders to one frame, renderer will prepare another frame to render to
        FrameData frames[FrameOverlapCount];

        /// contains destruction functions
        std::deque<std::function<void()>> deletors;

        /// default graphics pipeline
        VkPipeline pipeline;
        /// default graphics pipeline layout
        VkPipelineLayout pipelineLayout;

        /// allocator handle for vma
        VmaAllocator allocator;

        /// simple mesh
        Mesh mesh;

        /// depth image : manually allocated
        AllocatedImage depthImage;
        /// image view for depth image 
        VkImageView depthImageView;

        /// material map with their unique name
        std::unordered_map<std::string, Material> materials;
        /// model/mesh map with their unique name
        std::unordered_map<std::string, Mesh> meshes;

        /// renderable objects made from loaded meshes and materials
        std::vector<RenderObject> renderables;

        /// global descriptor set layout for sending uniform data
        VkDescriptorSetLayout descriptorSetLayout;
        /// global descriptor pool for allocation of uniforms
        VkDescriptorPool descriptorPool;

        /// camera data contains camera matrices : model, view, projection
        /// model is updated by renderer but view and projection are updated in main.cpp
        GPUCameraData cameraData;

        /// gpu immediate upload context
        /// used when uploading data to gpu using staging buffers
        UploadContext uploadContext;

        /// descriptor set layout for uploading a single texture at a time
        VkDescriptorSetLayout singleTextureSetLayout;

        /// map of textures with their unique name
        std::unordered_map<std::string, Texture> textures;

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

        /// add a new function to deletion queue
        void PushFunction(std::function<void()>&& function){
            deletors.emplace_back(function);
        }

        /// main draw loop
        void Draw();

        /// initialize scene
        void InitScene();

        /**
         * @brief draw count number of given objects
         * 
         * @param cmd : command buffer to record draw commands to
         * @param firstObject : first object in an array
         * @param count : total number of objects to draw
         */
        void DrawObjects(VkCommandBuffer cmd, RenderObject* firstObject, uint32_t count);
    
        /**
         * @brief Immediately submit a command buffer without any extra sync
         * 
         * @param function : contains vkCmdXXXX(...)
         */
        void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

        /**
         * @brief Upload a given mest to gpu
         * 
         * @param useStaging : use staging buffers for transferring data
         */
        void UploadMeshToGPU(Mesh* mesh, bool useStaging = true);
    };
}


#endif//GAMEZERO_RENDERER_HPP