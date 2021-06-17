#include "renderer.hpp"
#include "texture.hpp"
#include "utils/assert.hpp"
#include "utils.hpp"
#include "utils/vulkan_initializers.hpp"
#include "vulkan/vk_mem_alloc.hpp"
#include "vulkan/vulkan_core.h"
#include "shader.hpp"


GameZero::Renderer::Renderer(GameZero::Window& window) : window(window){
    // generate log
    LOG(INFO, "New Renderer created [ Renderer Window Name : %s ]", window.title);

    // initialize vulkan
    Initialize();
}

GameZero::Renderer::~Renderer(){
    // wait for all device operations to complete
    device.logical.waitIdle();

    // destroy swapchain
    swapchain.Destroy(device);
    // destroy device
    device.Destroy();
    // destroy surface
    surface.Destroy();
}


// initialize renderer
void GameZero::Renderer::Initialize(){
    InitSurface();
    InitDevice();
    InitCommands();
    InitDepthImage();
    InitRenderPass();
    InitFramebuffers();
    InitSyncStructures();
    InitDescriptors();
    InitPipelineLayouts();
    InitPipelines();
    InitMesh();
    LoadImages();
    InitScene();
}

// create surface for given window to this renderer
void GameZero::Renderer::InitSurface(){
    surface.Create(window);
}

// init device for the renderer
void GameZero::Renderer::InitDevice(){
    device.Create(surface);
}

// init swapchain
void GameZero::Renderer::InitSwapchain(){
    swapchain.Create(surface, device);
}

void GameZero::Renderer::InitCommands(){
    // create command pool for graphics queue
    vk::CommandPoolCreateInfo cmdPoolInfo(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, /* flags */
        device.graphicsQueueIndex /* queue index */
    );
    
    for(auto& frame : frames){
        frame.commandPool = device.logical.createCommandPool(cmdPoolInfo);

        // allocate 1 command buffer
        vk::CommandBufferAllocateInfo cmdBuffAllocInfo(
            frame.commandPool, /* command pool */
            vk::CommandBufferLevel::ePrimary, /* command buffer level */
            1 /* command buffer level count */
        );

        frame.commandBuffer = device.logical.allocateCommandBuffers(cmdBuffAllocInfo).front();
        // deletor
        PushFunction([=](){
            device.logical.destroyCommandPool(frame.commandPool);
        });
    }

    cmdPoolInfo.flags = {};
    uploadContext.commandPool = device.logical.createCommandPool(cmdPoolInfo);
    // deletor
    PushFunction([=](){
        device.logical.destroyCommandPool(uploadContext.commandPool);
    });
}

void GameZero::Renderer::InitDepthImage(){
    vk::Extent3D depthImageExtent;
    depthImageExtent.width = swapchain.imageExtent.width;
    depthImageExtent.height = swapchain.imageExtent.height;
    depthImageExtent.depth = 1.f;
   
    vk::ImageCreateInfo imageInfo(
        {}, /* flags */
        vk::ImageType::e2D, /* image type */
        vk::Format::eD32Sfloat, /* format */
        depthImageExtent, /* extent */
        1, /* mip levels */
        1, /* array layers */
        vk::SampleCountFlagBits::e1, /* sample count */
        vk::ImageTiling::eOptimal, /* tiling */
        vk::ImageUsageFlagBits::eDepthStencilAttachment /* usage */
    );
    
    vma::AllocationCreateInfo imageAllocInfo = {};
    imageAllocInfo.usage = vma::MemoryUsage::eGpuOnly;
    imageAllocInfo.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

    device.allocator.createImage(&imageInfo, &imageAllocInfo, &depthImage.image, &depthImage.allocation, nullptr);
    // deletor
    PushFunction([=](){
        device.allocator.destroyImage(depthImage.image, depthImage.allocation);
        LOG(INFO, "Destroyed Image");
    });

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

    // create image view
    depthImage.view = device.logical.createImageView(imageViewInfo);
    // deletor
    PushFunction([=](){
        device.logical.destroyImageView(depthImage.view);
    });
}

