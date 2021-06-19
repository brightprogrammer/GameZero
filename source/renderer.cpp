#include "renderer.hpp"
#include "texture.hpp"
#include "utils/assert.hpp"
#include "utils.hpp"
#include "utils/vulkan_initializers.hpp"
#include "vulkan/vk_mem_alloc.hpp"
#include "vulkan/vulkan.hpp"
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
    LOG(DEBUG, "DONE HERE");
    InitSurface();
    LOG(DEBUG, "DONE HERE");
    InitDevice();
    LOG(DEBUG, "DONE HERE");
    InitCommands();
    LOG(DEBUG, "DONE HERE");
    InitDepthImage();
    LOG(DEBUG, "DONE HERE");
    InitRenderPass();
    LOG(DEBUG, "DONE HERE");
    InitFramebuffers();
    LOG(DEBUG, "DONE HERE");
    InitSyncStructures();
    LOG(DEBUG, "DONE HERE");
    InitDescriptors();
    LOG(DEBUG, "DONE HERE");
    InitPipelineLayouts();
    LOG(DEBUG, "DONE HERE");
    InitPipelines();
    LOG(DEBUG, "DONE HERE");
    InitMesh();
    LOG(DEBUG, "DONE HERE");
    LoadImages();
    LOG(DEBUG, "DONE HERE");
    InitScene();
    LOG(DEBUG, "DONE HERE");
}

// create surface for given window to this renderer
void GameZero::Renderer::InitSurface(){
    LOG(DEBUG, "DONE HERE");
    surface.Create(window);
    LOG(DEBUG, "DONE HERE");
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
    });

    vk::ImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.viewType = vk::ImageViewType::e2D;
    imageViewInfo.image = depthImage.image;
    imageViewInfo.format = vk::Format::eD32Sfloat;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;

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
    CHECK_VK_RESULT(device.logical.waitForFences({frame.renderFence}, true, 1e9), "Wait for fences failed");
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
    ASSERT(device.presentQueue.presentKHR(presentInfo) == vk::Result::eSuccess, "Failed to present rendered images");
    
    // next frame
    frameNumber++;
}

// inpit pipeline layouts
void GameZero::Renderer::InitPipelineLayouts(){
    vk::DescriptorSetLayout setLayouts[] = {descriptorSetLayout, singleTextureSetLayout};

    // pipeline layout create info
    vk::PipelineLayoutCreateInfo layoutInfo(
        {}, /* flags */
        2, /* set layout count*/
        setLayouts, /* sey layouts */
        0, /* push constant range count */
        nullptr /* push constant ranges */
    );

    pipelineLayout = device.logical.createPipelineLayout(layoutInfo);
    // deletor
    PushFunction([=](){
        device.logical.destroyPipelineLayout(pipelineLayout);
    });
}

