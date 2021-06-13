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

    for(const auto& frame : frames){
        // destory sync structures
        vkDestroyFence(device.logical, frame.renderFence, nullptr);
        vkDestroySemaphore(device.logical, frame.renderSemaphore, nullptr);
        vkDestroySemaphore(device.logical, frame.presentSemaphore, nullptr);

        // destroy command pool
        // this automatically destroys allocated command buffers
        vkDestroyCommandPool(device.logical, frame.commandPool, nullptr);
        LOG(INFO, "Vulkan Command Pool destroyed [ Renderer Window Name : %s ]", window.title)

        vmaDestroyBuffer(allocator, frame.cameraBuffer.buffer, frame.cameraBuffer.allocation);
    }

    // destroy descriptor set layout
    vkDestroyDescriptorSetLayout(device.logical, descriptorSetLayout, nullptr);
    LOG(INFO, "Descriptor Set Layout destroyed");

    // destroy descriptor pool
    vkDestroyDescriptorPool(device.logical, descriptorPool, nullptr);
    LOG(INFO, "Descriptor Pool destroyed");

    // destroy framebuffers
    for(const auto& fb : renderPass.framebuffers) vkDestroyFramebuffer(device.logical, fb, nullptr);
    LOG(INFO, "Vulkan Framebuffers destroyed [ Renderer Window Name : %s ]", window.title)

    // destory renderpass
    vkDestroyRenderPass(device.logical, renderPass.renderPass, nullptr);
    LOG(INFO, "Vulkan Render Pass destroyed [ Renderer Window Name : %s ]", window.title)

    for(const auto& imageView : swapchain.imageViews) vkDestroyImageView(device.logical, imageView, nullptr);
    vkDestroySwapchainKHR(device.logical, swapchain.swapchain, nullptr);
    LOG(INFO, "Vulkan Swapchain destroyed [ Renderer Window Name : %s ]", window.title)
    
    // destroy depth image
    vkDestroyImageView(device.logical, depthImageView, nullptr);
    vmaDestroyImage(allocator, depthImage.image, depthImage.allocation);

    // destroy mesh before destroying logical device
    mesh.DestroyMesh();

    // destroy vulkan memory allocator before destroying logical device
    vmaDestroyAllocator(allocator);
    LOG(INFO, "Memory Allocator Destroyed [ Renderer Window Name : %s]", window.title);

    vkDestroyDevice(device.logical, nullptr);
    LOG(INFO, "Vulkan Device destroyed [ Renderer Window Name : %s ]", window.title)
    
    vkDestroySurfaceKHR(instance, window.surface, nullptr);
    LOG(INFO, "Vulkan Surface destroyed [ Renderer Window Name : %s ]", window.title)

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
    InitDepthImage();
    InitRenderPass();
    InitFramebuffers();
    InitSyncStructures();
    InitDescriptors();
    InitPipelineLayouts();
    InitPipelines();
    InitMesh();
    InitScene();
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
    LOG(INFO, "Vulkan Surface created [ Renderer Window Name : %s ]", window.title);

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
    LOG(INFO, "Vulkan Device created [ Renderer Window Name : %s ]", window.title);

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
    LOG(INFO, "Vulkan Swapchain created [ Renderer Window Name : %s ]", window.title);

    // init vulkan memory allocator
    VmaAllocatorCreateInfo allocInfo = {};
    allocInfo.instance = instance;
    allocInfo.physicalDevice = device.physical;
    allocInfo.device = device.logical;
    vmaCreateAllocator(&allocInfo, &allocator);
    LOG(INFO, "Memory Allocator created [ Renderer Window Name : %s ]", window.title);
}

void GameZero::Renderer::InitCommands(){
    // create command pool for graphics queue
    VkCommandPoolCreateInfo cmdPoolInfo = VulkanInitialize<VkCommandPoolCreateInfo>();
    cmdPoolInfo.queueFamilyIndex = device.graphicsQueueIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    for(size_t i=0; i<FrameOverlapCount; i++){
        CHECK_VK_RESULT(vkCreateCommandPool(device.logical, &cmdPoolInfo, nullptr, &frames[i].commandPool), "Command Pool creation failed");

        // allocate 1 command buffer
        VkCommandBufferAllocateInfo cmdBuffAllocInfo = VulkanInitialize<VkCommandBufferAllocateInfo>();
        cmdBuffAllocInfo.commandPool = frames[i].commandPool;
        cmdBuffAllocInfo.commandBufferCount = 1;
        cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        CHECK_VK_RESULT(vkAllocateCommandBuffers(device.logical, &cmdBuffAllocInfo, &frames[i].commandBuffer), "Command Buffer allocation failed");
    }
}