// init renderpass
void GameZero::Renderer::InitRenderPass(){
    vk::AttachmentDescription colorAttachment(
        {}, /* flags */
        swapchain.imageFormat, /* format */
        vk::SampleCountFlagBits::e1, /* sample count */
        vk::AttachmentLoadOp::eClear, /* attachment load op*/
        vk::AttachmentStoreOp::eStore, /* attachment store op*/
        vk::AttachmentLoadOp::eDontCare, /* stencil load op */
        vk::AttachmentStoreOp::eDontCare /* stencil store op */
    );
    

    // make an attachment reference for subpass
    vk::AttachmentReference colorAttachmentRef(
        0, /* attachment */
        vk::ImageLayout::eColorAttachmentOptimal /* layout */
    );

    // depth attachment
    vk::AttachmentDescription depthAttachment(
        {}, /* flags */
        vk::Format::eD32Sfloat, /* format */
        vk::SampleCountFlagBits::e1, /* sample count */
        vk::AttachmentLoadOp::eClear, /* load op */
        vk::AttachmentStoreOp::eStore, /* store op */
        vk::AttachmentLoadOp::eDontCare, /* stencil load op */
        vk::AttachmentStoreOp::eDontCare /* stencil store op */
    );

    // depth attachment reference
    vk::AttachmentReference depthAttachmentRef(
        1, /* attachment */
        vk::ImageLayout::eDepthStencilAttachmentOptimal /* layout */
    );

    // describe subpass
    vk::SubpassDescription subpass(
        {}, /* flags */
        vk::PipelineBindPoint::eGraphics, /* bind point */
        0, /* input attachment count */
        nullptr, /* input attachment */
        1, /* color attachment count */
        &colorAttachmentRef, /* color attachments */
        nullptr, /* resolve attachment */
        &depthAttachmentRef, /* depth stencil attachment */
        0, /* preserve attachment count */
        nullptr /* preserve attachment */
    );
    // attachments for renderpass
    vk::AttachmentDescription attachments[2] = { colorAttachment, depthAttachment };

    // create renderpass
    vk::RenderPassCreateInfo renderPassInfo(
        {}, /* flags */
        2, /* attachment count */
        attachments, /* attachments */
        1, /* subpass count */
        &subpass /* subpasses */
    );


    renderPass.renderPass = device.logical.createRenderPass(renderPassInfo, nullptr);
    // destroy renderpass
    PushFunction([=](){
        device.logical.destroyRenderPass(renderPass.renderPass);
    });
}

void GameZero::Renderer::InitFramebuffers(){
    renderPass.framebuffers.resize(swapchain.imageCount);
    for(size_t fbID = 0; fbID < swapchain.imageCount; fbID++){
        vk::ImageView fbAttachments[2] = {swapchain.imageViews[fbID], depthImage.view};
       
        vk::FramebufferCreateInfo fbInfo(
            {},
            renderPass.renderPass,
            2,
            fbAttachments,
            swapchain.imageExtent.width,
            swapchain.imageExtent.height,
            1
        );

        // framebuffer takes images view for swapchain image and depth image
        fbInfo.pAttachments = fbAttachments;
        renderPass.framebuffers[fbID] = device.logical.createFramebuffer(fbInfo);
        // deletor
        PushFunction([=](){
            device.logical.destroyFramebuffer(renderPass.framebuffers[fbID]);
        });
    }
}

void GameZero::Renderer::InitSyncStructures(){
    // fence info
    vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

    for(auto& frame : frames){
        frame.renderFence = device.logical.createFence(fenceInfo);
        frame.renderSemaphore = device.logical.createSemaphore({});
        frame.presentSemaphore = device.logical.createSemaphore({});
        // deletors
        PushFunction([=](){
            device.logical.destroyFence(frame.renderFence);
            device.logical.destroySemaphore(frame.renderSemaphore);
            device.logical.destroySemaphore(frame.presentSemaphore);
        });
    }

    uploadContext.uploadFence = device.logical.createFence({});
    // deletor
    PushFunction([=](){
        device.logical.destroyFence(uploadContext.uploadFence);
    });
}