// init pipelines
void GameZero::Renderer::InitPipelines(){
    // create shader modules
    vk::ShaderModule vertShader = LoadShaderModule(device, "shaders/vert.spv");
    vk::ShaderModule fragShader = LoadShaderModule(device, "shaders/frag.spv");

    // vertex shader stage
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
        {}, /* flags */
        vk::ShaderStageFlagBits::eVertex, /* stage */
        vertShader, /* module */
        "main", /* name */
        nullptr /* specitalization info */
    );
    
    // fragment shader stage
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
        {}, /* flags */
        vk::ShaderStageFlagBits::eFragment, /* stage */
        fragShader, /* module */
        "main", /* name */
        nullptr /* specitalization info */
    );
    
    vk::PipelineShaderStageCreateInfo shaderStages[2] = {
        fragShaderStageInfo, vertShaderStageInfo
    };

    // get vertex description
    VertexInputDescription vertexDescription = Vertex::GetVertexDescription();

    // vertex input state
    vk::PipelineVertexInputStateCreateInfo vertexInput(
        vertexDescription.flags, /* flags */
        vertexDescription.bindings.size(), /* binding count */
        vertexDescription.bindings.data(), /* bindings */
        vertexDescription.attributes.size(), /* attribute count */
        vertexDescription.attributes.data() /* attributes */
    );

    // input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
        {}, /* flags */
        vk::PrimitiveTopology::eTriangleList, /* topology */
        false /* primitive reastart enable */
    );

    // rasterization
    vk::PipelineRasterizationStateCreateInfo rasterizer(
        {}, /* flags */
        false, /* depth clamp enable */
        false, /* rasterization discard enable */
        vk::PolygonMode::eFill, /* polygon mode */
        vk::CullModeFlagBits::eNone, /* coll mode */
        vk::FrontFace::eClockwise, /* front face */
        false, /* depth bias enable */
        0.f, /* depth bias constant factor */
        0.f, /* depth bias clamp */
        0.f, /* depth bias slope factor */
        1.f /* line width*/
    );

    // multisampling
    vk::PipelineMultisampleStateCreateInfo multisampling (
        {}, /* flags */
        vk::SampleCountFlagBits::e1, /* rasterization samples */
        false, /* sample shading enable */
        1.f, /* min sample shading */
        nullptr, /* sample mask pointer */
        false, /* alpha to coverage enable */
        false /* alpha to one enable */
    );

    // depth stencil state
    vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
    // draw on other objects
    depthStencil.depthTestEnable = true;
    // write depth value
    depthStencil.depthWriteEnable = true;
    depthStencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
    depthStencil.depthBoundsTestEnable = false;
    depthStencil.stencilTestEnable = false;

    // color blend attachment
    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =   vk::ColorComponentFlagBits::eA |
                                            vk::ColorComponentFlagBits::eR |
                                            vk::ColorComponentFlagBits::eG |
                                            vk::ColorComponentFlagBits::eB;
    colorBlendAttachment.blendEnable = false;
    
    // color blend sate
    vk::PipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;

    // viewport
    vk::Viewport viewport = {};
    viewport.width = static_cast<float>(window.size.x);
    viewport.height = static_cast<float>(window.size.y);
    viewport.x = 0;
    viewport.y = 0;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    // scissor
    vk::Rect2D scissor = {{0, 0}, window.GetExtent()};

    // viewport state
    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    vk::GraphicsPipelineCreateInfo graphicsPipelineInfo = {};
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

    pipeline = device.logical.createGraphicsPipeline({}, graphicsPipelineInfo).value;
    // deletor
    PushFunction([=](){
        device.logical.destroyPipeline(pipeline);
    });

    // we dont need shader modules anymore
    device.logical.destroyShaderModule(vertShader);
    device.logical.destroyShaderModule(fragShader);

    // create default material
    CreateMaterial(pipeline, pipelineLayout, "default");
}

void GameZero::Renderer::InitMesh(){
    // TODO : DO SOMETHING ABOUT THIS PATH
    mesh.LoadMeshFromOBJ("../mesh/lost_empire.obj");
    UploadMeshToGPU(&mesh, device.allocator);
    meshes["TestMesh"] = mesh;
}

// create a new material for renderer
GameZero::Material* GameZero::Renderer::CreateMaterial(vk::Pipeline pipeline, vk::PipelineLayout layout, const std::string &name){
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
void GameZero::Renderer::DrawObjects(vk::CommandBuffer cmd, RenderObject *firstObject, uint32_t count){
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
        CHECK_VK_RESULT(device.allocator.mapMemory(GetCurrentFrame().cameraBuffer.allocation, &data), "Failed to map memory");
        memcpy(data, &cameraData, sizeof(GPUCameraData));
        device.allocator.unmapMemory(GetCurrentFrame().cameraBuffer.allocation);

		//only bind the mesh if it's a different one from last bind
		if (object.mesh != lastMesh) {
			//bind the mesh vertex buffer with offset 0
			vk::DeviceSize offset = 0;
			cmd.bindVertexBuffers(0, 1, &object.mesh->vertexBuffer.buffer, &offset);
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, object.material->pipelineLayout, 0, 1, &GetCurrentFrame().descriptorSet, 0, nullptr);
            lastMesh = object.mesh;
		}

        // bind texture pipeline only if there is a texture set in current object's material 
        if(object.material->textureSet){
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, object.material->pipelineLayout, 1, 1, &object.material->textureSet, 0, nullptr);
        }

		//we can now draw
		vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
	}   
}