void GameZero::Renderer::InitDepthImage(){
    VkExtent3D depthImageExtent;
    depthImageExtent.width = swapchain.imageExtent.width;
    depthImageExtent.height = swapchain.imageExtent.height;
    depthImageExtent.depth = 1.f;
   
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.arrayLayers = 1;
    imageInfo.mipLevels = 1;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.format = VK_FORMAT_D32_SFLOAT;
    imageInfo.extent = depthImageExtent;

    VmaAllocationCreateInfo imageAllocInfo = {};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    imageAllocInfo.requiredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    CHECK_VK_RESULT(vmaCreateImage(allocator, &imageInfo, &imageAllocInfo, &depthImage.image, &depthImage.allocation, nullptr), "Failed to create Depth Image");

    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.image = depthImage.image;
    imageViewInfo.format = VK_FORMAT_D32_SFLOAT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    CHECK_VK_RESULT(vkCreateImageView(device.logical, &imageViewInfo, nullptr, &depthImageView), "Failed to create Image View for Depth Image");
}

// init renderpass
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

    // depth attachment
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // depth attachment reference
    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // describe subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // attachments for renderpass
    VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };

    // create renderpass
    VkRenderPassCreateInfo renderPassInfo = VulkanInitialize<VkRenderPassCreateInfo>();
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    CHECK_VK_RESULT(vkCreateRenderPass(device.logical, &renderPassInfo, nullptr, &renderPass.renderPass), "Failed to create Vulkan Render Pass");
}

void GameZero::Renderer::InitFramebuffers(){
    VkFramebufferCreateInfo fbInfo = VulkanInitialize<VkFramebufferCreateInfo>();
    fbInfo.renderPass = renderPass.renderPass;
    fbInfo.attachmentCount = 2; // number of images attachments in renderpass
    fbInfo.width = swapchain.imageExtent.width;
    fbInfo.height = swapchain.imageExtent.height;
    fbInfo.layers = 1;

    renderPass.framebuffers.resize(swapchain.imageCount);
    for(size_t fbID = 0; fbID < swapchain.imageCount; fbID++){
        // framebuffer takes images view for swapchain image and depth image
        VkImageView fbAttachments[2] = {swapchain.imageViews[fbID], depthImageView};
        fbInfo.pAttachments = fbAttachments;
        CHECK_VK_RESULT(vkCreateFramebuffer(device.logical, &fbInfo, nullptr, &renderPass.framebuffers[fbID]), "Failed to create Vulkan Framebuffer");
    }
}

void GameZero::Renderer::InitSyncStructures(){
    // create fence
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // create semaphores
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for(auto& frame : frames){
        CHECK_VK_RESULT(vkCreateFence(device.logical, &fenceInfo, nullptr, &frame.renderFence), "Failed to create Vulkan Fence");
        CHECK_VK_RESULT(vkCreateSemaphore(device.logical, &semaphoreInfo, nullptr, &frame.renderSemaphore), "Failed to create Vulkan Semaphore");
        CHECK_VK_RESULT(vkCreateSemaphore(device.logical, &semaphoreInfo, nullptr, &frame.presentSemaphore), "Failed to create Vulkan Semaphore");
    }
}