void GameZero::Renderer::Draw(){
    auto& frame = GetCurrentFrame();

    // wait until gpu singals us that it is done rendering
    device.logical.waitForFences({frame.renderFence}, true, 1e9);
    // then reset render fence
    device.logical.resetFences({frame.renderFence});

    // get next image to render to from swapchain
    // swapchain will signal present semaphore when it is done presenting it
    uint32_t nextImageIndex = device.logical.acquireNextImageKHR(swapchain.swapchain, 1e9, frame.presentSemaphore).value;

    // reset command buffer for use
    frame.commandBuffer.reset();

    // rename it for shorter name
    vk::CommandBuffer cmd = frame.commandBuffer;

    // begin command buffer
    vk::CommandBufferBeginInfo cmdBeginInfo(
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    );
    cmd.begin(cmdBeginInfo);

    // clear value for color attachment on renderpass begin
    vk::ClearValue colorClear;
    colorClear.color = {};

    vk::ClearValue depthClear = {};
    depthClear.depthStencil.depth = 1.f;
    
    vk::ClearValue clearValues[2] = { colorClear, depthClear};

    // begin renderpass
    vk::RenderPassBeginInfo rpBeginInfo(
        renderPass.renderPass, /* renderpass */
        renderPass.framebuffers[nextImageIndex], /* framebuffer */
        vk::Rect2D( /* render area*/
            {0, 0},
            window.GetExtent()
        ),
        2, /* clear value count */
        clearValues /* clear values */
    );

    // begin renderpass
    cmd.beginRenderPass(rpBeginInfo, vk::SubpassContents::eInline);

    // draw objects
    DrawObjects(cmd, renderables.data(), renderables.size());

    // end renderpass
    cmd.endRenderPass();

    // end command buffer recording
    cmd.end();

    // prepare to submit command buffer
    vk::SubmitInfo submitInfo;
    
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.presentSemaphore;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &frame.renderSemaphore;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    submitInfo.pWaitDstStageMask = &waitStage;

    // submit command buffer to graphisc queue
    // render fence will now block cpu when cpu issues a vkWaitForFences until unless rendering is done
    device.graphicsQueue.submit(submitInfo, frame.renderFence);
    
    // after rendering we need to show it on screen
    vk::PresentInfoKHR presentInfo(
        1, /* wait semaphore count */
        &frame.renderSemaphore, /* wait semaphore */
        1, /* swapchain count */
        &swapchain.swapchain, /* swapchains */
        &nextImageIndex /* image indices */
    );

    // present to surface
    device.presentQueue.presentKHR(presentInfo);

    // next frame
    frameNumber++;
}

// inpit pipeline layouts
void GameZero::Renderer::InitPipelineLayouts(){
    VkDescriptorSetLayout setLayouts[] = {descriptorSetLayout, singleTextureSetLayout};

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 2;
    layoutInfo.pSetLayouts = setLayouts;
    layoutInfo.pushConstantRangeCount = 0;
    
    CHECK_VK_RESULT(vkCreatePipelineLayout(device.logical, &layoutInfo, nullptr, &pipelineLayout), "Failed to create Pipeline Layout");
    // deletor
    PushFunction([=](){
        vkDestroyPipelineLayout(device.logical, pipelineLayout, nullptr);
        LOG(INFO, "Destroted Pipeline Layout")
    });
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
    // deletor
    PushFunction([=](){
        vkDestroyPipeline(device.logical, pipeline, nullptr);
        LOG(INFO, "Destroyed Pipeline")
    });

    // we dont need shader modules anymore
    vkDestroyShaderModule(device.logical, vertShader, nullptr);
    vkDestroyShaderModule(device.logical, fragShader, nullptr);

    // create default material
    CreateMaterial(pipeline, pipelineLayout, "default");
}

void GameZero::Renderer::InitMesh(){
    // TODO : DO SOMETHING ABOUT THIS PATH
    mesh.LoadMeshFromOBJ("../mesh/lost_empire.obj");
    UploadMeshToGPU(&mesh, allocator);
    meshes["TestMesh"] = mesh;
}

// create a new material for renderer
GameZero::Material* GameZero::Renderer::CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name){
    Material material;
    material.pipeline = pipeline;
    material.pipelineLayout = pipelineLayout;
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

        if(object.material->textureSet != VK_NULL_HANDLE){
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1, 1, &object.material->textureSet, 0, nullptr);
        }

		//we can now draw
		vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
	}   
}