void GameZero::Renderer::InitScene(){
    vk::SamplerCreateInfo samplerInfo = {};
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.magFilter = vk::Filter::eNearest;
    samplerInfo.minFilter = vk::Filter::eNearest;

    vk::Sampler blockySampler;
    CHECK_VK_RESULT(device.logical.createSampler(&samplerInfo, nullptr, &blockySampler), "Failed to create sampler")
    PushFunction([=](){
        device.logical.destroySampler(blockySampler);
    });

    RenderObject object;
    object.mesh = GetMesh("TestMesh");
    if(!object.mesh) LOG(DEBUG, "Failed to Get Mesh");

    object.material = GetMaterial("default");
    if(!object.material) LOG(DEBUG, "Failed to Get Material");

    vk::DescriptorSetAllocateInfo allocInfo = {};
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &singleTextureSetLayout;

    CHECK_VK_RESULT(device.logical.allocateDescriptorSets(  &allocInfo, &object.material->textureSet), "Failed to allocate Descriptor Set");

    vk::DescriptorImageInfo imageBufferInfo = {};
	imageBufferInfo.sampler = blockySampler;
	imageBufferInfo.imageView = textures["empire_diffuse"].image.view;
	imageBufferInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::WriteDescriptorSet textureWrite = {};
    textureWrite.descriptorCount = 1;
    textureWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    textureWrite.dstSet = object.material->textureSet;
    textureWrite.dstBinding = 0;
    textureWrite.pImageInfo = &imageBufferInfo;
    
    // update descriptor sets
    device.logical.updateDescriptorSets(
        1, /* descriptor write count */
        &textureWrite, /* descriptor writes*/
        0, /* descriptor copy count */
        nullptr /* descriptor copies */
    );

    // add renderable
    renderables.push_back(object);
}