void GameZero::Renderer::Draw(){
    auto& frame = GetCurrentFrame();

    // wait until gpu singals us that it is done rendering
    CHECK_VK_RESULT(vkWaitForFences(device.logical, 1, &frame.renderFence, VK_TRUE, 1e9), "Waiting for Vulkan ( Render ) Fence failed");
    // then reset render fence
    CHECK_VK_RESULT(vkResetFences(device.logical, 1, &frame.renderFence), "Reset Vulkan ( Render ) Fence failed");

    // get next image to render to from swapchain
    // swapchain will signal present semaphore when it is done presenting it
    uint32_t nextImageIndex;
    CHECK_VK_RESULT(vkAcquireNextImageKHR(device.logical, swapchain.swapchain, 1e9, frame.presentSemaphore, VK_NULL_HANDLE, &nextImageIndex), "Failed to get next image from Swapchain");

    // reset command buffer for use
    CHECK_VK_RESULT(vkResetCommandBuffer(frame.commandBuffer, 0), "Failed to reset Command Buffer");

    // rename it for shorter name
    VkCommandBuffer cmd = frame.commandBuffer;

    // begin command buffer
    VkCommandBufferBeginInfo cmdBeginInfo = VulkanInitialize<VkCommandBufferBeginInfo>();
    cmdBeginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CHECK_VK_RESULT(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "Command Buffer recording failed");

    // clear value for color attachment on renderpass begin
    VkClearValue colorClear = {};
    colorClear.color = {{0.01f, 0.01f, 0.01f, 1.0f}};

    VkClearValue depthClear = {};
    depthClear.depthStencil.depth = 1.f;
    
    VkClearValue clearValues[2] = { colorClear, depthClear};

    // begin renderpass
    VkRenderPassBeginInfo rpBeginInfo = VulkanInitialize<VkRenderPassBeginInfo>();
    rpBeginInfo.clearValueCount = 2;
    rpBeginInfo.pClearValues = clearValues;
    rpBeginInfo.renderPass = renderPass.renderPass;
    rpBeginInfo.framebuffer = renderPass.framebuffers[nextImageIndex];
    rpBeginInfo.renderArea = VkRect2D{/*offset*/{0, 0}, /*extent*/{window.GetExtent()}};
    
    // begin renderpass
    vkCmdBeginRenderPass(cmd, &rpBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

    // draw objects
    DrawObjects(cmd, renderables.data(), renderables.size());

    // end renderpass
    vkCmdEndRenderPass(cmd);

    // end command buffer recording
    vkEndCommandBuffer(cmd);

    // prepare to submit command buffer
    VkSubmitInfo submitInfo = VulkanInitialize<VkSubmitInfo>();
    
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.presentSemaphore;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &frame.renderSemaphore;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitStage;

    // submit command buffer to graphisc queue
    // render fence will now block cpu when cpu issues a vkWaitForFences until unless rendering is done
    CHECK_VK_RESULT(vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, frame.renderFence), "Failed to submit Command Buffers to Graphics Queue");

    // after rendering we need to show it on screen
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain.swapchain;

    // this will ask vulkan to wait before rendering is complete
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &frame.renderSemaphore;

    presentInfo.pImageIndices = &nextImageIndex;
    CHECK_VK_RESULT(vkQueuePresentKHR(device.presentQueue, &presentInfo), "Failed to present rendered image to screen");
    
    // next frame
    frameNumber++;
}

// inpit pipeline layouts
void GameZero::Renderer::InitPipelineLayouts(){
    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayout;
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

    // get vertex description
    VertexInputDescription vertexDescription = Vertex::GetVertexDescription();

    // vertex input state
    VkPipelineVertexInputStateCreateInfo vertexInput = {};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = vertexDescription.bindings.size();
    vertexInput.pVertexBindingDescriptions = vertexDescription.bindings.data();
    vertexInput.vertexAttributeDescriptionCount = vertexDescription.attributes.size();
    vertexInput.pVertexAttributeDescriptions = vertexDescription.attributes.data();

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

    // depth stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // draw on other objects
    depthStencil.depthTestEnable = VK_TRUE;
    // write depth value
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

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

    graphicsPipelineInfo.pDepthStencilState = &depthStencil;
    graphicsPipelineInfo.pColorBlendState = &colorBlending;
    graphicsPipelineInfo.pViewportState = &viewportState;

    graphicsPipelineInfo.layout = pipelineLayout;
    graphicsPipelineInfo.renderPass =  renderPass.renderPass;
    graphicsPipelineInfo.subpass = 0;

    CHECK_VK_RESULT(vkCreateGraphicsPipelines(device.logical, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, nullptr, &pipeline), "Failed to create Graphics Pipeline");

    // we dont need shader modules anymore
    vkDestroyShaderModule(device.logical, vertShader, nullptr);
    vkDestroyShaderModule(device.logical, fragShader, nullptr);

    // create default material
    CreateMaterial(pipeline, pipelineLayout, "default");
}

void GameZero::Renderer::InitMesh(){
    // TODO : DO SOMETHING ABOUT THIS PATH
    mesh.LoadMeshFromOBJ("../mesh/GolemMan.obj");
    mesh.UploadMeshToGPU(allocator);

    meshes["GolemMan"] = mesh;
}

// create a new material for renderer
GameZero::Material* GameZero::Renderer::CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name){
    Material material{pipeline, layout};
    materials[name] = material;
    return &materials[name];
}

