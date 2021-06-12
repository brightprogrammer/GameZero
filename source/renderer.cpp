#include "renderer.hpp"
#include "utils/assert.hpp"
#include "utils/utils.hpp"
#include "utils/vulkan_initializers.hpp"
#include "vulkan/vulkan_core.h"
#include "shader.hpp"

GameZero::Renderer::Renderer(GameZero::Window& window) : window(window){
    // generate log
    LOG(INFO, "New Renderer created [ Renderer Window Name : %s ]", window.title);

    // initialize vulkan
    Initialize();
    
    // keep track of renderer instances
    objectCounter++;
}

GameZero::Renderer::~Renderer(){
    // wait for all device operations to complete
    vkDeviceWaitIdle(device.logical);

    // keep track of renderer instances
    objectCounter--;

    // destory pipeline layout
    vkDestroyPipelineLayout(device.logical, pipelineLayout, nullptr);

    // destroy pipeline
    vkDestroyPipeline(device.logical, pipeline, nullptr);

    // destory sync structures
    vkDestroyFence(device.logical, renderFence, nullptr);
    vkDestroySemaphore(device.logical, renderSemaphore, nullptr);
    vkDestroySemaphore(device.logical, presentSemaphore, nullptr);

    // destroy command pool
    // this automatically destroys allocated command buffers
    vkDestroyCommandPool(device.logical, commandPool, nullptr);
    LOG(INFO, "Vulkan Command Pool destroyed [ Renderer Window Name :  %s ]", window.title)

    // destroy framebuffers
    for(const auto& fb : renderPass.framebuffers) vkDestroyFramebuffer(device.logical, fb, nullptr);
    LOG(INFO, "Vulkan Framebuffers destroyed [ Renderer Window Name :  %s ]", window.title)

    // destory renderpass
    vkDestroyRenderPass(device.logical, renderPass.renderPass, nullptr);
    LOG(INFO, "Vulkan Render Pass destroyed [ Renderer Window Name :  %s ]", window.title)

    for(const auto& imageView : swapchain.imageViews) vkDestroyImageView(device.logical, imageView, nullptr);
    vkDestroySwapchainKHR(device.logical, swapchain.swapchain, nullptr);
    LOG(INFO, "Vulkan Swapchain destroyed [ Renderer Window Name :  %s ]", window.title)
    
    vkDestroyDevice(device.logical, nullptr);
    LOG(INFO, "Vulkan Device destroyed [ Renderer Window Name :  %s ]", window.title)
    
    vkDestroySurfaceKHR(instance, window.surface, nullptr);
    LOG(INFO, "Vulkan Surface destroyed [ Renderer Window Name :  %s ]", window.title)

    // if this is the last renderer, then destroy vulkan instance
    if(objectCounter == 0){
        vkb::destroy_debug_utils_messenger(instance, debugMessenger);
        LOG(INFO, "Vulkan Debug Messenger destroyed");

        vkDestroyInstance(instance, nullptr);
        LOG(INFO, "Vulkan Instance destroyed");
    }

    LOG(INFO, "Renderer destroyed [ Renderer Window Name :  %s ]", window.title)
}

void GameZero::Renderer::Initialize(){
    InitVulkan();
    InitCommands();
    InitRenderPass();
    InitFramebuffers();
    InitSyncStructures();
    InitPipelineLayouts();
    InitPipelines();
}