void GameZero::Renderer::InitDescriptors(){
    //create a descriptor pool that will hold 10 uniform buffers
	std::vector<vk::DescriptorPoolSize> sizes =
	{
		{ vk::DescriptorType::eUniformBuffer, 10 },
        { vk::DescriptorType::eCombinedImageSampler, 10}
	};

	vk::DescriptorPoolCreateInfo pool_info = {};
	pool_info.maxSets = 10;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	CHECK_VK_RESULT(device.logical.createDescriptorPool(&pool_info, nullptr, &descriptorPool), "Failed to create Descriptor Pool");
    // deletor
    PushFunction([=](){
        device.logical.destroyDescriptorPool(descriptorPool, nullptr);
    });
    LOG(INFO, "Created Descriptor Pool");

	//information about the binding.
	vk::DescriptorSetLayoutBinding camBufferBinding = {};
	camBufferBinding.binding = 0;
    // this variable is for passing array of data
	camBufferBinding.descriptorCount = 1;
	// it's a uniform buffer binding
	camBufferBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	// we use it from the vertex shader
	camBufferBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

	vk::DescriptorSetLayoutCreateInfo setInfo = {};
	setInfo.pNext = nullptr;
	//we are going to have 1 binding
	setInfo.bindingCount = 1;
	//point to the camera buffer binding
	setInfo.pBindings = &camBufferBinding;

    CHECK_VK_RESULT(device.logical.createDescriptorSetLayout(&setInfo, nullptr, &descriptorSetLayout), "Failed to create Descriptor Set Layout");
    // deletor
    PushFunction([=](){
        device.logical.destroyDescriptorSetLayout(descriptorSetLayout);
    });

    vk::DescriptorSetLayoutBinding textureBinding = {};
    textureBinding.binding = 0;
    textureBinding.descriptorCount = 1;
    textureBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    textureBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutCreateInfo textureSetLayoutInfo = {};
    textureSetLayoutInfo.bindingCount = 1;
    textureSetLayoutInfo.pBindings = &textureBinding;

    CHECK_VK_RESULT(device.logical.createDescriptorSetLayout(&textureSetLayoutInfo, nullptr, &singleTextureSetLayout), "Failed to create Descriptor Set Layout");
    // deletor
    PushFunction([=](){
        device.logical.destroyDescriptorSetLayout(singleTextureSetLayout);
        
    });

    for(auto& frame : frames){
        frame.cameraBuffer = CreateBuffer(device.allocator, sizeof(GPUCameraData), vk::BufferUsageFlagBits::eUniformBuffer, vma::MemoryUsage::eCpuToGpu);
        // deletor
        PushFunction([=](){
            device.allocator.destroyBuffer(frame.cameraBuffer.buffer, frame.cameraBuffer.allocation);
        });

        //allocate one descriptor set for each frame
		vk::DescriptorSetAllocateInfo allocInfo ={};
		allocInfo.pNext = nullptr;
		//using the pool we just set
		allocInfo.descriptorPool = descriptorPool;
		//only 1 descriptor
		allocInfo.descriptorSetCount = 1;
		//using the global data layout
		allocInfo.pSetLayouts = &descriptorSetLayout;

		CHECK_VK_RESULT(device.logical.allocateDescriptorSets(&allocInfo, &frame.descriptorSet), "Failed to allocate Descriptor Set");
    
        //information about the buffer we want to point at in the descriptor
		vk::DescriptorBufferInfo binfo = {};
		//it will be the camera buffer
		binfo.buffer = frame.cameraBuffer.buffer;
		//at 0 offset
		binfo.offset = 0;
		//of the size of a camera data struct
		binfo.range = sizeof(GPUCameraData);

		vk::WriteDescriptorSet setWrite = {};
		//we are going to write into binding number 0
		setWrite.dstBinding = 0;
		//of the global descriptor
		setWrite.dstSet = frame.descriptorSet;

		setWrite.descriptorCount = 1;
		//and the type is uniform buffer
		setWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		setWrite.pBufferInfo = &binfo;

        // update
		device.logical.updateDescriptorSets(
            1, /* write count */ 
            &setWrite, /* writes */
            0, /* copy count */
            nullptr /* copies */
        );
    }
}

void GameZero::Renderer::ImmediateSubmit(std::function<void (vk::CommandBuffer)> &&function){
    // allocate default command buffer that we will use for instant commands
    vk::CommandBufferAllocateInfo cmdBuffAllocInfo = {};
    cmdBuffAllocInfo.commandBufferCount = 1;
    cmdBuffAllocInfo.commandPool = uploadContext.commandPool;
    cmdBuffAllocInfo.level = vk::CommandBufferLevel::ePrimary;

    // allocate
    vk::CommandBuffer cmd;
    CHECK_VK_RESULT(device.logical.allocateCommandBuffers(&cmdBuffAllocInfo, &cmd), "Failed to allocate Command Buffer");
    
    // begin command buffer
    // we will use this command buffer only once, unlike in a draw command
    vk::CommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    // begin recording
    cmd.begin(cmdBeginInfo);

    // execute function
    function(cmd);

    // end recording
    cmd.end();   

    // submit info
    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 0;

    // submit to graphics queue
    CHECK_VK_RESULT(device.graphicsQueue.submit(1, &submitInfo, uploadContext.uploadFence), "Failed to submit Command Buffer");

    // wait for operations to complete
    // and then reset fence
    CHECK_VK_RESULT(device.logical.waitForFences(1, &uploadContext.uploadFence, VK_TRUE, 1e9), "Failed to wait for Fence");
    CHECK_VK_RESULT(device.logical.resetFences(1, &uploadContext.uploadFence), "Failed to reset Fence");

    // reset command pool
    device.logical.resetCommandPool(uploadContext.commandPool);
}