void GameZero::Renderer::InitScene(){
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;

    VkSampler blockySampler;
    CHECK_VK_RESULT(vkCreateSampler(device.logical, &samplerInfo, nullptr, &blockySampler), "Failed to create sampler")
    PushFunction([=](){
        vkDestroySampler(device.logical, blockySampler, nullptr);
        LOG(INFO, "Destroyed Sampler");
    });

    RenderObject object;
    object.mesh = GetMesh("TestMesh");
    if(!object.mesh) LOG(DEBUG, "Failed to Get Mesh");

    object.material = GetMaterial("default");
    if(!object.material) LOG(DEBUG, "Failed to Get Material");

    VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &singleTextureSetLayout;

    CHECK_VK_RESULT(vkAllocateDescriptorSets(device.logical, &allocInfo, &object.material->textureSet), "Failed to allocate Descriptor Set");

    VkDescriptorImageInfo imageBufferInfo = {};
	imageBufferInfo.sampler = blockySampler;
	imageBufferInfo.imageView = textures["empire_diffuse"].imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet textureWrite = {};
    textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    textureWrite.descriptorCount = 1;
    textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureWrite.dstSet = object.material->textureSet;
    textureWrite.dstBinding = 0;
    textureWrite.pImageInfo = &imageBufferInfo;
    
    vkUpdateDescriptorSets(device.logical, 1, &textureWrite, 0, nullptr);

    renderables.push_back(object);
}