// initialize renderer
void GameZero::Renderer::InitVulkan(){
    // create instance ------------------------------------------
    // WARNING : Might create problem when working with multiple renderers
    // next instance of GameZero::Renderer will have an empty vkbInstance!
    // that will fail device selection and further steps!
    vkb::Instance vkbInstance;
    if(instance == VK_NULL_HANDLE){
        vkb::InstanceBuilder instBuilder;
        auto instRet = instBuilder  .set_app_name(GameZero::GameZeroApplicationName)
                                    .request_validation_layers(true)
                                    .require_api_version(1, 1, 0)
                                    .use_default_debug_messenger()
                                    .build();
        vkbInstance = instRet.value();

        instance = vkbInstance.instance;
        debugMessenger = vkbInstance.debug_messenger;
        LOG(INFO, "Vulkan Instance created");
    }
    
    // create surface -------------------------------------------
    window.CreateSurface(instance);
    LOG(INFO, "Vulkan Surface created [ Renderer Window Name :  %s ]", window.title);

	// create device --------------------------------------------
	//We want a GPU that can write to the SDL surface and supports Vulkan 1.1
	vkb::PhysicalDeviceSelector selector{ vkbInstance };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(window.surface)
        // .add_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
		.select()
		.value();

	//create the final Vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();
    LOG(INFO, "Vulkan Device created [ Renderer Window Name :  %s ]", window.title);

	// Get the VkDevice handle used in the rest of a Vulkan application
	device.physical = physicalDevice.physical_device;
	device.logical = vkbDevice.device;

    // get device queues and queue indices
    device.presentQueue = vkbDevice.get_queue(vkb::QueueType::present).value();
    device.presentQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::present).value();
    
    device.graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    device.graphicsQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // create swapchain ------------------------------------------
    vkb::SwapchainBuilder swapchainBuilder {device.physical, device.logical, window.surface};
    vkb::Swapchain vkbSwapchain = swapchainBuilder.use_default_format_selection()
		                            /* vsync */ .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		                                        .set_desired_extent(window.size.x, window.size.y)
		                                        .build()
		                                        .value();
    // fill in swapchain details
    swapchain.swapchain = vkbSwapchain.swapchain;
    swapchain.images = vkbSwapchain.get_images().value();
    swapchain.imageViews = vkbSwapchain.get_image_views().value();
    swapchain.imageFormat = vkbSwapchain.image_format;
    swapchain.imageExtent = window.GetExtent();
    swapchain.imageCount = swapchain.images.size();
    LOG(INFO, "Vulkan Swapchain created [ Renderer Window Name :  %s ]", window.title);
}

void GameZero::Renderer::InitCommands(){
    // create command pool for graphics queue
    VkCommandPoolCreateInfo cmdPoolInfo = VulkanInitialize<VkCommandPoolCreateInfo>();
    cmdPoolInfo.queueFamilyIndex = device.graphicsQueueIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CHECK_VK_RESULT(vkCreateCommandPool(device.logical, &cmdPoolInfo, nullptr, &commandPool), "Command Pool creation failed");

    // allocate 1 command buffer
    VkCommandBufferAllocateInfo cmdBuffAllocInfo = VulkanInitialize<VkCommandBufferAllocateInfo>();
    cmdBuffAllocInfo.commandPool = commandPool;
    cmdBuffAllocInfo.commandBufferCount = 1;
    cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CHECK_VK_RESULT(vkAllocateCommandBuffers(device.logical, &cmdBuffAllocInfo, &commandBuffer), "Command Buffer allocation failed");
}

void GameZero::Renderer::InitRenderPass(){
    VkAttachmentDescription colorAttachment = {};
    // no multisampling
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // attachment format is same as swapchain image format
    colorAttachment.format = swapchain.imageFormat;
    // we dont know what initial layout is (before renderpass starts)
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // we want image to be presentable to screen finally (after renderpass completion)
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // we want to clear image on load
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // we want to save image finally
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // we dont care about stencil
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // make an attachment reference for subpass
    VkAttachmentReference colorAttachmentRef = {};
    // first attachment in array of attachments in renderpass
    colorAttachmentRef.attachment = 0;
    // image must be optimal for color attachment on load
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // describe subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // create renderpass
    VkRenderPassCreateInfo renderPassInfo = VulkanInitialize<VkRenderPassCreateInfo>();
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    CHECK_VK_RESULT(vkCreateRenderPass(device.logical, &renderPassInfo, nullptr, &renderPass.renderPass), "Failed to create Vulkan Render Pass");
}