void GameZero::Renderer::UploadMeshToGPU(Mesh* mesh, bool useStaging){
	// no staging buffer
	if(!useStaging){
		vk::BufferCreateInfo  bufferInfo = {};
		bufferInfo.size = mesh->vertices.size() * sizeof(Vertex);
		bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;

		// cpu to gpu read is slow
		vma::AllocationCreateInfo allocInfo({}, vma::MemoryUsage::eCpuToGpu);

		// create buffer
		CHECK_VK_RESULT(device.allocator.createBuffer(&bufferInfo, &allocInfo, &mesh->vertexBuffer.buffer, &mesh->vertexBuffer.allocation, nullptr), "Failed to create vertex buffer");
        // deletor
        PushFunction([=](){
            device.allocator.destroyBuffer(mesh->vertexBuffer.buffer, mesh->vertexBuffer.allocation);
        });

		// copy vertex data
		void* data;
		device.allocator.mapMemory(mesh->vertexBuffer.allocation, &data);
		memcpy(data, mesh->vertices.data(), mesh->vertices.size() * sizeof(Vertex));
		device.allocator.unmapMemory(mesh->vertexBuffer.allocation);
	}else{
		// use staging buffer
		// staging buffer is basically a cpu only buffer copied to gpu only buffer
		// that gpu only buffer is called staged buffer
		const size_t bufferSize = mesh->vertices.size() * sizeof(Vertex);
		// allocate staging buffer
		vk::BufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.size = bufferSize;
		// this is source of transfer
		stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;

		// we want this data to be cpu readble only
		vma::AllocationCreateInfo allocInfo = {};
		allocInfo.usage = vma::MemoryUsage::eCpuOnly;
		
		AllocatedBuffer stagingBuffer;

		// allocate buffer
		CHECK_VK_RESULT(device.allocator.createBuffer(&stagingBufferInfo, &allocInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, nullptr), "Failed to create Staging Buffer");

		// copy data to staging buffer
		void *data;
		device.allocator.mapMemory(stagingBuffer.allocation, &data);
		memcpy(data, mesh->vertices.data(), bufferSize);
		device.allocator.unmapMemory(stagingBuffer.allocation);

		// create staged buffer
		vk::BufferCreateInfo stagedBufferInfo = {};
		stagedBufferInfo.size = bufferSize;
		// this is destination of a transfer operation and also a vertex buffer at the same time
		stagedBufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer  | vk::BufferUsageFlagBits::eTransferDst;

		// this buffer will be gpu only
		allocInfo.usage = vma::MemoryUsage::eGpuOnly;

		// allocate buffer
		CHECK_VK_RESULT(device.allocator.createBuffer(&stagedBufferInfo, &allocInfo, &mesh->vertexBuffer.buffer, &mesh->vertexBuffer.allocation, nullptr), "Failed to create Staged Buffer");
        // deletor
        PushFunction([=](){
            device.allocator.destroyBuffer(mesh->vertexBuffer.buffer, mesh->vertexBuffer.allocation);
        });

		// perform an immediate copy operation
        ImmediateSubmit([=](vk::CommandBuffer cmd){
            vk::BufferCopy copy(0, 0, bufferSize);
            cmd.copyBuffer(stagingBuffer.buffer, mesh->vertexBuffer.buffer, 1, &copy);
        });

        // after copy, destroy staging buffer as we dont need it anymore
        device.allocator.destroyBuffer(stagingBuffer.buffer, stagingBuffer.allocation);
	}
}

void GameZero::Renderer::LoadImages(){
    Texture texture;
    LoadImageFromFile(this, "../assets/textures/lost_empire-RGBA.png", texture.image);
    
    vk::ImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.format = vk::Format::eR8G8B8A8Srgb;
    imageViewInfo.image = texture.image.image;
    imageViewInfo.viewType = vk::ImageViewType::e2D;
    imageViewInfo.components.r = vk::ComponentSwizzle::eIdentity;
    imageViewInfo.components.g = vk::ComponentSwizzle::eIdentity;
    imageViewInfo.components.b = vk::ComponentSwizzle::eIdentity;
    imageViewInfo.components.a = vk::ComponentSwizzle::eIdentity;
    imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.levelCount = 1;

    // create image view
    texture.image.view = device.logical.createImageView(imageViewInfo);
    PushFunction([=](){
        device.logical.destroyImageView(texture.image.view);
    });

    textures["empire_diffuse"] = texture;
}