// get material from name
GameZero::Material* GameZero::Renderer::GetMaterial(const std::string &name){
    //search for the object, and return nullptr if not found
	auto it = materials.find(name);
	if (it == materials.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}

// get mesh
GameZero::Mesh* GameZero::Renderer::GetMesh(const std::string &name){
    auto it = meshes.find(name);
	if (it == meshes.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}

// draw multiple objects
void GameZero::Renderer::DrawObjects(VkCommandBuffer cmd, RenderObject *firstObject, uint32_t count){
	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = firstObject[i];

		//only bind the pipeline if it doesn't match with the already bound one
		if (object.material != lastMaterial) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
			lastMaterial = object.material;
		}

        // update camera data
        cameraData.model = object.transform;

        //and copy it to the buffer
        void* data;
        vmaMapMemory(allocator, GetCurrentFrame().cameraBuffer.allocation, &data);
        memcpy(data, &cameraData, sizeof(GPUCameraData));
        vmaUnmapMemory(allocator, GetCurrentFrame().cameraBuffer.allocation);

		//only bind the mesh if it's a different one from last bind
		if (object.mesh != lastMesh) {
			//bind the mesh vertex buffer with offset 0
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer.buffer, &offset);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 0, 1, &GetCurrentFrame().descriptorSet, 0, nullptr);
            lastMesh = object.mesh;
		}

		//we can now draw
		vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
	}   
}

void GameZero::Renderer::InitScene(){
    RenderObject object;
    object.mesh = GetMesh("GolemMan");
    if(!object.mesh) LOG(DEBUG, "Failed to Get Mesh");
    object.material = GetMaterial("default");
    if(!object.material) LOG(DEBUG, "Failed to Get Material");

    renderables.push_back(object);
}

void GameZero::Renderer::InitDescriptors(){
    //create a descriptor pool that will hold 10 uniform buffers
	std::vector<VkDescriptorPoolSize> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = 0;
	pool_info.maxSets = 10;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	CHECK_VK_RESULT(vkCreateDescriptorPool(device.logical, &pool_info, nullptr, &descriptorPool), "Failed to create Descriptor Pool");
    LOG(INFO, "Created Descriptor Pool");

	//information about the binding.
	VkDescriptorSetLayoutBinding camBufferBinding = {};
	camBufferBinding.binding = 0;
	camBufferBinding.descriptorCount = 1;
	// it's a uniform buffer binding
	camBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	// we use it from the vertex shader
	camBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo setInfo = {};
	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setInfo.pNext = nullptr;
	//we are going to have 1 binding
	setInfo.bindingCount = 1;
	//no flags
	setInfo.flags = 0;
	//point to the camera buffer binding
	setInfo.pBindings = &camBufferBinding;

	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(device.logical, &setInfo, nullptr, &descriptorSetLayout), "Failed to create Descriptor Set Layout");
    LOG(INFO, "Create Descriptor Set Layout");

    for(auto& frame : frames){
        frame.cameraBuffer = CreateBuffer(allocator, sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    
        //allocate one descriptor set for each frame
		VkDescriptorSetAllocateInfo allocInfo ={};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		//using the pool we just set
		allocInfo.descriptorPool = descriptorPool;
		//only 1 descriptor
		allocInfo.descriptorSetCount = 1;
		//using the global data layout
		allocInfo.pSetLayouts = &descriptorSetLayout;

		CHECK_VK_RESULT(vkAllocateDescriptorSets(device.logical, &allocInfo, &frame.descriptorSet), "Failed to allocate Descriptor Set");
    
        //information about the buffer we want to point at in the descriptor
		VkDescriptorBufferInfo binfo = {};
		//it will be the camera buffer
		binfo.buffer = frame.cameraBuffer.buffer;
		//at 0 offset
		binfo.offset = 0;
		//of the size of a camera data struct
		binfo.range = sizeof(GPUCameraData);

		VkWriteDescriptorSet setWrite = {};
		setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrite.pNext = nullptr;
		//we are going to write into binding number 0
		setWrite.dstBinding = 0;
		//of the global descriptor
		setWrite.dstSet = frame.descriptorSet;

		setWrite.descriptorCount = 1;
		//and the type is uniform buffer
		setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrite.pBufferInfo = &binfo;

        // update
		vkUpdateDescriptorSets(device.logical, 1, &setWrite, 0, nullptr);
    }
}