void GameZero::Renderer::InitDescriptors(){
    //create a descriptor pool that will hold 10 uniform buffers
	std::vector<VkDescriptorPoolSize> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = 0;
	pool_info.maxSets = 10;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	CHECK_VK_RESULT(vkCreateDescriptorPool(device.logical, &pool_info, nullptr, &descriptorPool), "Failed to create Descriptor Pool");
    // deletor
    PushFunction([=](){
        vkDestroyDescriptorPool(device.logical, descriptorPool, nullptr);
        LOG(INFO, "Destroyed Descriptor Pool")
    });
    LOG(INFO, "Created Descriptor Pool");

	//information about the binding.
	VkDescriptorSetLayoutBinding camBufferBinding = {};
	camBufferBinding.binding = 0;
    // this variable is for passing array of data
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
    // deletor
    PushFunction([=](){
        vkDestroyDescriptorSetLayout(device.logical, descriptorSetLayout, nullptr);
        LOG(INFO, "Destroyed Descriptor Set Layout");
    });
    LOG(INFO, "Created Descriptor Set Layout");

    VkDescriptorSetLayoutBinding textureBinding = {};
    textureBinding.binding = 0;
    textureBinding.descriptorCount = 1;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo textureSetLayoutInfo = {};
    textureSetLayoutInfo.bindingCount = 1;
    textureSetLayoutInfo.pBindings = &textureBinding;
    textureSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    CHECK_VK_RESULT(vkCreateDescriptorSetLayout(device.logical, &textureSetLayoutInfo, nullptr, &singleTextureSetLayout), "Failed to create Descriptor Set Layout");
    // deletor
    PushFunction([=](){
        vkDestroyDescriptorSetLayout(device.logical, singleTextureSetLayout, nullptr);
        LOG(INFO, "Destroyed Descriptor Set Layout");
    });
    LOG(INFO, "Created Descriptor Set Layout");

    for(auto& frame : frames){
        frame.cameraBuffer = CreateBuffer(allocator, sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        // deletor
        PushFunction([=](){
            vmaDestroyBuffer(allocator, frame.cameraBuffer.buffer, frame.cameraBuffer.allocation);
            LOG(INFO, "Destroyed Camera Buffer (Uniform)");
        });

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

void GameZero::Renderer::ImmediateSubmit(std::function<void (VkCommandBuffer)> &&function){
    // allocate default command buffer that we will use for instant commands
    VkCommandBufferAllocateInfo cmdBuffAllocInfo = {};
    cmdBuffAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBuffAllocInfo.commandBufferCount = 1;
    cmdBuffAllocInfo.commandPool = uploadContext.commandPool;
    cmdBuffAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    // allocate
    VkCommandBuffer cmd;
    CHECK_VK_RESULT(vkAllocateCommandBuffers(device.logical, &cmdBuffAllocInfo, &cmd), "Failed to allocate Command Buffer");
    
    // begin command buffer
    // we will use this command buffer only once, unlike in a draw command
    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // begin recording
    CHECK_VK_RESULT(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "Failed to begin Command Buffer recording");

    // execute function
    function(cmd);

    // end recording
    CHECK_VK_RESULT(vkEndCommandBuffer(cmd), "Failed to end Command Buffer recording");

    // submit info
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 0;

    // submit to graphics queue
    CHECK_VK_RESULT(vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, uploadContext.uploadFence), "Failed to submit Command Buffer");

    // wait for operations to complete
    // and then reset fence
    CHECK_VK_RESULT(vkWaitForFences(device.logical, 1, &uploadContext.uploadFence, VK_TRUE, 1e9), "Failed to wait for Fence");
    CHECK_VK_RESULT(vkResetFences(device.logical, 1, &uploadContext.uploadFence), "Failed to reset Fence");

    // reset command pool
    CHECK_VK_RESULT(vkResetCommandPool(device.logical, uploadContext.commandPool, 0), "Failed to reset Command Pool");
}

void GameZero::Renderer::UploadMeshToGPU(Mesh* mesh, bool useStaging){
	// no staging buffer
	if(!useStaging){
		VkBufferCreateInfo  bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = mesh->vertices.size() * sizeof(Vertex);
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		// cpu to gpu read is slow
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		// create buffer
		CHECK_VK_RESULT(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &mesh->vertexBuffer.buffer, &mesh->vertexBuffer.allocation, nullptr), "Failed to create vertex buffer")
        // deletor
        PushFunction([=](){
            vmaDestroyBuffer(allocator, mesh->vertexBuffer.buffer, mesh->vertexBuffer.allocation);
            LOG(INFO, "Destroyed Vertex Buffer for mesh");
        });

		// copy vertex data
		void* data;
		vmaMapMemory(allocator, mesh->vertexBuffer.allocation, &data);
		memcpy(data, mesh->vertices.data(), mesh->vertices.size() * sizeof(Vertex));
		vmaUnmapMemory(allocator, mesh->vertexBuffer.allocation);
	}else{
		// use staging buffer
		// staging buffer is basically a cpu only buffer copied to gpu only buffer
		// that gpu only buffer is called staged buffer
		const size_t bufferSize = mesh->vertices.size() * sizeof(Vertex);
		// allocate staging buffer
		VkBufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.size = bufferSize;
		// this is source of transfer
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		// we want this data to be cpu readble only
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_COPY;
		
		AllocatedBuffer stagingBuffer;

		// allocate buffer
		CHECK_VK_RESULT(vmaCreateBuffer(allocator, &stagingBufferInfo, &allocInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, nullptr), "Failed to create Staging Buffer");

		// copy data to staging buffer
		void *data;
		vmaMapMemory(allocator, stagingBuffer.allocation, &data);
		memcpy(data, mesh->vertices.data(), bufferSize);
		vmaUnmapMemory(allocator, stagingBuffer.allocation);

		// create staged buffer
		VkBufferCreateInfo stagedBufferInfo = {};
		stagedBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagedBufferInfo.size = bufferSize;
		// this is destination of a transfer operation and also a vertex buffer at the same time
		stagedBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		// this buffer will be gpu only
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		// allocate buffer
		CHECK_VK_RESULT(vmaCreateBuffer(allocator, &stagedBufferInfo, &allocInfo, &mesh->vertexBuffer.buffer, &mesh->vertexBuffer.allocation, nullptr), "Failed to create Staged Buffer");
        // deletor
        PushFunction([=](){
            vmaDestroyBuffer(allocator, mesh->vertexBuffer.buffer, mesh->vertexBuffer.allocation);
            LOG(INFO, "Destroyed Vertex Buffer for mesh");
        });

		// perform an immediate copy operation
        ImmediateSubmit([=](VkCommandBuffer cmd){
            VkBufferCopy copy;
            copy.dstOffset = 0;
            copy.srcOffset = 0;
            copy.size = bufferSize;
            vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh->vertexBuffer.buffer, 1, &copy);
        });

        // after copy, destroy staging buffer as we dont need it anymore
        vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
	}
}

void GameZero::Renderer::LoadImages(){
    Texture texture;
    LoadImageFromFile(this, "../assets/textures/lost_empire-RGBA.png", texture.image);
    
    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageViewInfo.image = texture.image.image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.levelCount = 1;

    // create image view
    CHECK_VK_RESULT(vkCreateImageView(device.logical, &imageViewInfo, nullptr, &texture.imageView), "Failed to create Wmage View");
    PushFunction([=](){
        vkDestroyImageView(device.logical, texture.imageView, nullptr);
        LOG(INFO, "Destroted Image View");
    });

    textures["empire_diffuse"] = texture;
}