void GameZero::Renderer::InitFramebuffers(){
    VkFramebufferCreateInfo fbInfo = VulkanInitialize<VkFramebufferCreateInfo>();
    fbInfo.renderPass = renderPass.renderPass;
    fbInfo.attachmentCount = 1; // number of images attachments in renderpass
    fbInfo.width = swapchain.imageExtent.width;
    fbInfo.height = swapchain.imageExtent.height;
    fbInfo.layers = 1;

    renderPass.framebuffers.resize(swapchain.imageCount);
    for(size_t fbID = 0; fbID < swapchain.imageCount; fbID++){
        fbInfo.pAttachments = &swapchain.imageViews[fbID];
        CHECK_VK_RESULT(vkCreateFramebuffer(device.logical, &fbInfo, nullptr, &renderPass.framebuffers[fbID]), "Failed to create Vulkan Framebuffer");
    }
}

void GameZero::Renderer::InitSyncStructures(){
    // create fence
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    CHECK_VK_RESULT(vkCreateFence(device.logical, &fenceInfo, nullptr, &renderFence), "Failed to create Vulkan Fence");

    // create semaphores
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    CHECK_VK_RESULT(vkCreateSemaphore(device.logical, &semaphoreInfo, nullptr, &renderSemaphore), "Failed to create Vulkan Semaphore");
    CHECK_VK_RESULT(vkCreateSemaphore(device.logical, &semaphoreInfo, nullptr, &presentSemaphore), "Failed to create Vulkan Semaphore");
}

void GameZero::Renderer::Draw(){
    // wait until gpu singals us that it is done rendering
    CHECK_VK_RESULT(vkWaitForFences(device.logical, 1, &renderFence, VK_TRUE, 1e9), "Waiting for Vulkan ( Render ) Fence failed");
    // then reset render fence
    CHECK_VK_RESULT(vkResetFences(device.logical, 1, &renderFence), "Reset Vulkan ( Render ) Fence failed");

    // get next image to render to from swapchain
    // swapchain will signal present semaphore when it is done presenting it
    uint32_t nextImageIndex;
    CHECK_VK_RESULT(vkAcquireNextImageKHR(device.logical, swapchain.swapchain, 1e9, presentSemaphore, VK_NULL_HANDLE, &nextImageIndex), "Failed to get next image from Swapchain");

    // reset command buffer for use
    CHECK_VK_RESULT(vkResetCommandBuffer(commandBuffer, 0), "Failed to reset Command Buffer");

    // rename it for shorter name
    VkCommandBuffer cmd = commandBuffer;

    // begin command buffer
    VkCommandBufferBeginInfo cmdBeginInfo = VulkanInitialize<VkCommandBufferBeginInfo>();
    cmdBeginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CHECK_VK_RESULT(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "Command Buffer recording failed");

    // clear value for color attachment on renderpass begin
    VkClearValue clearValue = {};
    clearValue.color = {{0.01f, 0.01f, 0.01f, 1.0f}};
    
    // begin renderpass
    VkRenderPassBeginInfo rpBeginInfo = VulkanInitialize<VkRenderPassBeginInfo>();
    rpBeginInfo.clearValueCount = 1;
    rpBeginInfo.renderPass = renderPass.renderPass;
    rpBeginInfo.pClearValues = &clearValue;
    rpBeginInfo.framebuffer = renderPass.framebuffers[nextImageIndex];
    rpBeginInfo.renderArea = VkRect2D{/*offset*/{0, 0}, /*extent*/{window.GetExtent()}};
    
    // begin renderpass
    vkCmdBeginRenderPass(cmd, &rpBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(cmd, 3, 1, 0, 0);

    // end renderpass
    vkCmdEndRenderPass(cmd);

    // end command buffer recording
    vkEndCommandBuffer(cmd);

    // prepare to submit command buffer
    VkSubmitInfo submitInfo = VulkanInitialize<VkSubmitInfo>();
    
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &presentSemaphore;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderSemaphore;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitStage;

    // submit command buffer to graphisc queue
    // render fence will now block cpu when cpu issues a vkWaitForFences until unless rendering is done
    CHECK_VK_RESULT(vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, renderFence), "Failed to submit Command Buffers to Graphics Queue");

    // after rendering we need to show it on screen
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain.swapchain;

    // this will ask vulkan to wait before rendering is complete
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderSemaphore;

    presentInfo.pImageIndices = &nextImageIndex;
    CHECK_VK_RESULT(vkQueuePresentKHR(device.presentQueue, &presentInfo), "Failed to present rendered image to screen");
}

// inpit pipeline layouts
void GameZero::Renderer::InitPipelineLayouts(){
    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pushConstantRangeCount = 0;
    
    CHECK_VK_RESULT(vkCreatePipelineLayout(device.logical, &layoutInfo, nullptr, &pipelineLayout), "Failed to create Pipeline Layout");
}

// init pipelines
void GameZero::Renderer::InitPipelines(){
    // create shader modules
    VkShaderModule vertShader = LoadShaderModule(device.logical, "shaders/vert.spv");
    ASSERT(vertShader != VK_NULL_HANDLE, "Failed to load shader module");
    VkShaderModule fragShader = LoadShaderModule(device.logical, "shaders/frag.spv");
    ASSERT(fragShader != VK_NULL_HANDLE, "Failed to load shader module");

    // vertex shader stage
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.module = vertShader;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.pName = "main";

    // fragment shader stage
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.module = fragShader;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        fragShaderStageInfo, vertShaderStageInfo
    };

    // vertex input state
    VkPipelineVertexInputStateCreateInfo vertexInput = {};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 0;
    vertexInput.vertexAttributeDescriptionCount = 0;

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // rasterization
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.f;
    rasterizer.depthBiasClamp = 0.f;
    rasterizer.depthBiasSlopeFactor = 0.f;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.lineWidth = 1.f;

    // multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // color blend attachment
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =  VK_COLOR_COMPONENT_A_BIT |
                                    VK_COLOR_COMPONENT_G_BIT |
                                    VK_COLOR_COMPONENT_B_BIT |
                                    VK_COLOR_COMPONENT_R_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    // color blend sate
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;

    // viewport
    VkViewport viewport = {};
    viewport.width = static_cast<float>(window.size.x);
    viewport.height = static_cast<float>(window.size.y);
    viewport.x = 0;
    viewport.y = 0;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    // scissor
    VkRect2D scissor = {{0, 0}, window.GetExtent()};

    // viewport state
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {};
    graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    graphicsPipelineInfo.stageCount = 2;
    graphicsPipelineInfo.pStages = shaderStages;

    graphicsPipelineInfo.pVertexInputState = &vertexInput;
    graphicsPipelineInfo.pInputAssemblyState = &inputAssembly;

    graphicsPipelineInfo.pRasterizationState = &rasterizer;
    graphicsPipelineInfo.pMultisampleState = &multisampling;

    graphicsPipelineInfo.pColorBlendState = &colorBlending;
    graphicsPipelineInfo.pViewportState = &viewportState;

    graphicsPipelineInfo.layout = pipelineLayout;
    graphicsPipelineInfo.renderPass =  renderPass.renderPass;
    graphicsPipelineInfo.subpass = 0;

    CHECK_VK_RESULT(vkCreateGraphicsPipelines(device.logical, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &pipeline), "Failed to create Graphics Pipeline");

    // we dont need shader modules anymore
    vkDestroyShaderModule(device.logical, vertShader, nullptr);
    vkDestroyShaderModule(device.logical, fragShader, nullptr);
}