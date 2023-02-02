#include "vk0.h"

// ----------------------------------------------------------------------------------------------------------------------------# @blk1  entry
fdefe int main(){  // Pseudocode of what an application looks like. I've omitted most creation structures, almost all synchronisation and all error checking. This is not a copy-paste guide!
	vk_t vk;

	// ----------------------------------------------------------------------------------------------------------------------------# validation layer?
	u32               layerPropertiesCount;                  vkchk(vkEnumerateInstanceLayerProperties(&layerPropertiesCount, NULL));  // call with last arg NULL to get the count
	VkLayerProperties layerProperties[layerPropertiesCount]; vkchk(vkEnumerateInstanceLayerProperties(&layerPropertiesCount, layerProperties));  printf("layerPropertiesCount \x1b[34m%d\x1b[0m\n", layerPropertiesCount);  chk(layerPropertiesCount!=0, "layers not found");
	mfor(i, 0,layerPropertiesCount)  printf("layerProperties \x1b[32m%-35s \x1b[34m%s\x1b[0m\n", layerProperties[i].layerName, layerProperties[i].description);

	// ----------------------------------------------------------------------------------------------------------------------------# 0) instance
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {  // VK_EXT_debug_utils
		sType:           VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		messageSeverity: VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT|/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT|*/VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT|VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
		messageType:     VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT|VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT|VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		pfnUserCallback: fnDebugUtilsMessengerCallback,
		pUserData:       NULL,
	};

	vkchk(vkCreateInstance(&(VkInstanceCreateInfo){  // Note we activate the WSI instance extensions, provided by the ICD to allow us to create an xcb surface
		sType:                   VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,                  // VkStructureType
		pApplicationInfo:        &(VkApplicationInfo){  // app info is optional (ie. specify app name, version)
			sType:              VK_STRUCTURE_TYPE_APPLICATION_INFO,  // VkStructureType
			pNext:              &debugUtilsMessengerCreateInfo,      // const void*
			pApplicationName:   "abc",                               // const char*
			applicationVersion: VK_MAKE_VERSION(1,3,0),              // uint32_t
			pEngineName:        "def",                               // const char*
			engineVersion:      VK_MAKE_VERSION(1,3,0),              // uint32_t
			apiVersion:         VK_API_VERSION_1_0,                  // uint32_t
		},                                        // const VkApplicationInfo*
		enabledLayerCount:       0,                                                                            // u32
		ppEnabledLayerNames:     (const char*[]){"VK_LAYER_LUNARG_standard_validation"},                       // const char* const*. don't forget to disable the layers when done debugging? VK_LAYER_LUNARG_standard_validation VK_LAYER_KHRONOS_validation
		enabledExtensionCount:   3,                                                                            // u32
		ppEnabledExtensionNames: (const char*[]){"VK_KHR_surface","VK_KHR_xcb_surface","VK_EXT_debug_utils"},  // const char* const*. VK_KHR_surface  VK_KHR_xcb_surface  VK_EXT_debug_utils  VK_EXT_debug_report  VK_EXT_validation_features
	}, NULL, &vk.instance));                // 50 ms...

	// ----------------------------------------------------------------------------------------------------------------------------# 1) debug
	// VK_EXT_debug_utils
	PFN_vkCreateDebugUtilsMessengerEXT  vkCreateDebugUtilsMessengerEXT  = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vk.instance,"vkCreateDebugUtilsMessengerEXT");
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk.instance,"vkDestroyDebugUtilsMessengerEXT");
	vkchk(vkCreateDebugUtilsMessengerEXT(vk.instance, &debugUtilsMessengerCreateInfo, NULL, &vk.debugUtilsMessenger));

	// ----------------------------------------------------------------------------------------------------------------------------# 2) surface
	int xcb_screen_idx;
	vk.xcb_connection = xcb_connect(NULL, &xcb_screen_idx);  if(xcb_connection_has_error(vk.xcb_connection)>0) printf("\x1b[31mWARN  \x1b[32mxcb  \x1b[91m%d  \x1b[0m\n", xcb_connection_has_error(vk.xcb_connection));  // st=xcb_connection_has_error(vk.xcb_connection); if(st>0) printf("\x1b[31mWARN  \x1b[32mxcb  \x1b[91m%s  \x1b[0m\n", XCB_LUT_CONN_ERRORS[st]);
	vk.xcb_screen     = xcb_get_screen(vk.xcb_connection, xcb_screen_idx);  // vk.xcb_screen->width_in_pixels, vk.xcb_screen->height_in_pixels

	vk.xcb_window = xcb_generate_id(vk.xcb_connection);
	xcb_create_window(vk.xcb_connection, vk.xcb_screen->root_depth, vk.xcb_window, vk.xcb_screen->root, 0,0,vk.xcb_screen->width_in_pixels/1-1,vk.xcb_screen->height_in_pixels/1, 0,XCB_WINDOW_CLASS_INPUT_OUTPUT, vk.xcb_screen->root_visual, XCB_CW_BACK_PIXMAP|XCB_CW_EVENT_MASK, (u32[]){XCB_BACK_PIXMAP_NONE, XCB_EVENT_MASK_KEY_PRESS|XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE|XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_STRUCTURE_NOTIFY});  // The values of this array MUST match the order of the enum where all the masks are defined!
	xcb_map_window(vk.xcb_connection, vk.xcb_window);
	xcb_flush(vk.xcb_connection);

	vkchk(vkCreateXcbSurfaceKHR(vk.instance, &(VkXcbSurfaceCreateInfoKHR){  // fetch vkCreateXcbSurfaceKHR() extension function pointer via vkGetInstanceProcAddr
		sType:      VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,  // VkStructureType
		connection: vk.xcb_connection,                              // xcb_connection_t*
		window:     vk.xcb_window,                                  // xcb_window_t
	}, NULL, &vk.surface));  // vkCreateXcbSurfaceKHR() vkCreateXlibSurfaceKHR() vkCreateWin32SurfaceKHR()

	// ----------------------------------------------------------------------------------------------------------------------------# 3) physical device
	u32              physicalDeviceCount;                  vkchk(vkEnumeratePhysicalDevices(vk.instance, &physicalDeviceCount, NULL));  // call with last arg NULL to get the count
	VkPhysicalDevice physicalDevices[physicalDeviceCount]; vkchk(vkEnumeratePhysicalDevices(vk.instance, &physicalDeviceCount, physicalDevices));
	printf("physicalDeviceCount \x1b[34m%d\x1b[0m\n", physicalDeviceCount);  chk(physicalDeviceCount=!0, "physical devices not found");

	VkPhysicalDeviceProperties physicalDeviceProperties;
	foru(i, 0,physicalDeviceCount){  // find a vk-ready device
		u32                     queueFamilyCount;                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, NULL);  printf("queueFamilyCount \x1b[32m%d\x1b[0m\n", queueFamilyCount);
		VkQueueFamilyProperties queueFamilies[queueFamilyCount]; vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilies);

		vk.graphicsQueueFamilyIndex = UINT32_MAX;
		foru(j, 0,queueFamilyCount){  // find a queue family that admits graphics operations
			VkBool32 supportsPresent; vkchk(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], j, vk.surface, &supportsPresent));
			if(supportsPresent && (queueFamilies[j].queueFlags&VK_QUEUE_GRAPHICS_BIT)){
				vk.physicalDevice           = physicalDevices[i];
				vk.graphicsQueueFamilyIndex = j;
				vkGetPhysicalDeviceProperties(vk.physicalDevice, &physicalDeviceProperties);
				break;
			}
		} chk(vk.graphicsQueueFamilyIndex!=UINT32_MAX, "vulkan-ready queue family not found");
		printf("graphicsQueueFamilyIndex \x1b[32m%d\x1b[0m\n", vk.graphicsQueueFamilyIndex);
		if(vk.physicalDevice)  break;
	} chk(vk.physicalDevice!=0, "physical device not found");
	printf("physicalDeviceProperties %x %x %x %x %s\n", physicalDeviceProperties.apiVersion,physicalDeviceProperties.driverVersion,physicalDeviceProperties.vendorID,physicalDeviceProperties.deviceID, physicalDeviceProperties.deviceName);

	// ----------------------------------------------------------------------------------------------------------------------------# 4) logical device
	vkchk(vkCreateDevice(vk.physicalDevice, &(VkDeviceCreateInfo){
		sType:                   VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,                    // VkStructureType
		queueCreateInfoCount:    2,                                                       // uint32_t
		pQueueCreateInfos:       (VkDeviceQueueCreateInfo[]){
			{
				sType:            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				queueFamilyIndex: vk.graphicsQueueFamilyIndex,
				queueCount:       1,
				pQueuePriorities: (f32[]){1.0},  // 1.0 is highest priority
			},
			{
				sType:            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				queueFamilyIndex: vk.graphicsQueueFamilyIndex,
				queueCount:       1,
				pQueuePriorities: (f32[]){1.0},  // 1.0 is highest priority
			},
		},
		enabledLayerCount:       0,                                                       // uint32_t
		ppEnabledLayerNames:     (const char*[]){"VK_LAYER_LUNARG_standard_validation"},  // const char* const*. don't forget to disable the layers when done debugging? VK_LAYER_LUNARG_standard_validation VK_LAYER_KHRONOS_validation
		enabledExtensionCount:   1,                                                       // uint32_t
		ppEnabledExtensionNames: (const char*[]){"VK_KHR_swapchain"},                     // const char* const*
		pEnabledFeatures:        NULL,                                                    // const VkPhysicalDeviceFeatures*
	}, NULL, &vk.device));
	vkGetDeviceQueue(vk.device, vk.graphicsQueueFamilyIndex,0, &vk.graphicsQueue);
	vkGetDeviceQueue(vk.device, vk.graphicsQueueFamilyIndex,0, &vk.presentQueue);

	// ----------------------------------------------------------------------------------------------------------------------------# 5) swapchain
	u32                surfaceFormatCount;                 vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physicalDevice, vk.surface, &surfaceFormatCount, NULL);
	VkSurfaceFormatKHR surfaceFormats[surfaceFormatCount]; vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physicalDevice, vk.surface, &surfaceFormatCount, surfaceFormats);
	printf("surfaceFormatCount \x1b[31m%d\x1b[0m\n", surfaceFormatCount);
	foru(i, 0,surfaceFormatCount)  printf("SurfaceFormat \x1b[32m%x \x1b[34m%x\x1b[0m\n", surfaceFormats[i].format, surfaceFormats[i].colorSpace);

	VkSurfaceCapabilitiesKHR surfaceCapabilities; vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.physicalDevice, vk.surface, &surfaceCapabilities);  printf("surfaceCapabilities \x1b[32m%d \x1b[34m%d \x1b[31m%d \x1b[32m%d\x1b[0m\n", surfaceCapabilities.currentExtent.width,surfaceCapabilities.currentExtent.height, surfaceCapabilities.minImageCount,surfaceCapabilities.maxImageCount);
	surfaceCapabilities.currentExtent.width  = surfaceCapabilities.currentExtent.width!=0xffffffff ? surfaceCapabilities.currentExtent.width  : 1920;
	surfaceCapabilities.currentExtent.height = surfaceCapabilities.currentExtent.width!=0xffffffff ? surfaceCapabilities.currentExtent.height : 1080;

	vk.swapchainFormat = surfaceFormatCount==1 && surfaceFormats[0].format==VK_FORMAT_UNDEFINED ? VK_FORMAT_B8G8R8_UNORM : surfaceFormats[0].format;  printf("swapchainFormat \x1b[35m%02x\x1b[0m\n",vk.swapchainFormat);  // surfaceFormatCount==1 && surfaceFormats[0].format==VK_FORMAT_UNDEFINED ? VK_FORMAT_B8G8R8_UNORM : surfaceFormats[0].format,
	vk.swapchainExtent = surfaceCapabilities.currentExtent;

	// u32              presentModeCount;               vkchk(vkGetPhysicalDeviceSurfacePresentModesKHR(vk.physicalDevice, vk.surface, &presentModeCount, NULL));
	// VkPresentModeKHR presentModes[presentModeCount]; vkchk(vkGetPhysicalDeviceSurfacePresentModesKHR(vk.physicalDevice, vk.surface, &presentModeCount, presentModes));
	// VkPresentModeKHR presentMode;  for(u32 i=0; i<presentModeCount; ++i){ if(presentModes[i]==VK_PRESENT_MODE_IMMEDIATE_KHR){ presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; break; } }

	vkchk(vkCreateSwapchainKHR(vk.device, &(VkSwapchainCreateInfoKHR){
		sType:                 VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // VkStructureType
		surface:               vk.surface,                                   // VkSurfaceKHR
		minImageCount:         clamp(2, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount),  // we are effectively looking for double-buffering. if surfaceCapabilities.maxImageCount==0 there is actually no limit on the number of images!  // surfaceCapabilities.maxImageCount==0 ? 2 : clamp(2, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount)
		imageFormat:           vk.swapchainFormat,                           // VkFormat. if the format list includes just one entry of VK_FORMAT_UNDEFINED, the surface has no preferred format, else at least one supported format will be returned
		imageColorSpace:       surfaceFormats[0].colorSpace,                 // VkColorSpaceKHR
		imageExtent:           surfaceCapabilities.currentExtent,            // VkExtent2D
		imageArrayLayers:      1,                                            // uint32_t
		imageUsage:            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,          // VkImageUsageFlags
		imageSharingMode:      VK_SHARING_MODE_EXCLUSIVE,                    // VkSharingMode
		queueFamilyIndexCount: 0,                                            // uint32_t
		pQueueFamilyIndices:   NULL,                                         // const uint32_t*
		preTransform:          surfaceCapabilities.supportedTransforms&VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfaceCapabilities.currentTransform,  // VkSurfaceTransformFlagBitsKHR
		compositeAlpha:        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,            // VkCompositeAlphaFlagBitsKHR
		presentMode:           VK_PRESENT_MODE_IMMEDIATE_KHR,                // VkPresentModeKHR. VK_PRESENT_MODE_IMMEDIATE_KHR  VK_PRESENT_MODE_MAILBOX_KHR  VK_PRESENT_MODE_FIFO_KHR  VK_PRESENT_MODE_FIFO_RELAXED_KHR  VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR  VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR  // VK_PRESENT_MODE_IMMEDIATE_KHR is the fastest, ie. no vsync?  // VK_PRESENT_MODE_FIFO_KHR should be always available
		clipped:               VK_TRUE,                                      // VkBool32
		oldSwapchain:          VK_NULL_HANDLE,                               // VkSwapchainKHR
	}, NULL, &vk.swapchain));  // the swap chain has been created now, so all that remains is retrieving the handles of the VkImages in it

	vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.swapchainImageCount, NULL);  printf("swapchainImageCount \x1b[92m%'u\x1b[0m\n", vk.swapchainImageCount);
	VkImage         swapchainImages[        vk.swapchainImageCount];
	VkFramebuffer   swapchainFramebuffers[  vk.swapchainImageCount];
	VkImageView     swapchainImageViews[    vk.swapchainImageCount];  // to use any VkImage, including those in the swap chain, in the render pipeline we must create a VkImageView object
	VkCommandBuffer swapchainCommandBuffers[vk.swapchainImageCount];
	vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.swapchainImageCount, swapchainImages);  // then we'll set up the imgs as render target

	// ----------------------------------------------------------------------------------------------------------------------------# 5) image views
	foru(i, 0,vk.swapchainImageCount){
		vkchk(vkCreateImageView(vk.device, &(VkImageViewCreateInfo){
			sType:            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			image:            swapchainImages[i],  // The viewType and format fields specify how the image data should be interpreted. The viewType parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps.
			viewType:         VK_IMAGE_VIEW_TYPE_2D,
			format:           vk.swapchainFormat,  // The components field allows you to swizzle the color channels around. For example, you can map all of the channels to the red channel for a monochrome texture. You can also map constant values of 0 and 1 to a channel. In our case we'll stick to the default mapping.
			components:       (VkComponentMapping){
				r: VK_COMPONENT_SWIZZLE_IDENTITY,
				g: VK_COMPONENT_SWIZZLE_IDENTITY,
				b: VK_COMPONENT_SWIZZLE_IDENTITY,
				a: VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			subresourceRange: (VkImageSubresourceRange){
				aspectMask:     VK_IMAGE_ASPECT_COLOR_BIT,
				baseMipLevel:   0,
				levelCount:     1,
				baseArrayLayer: 0,
				layerCount:     1
			},  // If you were working on a stereographic 3D application, then you would create a swap chain with multiple layers. You could then create multiple image views for each image representing the views for the left and right eyes by accessing different layers
		}, NULL, &swapchainImageViews[i]));  // an image view is sufficient to start using an image as a texture, but it's not quite ready to be used as a render target just yet. That requires one more step of indirection, known as a framebuffer
	}

	// ----------------------------------------------------------------------------------------------------------------------------# 7) render pass
	vkchk(vkCreateRenderPass(vk.device, &(VkRenderPassCreateInfo){
		sType:           VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		attachmentCount: 1,
		pAttachments:    &(VkAttachmentDescription){
			format:         vk.swapchainFormat,
			samples:        VK_SAMPLE_COUNT_1_BIT,
			loadOp:         VK_ATTACHMENT_LOAD_OP_CLEAR,
			storeOp:        VK_ATTACHMENT_STORE_OP_STORE,
			stencilLoadOp:  VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			stencilStoreOp: VK_ATTACHMENT_STORE_OP_DONT_CARE,
			initialLayout:  VK_IMAGE_LAYOUT_UNDEFINED,
			finalLayout:    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		},
		subpassCount:    1,
		pSubpasses:      &(VkSubpassDescription){
			pipelineBindPoint:    VK_PIPELINE_BIND_POINT_GRAPHICS,
			colorAttachmentCount: 1,
			pColorAttachments:    &(VkAttachmentReference){
				attachment: 0,
				layout:     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
		},
		dependencyCount: 1,
		pDependencies:   &(VkSubpassDependency){
			srcSubpass:    VK_SUBPASS_EXTERNAL,
			dstSubpass:    0,
			srcStageMask:  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			dstStageMask:  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			srcAccessMask: 0,
			dstAccessMask: VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},
	}, NULL, &vk.renderPass));

	// ----------------------------------------------------------------------------------------------------------------------------# 8) shaders
	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[arridim(SHDRS)];  foru(i, 0,arridim(SHDRS)){  // create shaders
		SHDRS[i].file = file_ini(SHDRS[i].path);

		vkCreateShaderModule(vk.device, &(VkShaderModuleCreateInfo){  // before we can pass the spir-v code to the pipeline, we must wrap it in a VkShaderModule object
			sType:    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			codeSize: SHDRS[i].file.bdim,
			pCode:    (void*)SHDRS[i].file.data,
		}, NULL, &SHDRS[i].shaderModule);  // shader modules are just a thin wrapper around the shader bytecode. compilation and linking of the SPIR-V bytecode to machine code for execution by the GPU doesn't happen until the graphics pipeline is created.

		pipelineShaderStageCreateInfos[i] = (VkPipelineShaderStageCreateInfo){  // to actually use the shaders we'll must assign them to a specific pipeline
			sType:               VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // The first step, besides the obligatory sType member, is telling Vulkan in which pipeline stage the shader is going to be used. There is an enum value for each of the programmable stages described in the previous chapter.
			stage:               SHDRS[i].shaderStageFlagBits,
			module:              SHDRS[i].shaderModule,
			pName:               "main",
			pSpecializationInfo: NULL,  // use to specify values for shader constants
		};
	}

	// ----------------------------------------------------------------------------------------------------------------------------# 9) graphics pipeline
	vkchk(vkCreatePipelineCache(vk.device, &(VkPipelineCacheCreateInfo){
		sType: VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
	}, NULL, &vk.graphicsPipelineCache));

	vkCreatePipelineLayout(vk.device, &(VkPipelineLayoutCreateInfo){
		sType:                  VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		setLayoutCount:         0,
		pSetLayouts:            NULL,
		pushConstantRangeCount: 0,
		pPushConstantRanges:    NULL,
	}, NULL, &vk.graphicsPipelineLayout);  // the old graphics APIs provided default state for most of the stages of the graphics pipeline. in Vulkan you must be explicit about most pipeline states as it'll be baked into an immutable pipeline state object. while most of the pipeline state needs to be baked into the pipeline state, a limited amount of the state can actually be changed without recreating the pipeline at draw time: eg. the size of the viewport, line width, blend constants. if you want to use dynamic state and keep these properties out, then you'll have to fill in a VkPipelineDynamicStateCreateInfo. This'll cause the config of these vals to be ignored and you'll have to specify the data at drawing time. This yields a more flexible setup and is very common for things like viewport/scissor state, which would yield a more complex setup when being baked into the pipeline state

	vkchk(vkCreateGraphicsPipelines(vk.device, vk.graphicsPipelineCache, 1,(VkGraphicsPipelineCreateInfo[]){{
		sType:               VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		stageCount:          arridim(SHDRS),
		pStages:             pipelineShaderStageCreateInfos,  // BUG tcc! tcc needs the array size?
		pVertexInputState:   &(VkPipelineVertexInputStateCreateInfo){  // describe the format of the vertex data that will be passed to the vshdr: bindings, attribute descriptions
			sType:                           VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			vertexBindingDescriptionCount:   0,
			pVertexBindingDescriptions:      NULL,
			vertexAttributeDescriptionCount: 0,
			pVertexAttributeDescriptions:    NULL,
		},
		pInputAssemblyState: &(VkPipelineInputAssemblyStateCreateInfo){  // what kind of geometry will be drawn from the vertices and if primitive restart should be enabled
			sType:                  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			topology:               VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,  // IMPORTANT!
			primitiveRestartEnable: VK_FALSE,
		},
		pTessellationState:  NULL,  // VkPipelineTessellationStateCreateInfo
		pViewportState:  &(VkPipelineViewportStateCreateInfo){
			sType:         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			viewportCount: 1,
			pViewports:    (VkViewport[]){{x:0.0, y:0.0, width:vk.swapchainExtent.width, height:vk.swapchainExtent.height, minDepth:0.0, maxDepth:1.0}},
			scissorCount:  1,
			pScissors:     (VkRect2D[]){{offset:{0,0}, extent:vk.swapchainExtent}},
		},
		pRasterizationState: &(VkPipelineRasterizationStateCreateInfo){
			sType:                   VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			depthClampEnable:        VK_FALSE,
			rasterizerDiscardEnable: VK_FALSE,  // NOTE! nice to have?
			polygonMode:             VK_POLYGON_MODE_FILL,
			cullMode:                VK_CULL_MODE_NONE,                // use VK_CULL_MODE_NONE to disable culling! now we don't have to worry about the winding of our triangles  // VK_CULL_MODE_BACK_BIT
			frontFace:               VK_FRONT_FACE_COUNTER_CLOCKWISE,  // IMPORTANT! but only if you set cullMode to something like VK_CULL_MODE_BACK_BIT
			depthBiasEnable:         VK_FALSE,
			depthBiasConstantFactor: 0.0,  // optional?
			depthBiasClamp:          0.0,  // optional?
			depthBiasSlopeFactor:    0.0,  // optional?
			lineWidth:               1.0,
		},
		pMultisampleState:   &(VkPipelineMultisampleStateCreateInfo){
			sType:                 VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			rasterizationSamples:  VK_SAMPLE_COUNT_1_BIT,
			sampleShadingEnable:   VK_FALSE,
			minSampleShading:      1.0,
			pSampleMask:           NULL,
			alphaToCoverageEnable: VK_FALSE,
			alphaToOneEnable:      VK_FALSE,
		},
		pDepthStencilState:  &(VkPipelineDepthStencilStateCreateInfo){  // optional
			sType:                 VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			depthTestEnable:       VK_FALSE,
			depthWriteEnable:      VK_FALSE,
			depthCompareOp:        VK_COMPARE_OP_LESS_OR_EQUAL,
			depthBoundsTestEnable: VK_FALSE,
			stencilTestEnable:     VK_FALSE,
			front:                 (VkStencilOpState){failOp:VK_STENCIL_OP_KEEP, passOp:VK_STENCIL_OP_KEEP, compareOp:VK_COMPARE_OP_ALWAYS},
			back:                  (VkStencilOpState){failOp:VK_STENCIL_OP_KEEP, passOp:VK_STENCIL_OP_KEEP, compareOp:VK_COMPARE_OP_ALWAYS},
		},
		pColorBlendState:    &(VkPipelineColorBlendStateCreateInfo){
			sType:           VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			logicOpEnable:   VK_FALSE,
			logicOp:         VK_LOGIC_OP_COPY,  // optional
			attachmentCount: 1,
			pAttachments:    &(VkPipelineColorBlendAttachmentState){
				blendEnable:         VK_FALSE,
				srcColorBlendFactor: VK_BLEND_FACTOR_SRC_ALPHA,            // optional
				dstColorBlendFactor: VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,  // optional
				colorBlendOp:        VK_BLEND_OP_ADD,                      // optional
				srcAlphaBlendFactor: VK_BLEND_FACTOR_ONE,                  // optional
				dstAlphaBlendFactor: VK_BLEND_FACTOR_ZERO,                 // optional
				alphaBlendOp:        VK_BLEND_OP_ADD,                      // optional
				colorWriteMask:      VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT,
			},
			blendConstants:  {0.0,0.0,0.0,0.0}, // Optional
		},
		pDynamicState:       &(VkPipelineDynamicStateCreateInfo){
			sType:             VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			dynamicStateCount: 2,  // viewport, scissor?
			pDynamicStates:    (VkDynamicState[]){VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR},
		},
		layout:              vk.graphicsPipelineLayout,
		renderPass:          vk.renderPass,
		subpass:             0,
		basePipelineHandle:  VK_NULL_HANDLE,
		basePipelineIndex:   0,
	}}, NULL, &vk.graphicsPipeline));  // vkCreateGraphicsPipelines() function has more parameters than the usual object creation functions in vk. it's designed to take multiple VkGraphicsPipelineCreateInfo's and create multiple VkPipeline's in a single call

	foru(i, 0,arridim(SHDRS)){
		file_end(&SHDRS[i].file);
		vkDestroyShaderModule(vk.device, SHDRS[i].shaderModule, NULL);  // the compilation and linking of the SPIR-V bytecode to machine code doesn't happen until the graphics pipeline is created. That means that we can destroy the shdr modules after the pipeline is created
	}

	// ----------------------------------------------------------------------------------------------------------------------------# 10) framebuffers
	foru(i, 0,vk.swapchainImageCount){
		vkchk(vkCreateFramebuffer(vk.device, &(VkFramebufferCreateInfo){
			sType:           VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			renderPass:      vk.renderPass,
			attachmentCount: 1,
			pAttachments:    &swapchainImageViews[i],
			width:           vk.swapchainExtent.width,
			height:          vk.swapchainExtent.height,
			layers:          1,
		}, NULL, &swapchainFramebuffers[i]));
	}

	// ----------------------------------------------------------------------------------------------------------------------------# 11) cmd pools & cmd bufs
	// to create command buffers we need to create a command pool
#if 0
	vkchk(vkCreateCommandPool(vk.device, &(VkCommandPoolCreateInfo){
		sType:            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		flags:            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,  // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
		queueFamilyIndex: vk.graphicsQueueFamilyIndex,
	}, NULL, &vk.commandPool));

	vkchk(vkAllocateCommandBuffers(vk.device, &(VkCommandBufferAllocateInfo){
		sType:              VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		commandPool:        vk.commandPool,
		level:              VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		commandBufferCount: 1,
	}, &vk.commandBuffer));  // cmd bufs are freed when their cmd pool is destroyed, no need to clean up
#endif

#if 1
	vkchk(vkCreateCommandPool(vk.device, &(VkCommandPoolCreateInfo){
		sType:            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		flags:            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,  // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
		queueFamilyIndex: vk.graphicsQueueFamilyIndex,
	}, NULL, &vk.commandPool));

	vkchk(vkAllocateCommandBuffers(vk.device, &(VkCommandBufferAllocateInfo){
		sType:              VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		commandPool:        vk.commandPool,
		level:              VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		commandBufferCount: vk.swapchainImageCount,
	}, swapchainCommandBuffers));  // cmd bufs are freed when their cmd pool is destroyed, no need to clean up

	foru(i, 0,vk.swapchainImageCount){  // record draw cmds to the cmd bufs
		vkchk(vkBeginCommandBuffer(swapchainCommandBuffers[i], &(VkCommandBufferBeginInfo){
			sType:            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			flags:            0,    // optional. VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT  VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT  VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
			pInheritanceInfo: NULL, // optional
		}));  // ini cmd buf recording. after call vkCmd*() fns

		vkCmdBeginRenderPass(swapchainCommandBuffers[i], &(VkRenderPassBeginInfo){  // start render pass. drawing starts by beginning the render pass with vkCmdBeginRenderPass
			sType:             VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			renderPass:        vk.renderPass,
			framebuffer:       swapchainFramebuffers[i],
			renderArea:        (VkRect2D){(VkOffset2D){0,0}, extent:vk.swapchainExtent},
			clearValueCount:   1,
			pClearValues:      &(VkClearValue){{{0.0f, 0.0f, 0.0f, 1.0f}}},
		}, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline( swapchainCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vk.graphicsPipeline);  // VK_PIPELINE_BIND_POINT_GRAPHICS  VK_PIPELINE_BIND_POINT_COMPUTE  VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
		vkCmdSetViewport(  swapchainCommandBuffers[i], 0, 1, &(VkViewport){0.0,0.0, vk.swapchainExtent.width,vk.swapchainExtent.height, 0.0,0.0});
		vkCmdSetScissor(   swapchainCommandBuffers[i], 0, 1, &(VkRect2D){offset:{0,0}, extent:vk.swapchainExtent});
		vkCmdDraw(         swapchainCommandBuffers[i], 4, 1, 0, 0);  // IMPORTANT! draw as many vertices as you're passing to the vshdr
		vkCmdEndRenderPass(swapchainCommandBuffers[i]);
		vkchk(vkEndCommandBuffer(swapchainCommandBuffers[i]));  // end cmd buf recording
	}
#endif

	// ----------------------------------------------------------------------------------------------------------------------------# 12) sync: semaphores, fences, barriers, events
	vkchk(vkCreateSemaphore(vk.device, &(VkSemaphoreCreateInfo){sType:VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO}, NULL, &vk.imgrdySemaphore));
	vkchk(vkCreateSemaphore(vk.device, &(VkSemaphoreCreateInfo){sType:VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO}, NULL, &vk.drawdoneSemaphore));
	vkchk(vkCreateFence(vk.device, &(VkFenceCreateInfo){
		sType: VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		flags: VK_FENCE_CREATE_SIGNALED_BIT,  // prevent the fence from waiting indefinitely
	}, NULL, &vk.drawFence));

	// ----------------------------------------------------------------------------------------------------------------------------# 13) main loop: acquire an img from the swap chain, record/execute a cmd buf, return the img to the swap chain
	vk.is_running = 1;
	vk.t0         = dt_abs();

	xcb_generic_event_t* xcb_ev = NULL;
	while(vk.is_running){  // Event loop!

		// ----------------------------------------------------------------------------------------------------------------------------
		// 0) ev handle!  // printf("%u %d %d\n", time(NULL), vk.draw_mode, vk.draw_mode_sleep);
		if(xcb_ev!=NULL)  free(xcb_ev);  // ALL events must be freed *every time they come*!
		xcb_ev = xcb_ev_poll(vk.xcb_connection, 16);  // 2.1) input-event handling!
		if(xcb_ev!=NULL){
			switch(xcb_ev->response_type & 0b01111111){
				// ----------------------------------------------------------------------------------------------------------------------------#
				case XCB_KEY_PRESS:{  xcb_key_press_event_t* ev_key_press = (xcb_key_press_event_t*)xcb_ev;  // `xcb_generic_event_t` is a polymorphic dstruct! The first 8 bits are an index into a LUT of binary layouts!
					vk.keycode = ev_key_press->detail;  printf("\x1b[34m%02x\x1b[0m\n", vk.keycode);  // NOTE! We need to store the keycode because it's part of the (relevant) state and we'll display on a table later!
					switch(vk.keycode){
						case 0x09: vk.is_running=0; break;  // This ensures we go through the renderpass clean-up!
						case 0x24: break;
					}
				}break;
				// ----------------------------------------------------------------------------------------------------------------------------#
				case XCB_KEY_RELEASE:{  xcb_key_release_event_t* ev_key_release = (xcb_key_release_event_t*)xcb_ev;  // `xcb_generic_event_t` is a polymorphic dstruct! The first 8 bits are an index into a LUT of binary layouts!
					continue;  // NOTE! A @continue is important for performance (otherwise half of our drawcalls are redundant)! BUT if you continue, then you MUST free the event here!
				}break;
				// ----------------------------------------------------------------------------------------------------------------------------#
				case XCB_BUTTON_PRESS:{  xcb_button_press_event_t* ev_button_press = (xcb_button_press_event_t*)xcb_ev;
					vk.mouse_key   = ev_button_press->detail;
					vk.mouse_state = ev_button_press->state;  // state&XCB_BUTTON_MASK_1 state&XCB_BUTTON_MASK_2 state&XCB_BUTTON_MASK_3 state&XCB_BUTTON_MASK_4 state&XCB_BUTTON_MASK_5
					vk.mouse_x     = ev_button_press->event_x;
					vk.mouse_y     = ev_button_press->event_y;
					switch(vk.mouse_key){
					}  // switch(mouse_key)
				}break;
				// ----------------------------------------------------------------------------------------------------------------------------#
				case XCB_BUTTON_RELEASE:{  xcb_button_release_event_t* ev_button_release = (xcb_button_release_event_t*)xcb_ev;
					vk.mouse_key   = ev_button_release->detail;
					vk.mouse_state = ev_button_release->state;  // state&XCB_BUTTON_MASK_1 state&XCB_BUTTON_MASK_2 state&XCB_BUTTON_MASK_3 state&XCB_BUTTON_MASK_4 state&XCB_BUTTON_MASK_5
					vk.mouse_x     = ev_button_release->event_x;
					vk.mouse_y     = ev_button_release->event_y;
					switch(vk.mouse_key){
					}  // switch(mouse_key)
				}break;
				// ----------------------------------------------------------------------------------------------------------------------------#
				case XCB_MOTION_NOTIFY:{  xcb_motion_notify_event_t* ev_motion_notify = (xcb_motion_notify_event_t*)xcb_ev;
					vk.mouse_key   = ev_motion_notify->detail;
					vk.mouse_state = ev_motion_notify->state;  // state&XCB_BUTTON_MASK_1 state&XCB_BUTTON_MASK_2 state&XCB_BUTTON_MASK_3 state&XCB_BUTTON_MASK_4 state&XCB_BUTTON_MASK_5
					vk.mouse_x     = ev_motion_notify->event_x;
					vk.mouse_y     = ev_motion_notify->event_y;
				}break;
				// ----------------------------------------------------------------------------------------------------------------------------#
				case XCB_EXPOSE:{  xcb_expose_event_t* ev_expose = (xcb_expose_event_t*)xcb_ev;  // We DO need XCB_EXPOSE! Maybe not when WE move the win (since XCB_CONFIGURE_NOTIFY takes care of that), but if another win occludes and then un-occludes this win, that's when XCB_EXPOSE is needed! On the other hand, XCB_MAP_NOTIFY is NOT needed, because XCB_EXPOSE takes care of that!
					if(ev_expose->count>0) break;  // Proceed only if this is the last expose event in the queue!
				}break;
				// ----------------------------------------------------------------------------------------------------------------------------#
				case XCB_MAP_NOTIFY:{  xcb_map_notify_event_t* ev_map_notify = (xcb_map_notify_event_t*)xcb_ev;  // Trigger when the win first pops up, and all the times afterwards!
				}break;
				// ----------------------------------------------------------------------------------------------------------------------------#
				case XCB_UNMAP_NOTIFY:{  xcb_unmap_notify_event_t* ev_unmap_notify = (xcb_unmap_notify_event_t*)xcb_ev;  // Trigger when the win pops down (aka. is minimized)!
				}break;
				// ----------------------------------------------------------------------------------------------------------------------------# No need to render on configure_notify because it always generates expose events, methinks!
				case XCB_CONFIGURE_NOTIFY:{  xcb_configure_notify_event_t* ev_configure_notify = (xcb_configure_notify_event_t*)xcb_ev;  // `xcb_generic_event_t` is a polymorphic dstruct! The first 8 bits are an index into a LUT of binary layouts!
				}break;
				// ----------------------------------------------------------------------------------------------------------------------------#
				default:{
					continue;
				}break;
			}  // END  switch(xcb_ev->response_type & 0b01111111)
		}  // END  if(xcb_ev!=NULL)

		// ----------------------------------------------------------------------------------------------------------------------------
		// 1) attr update!

		// ----------------------------------------------------------------------------------------------------------------------------
		// 2) draw!
#if 0
		u32 swapchainIdx;
		vkWaitForFences(vk.device, 1, &vk.drawFence, VK_TRUE, UINT64_MAX);  // at frame ini, wait until the prev frame has finished, so that the cmd buf and semaphores are available to use
		vkResetFences(  vk.device, 1, &vk.drawFence);  // after waiting, we need to manually reset the fence to the unsignaled state with the vkResetFences call:
		vkAcquireNextImageKHR(vk.device, vk.swapchain, UINT64_MAX, vk.imgrdySemaphore, VK_NULL_HANDLE, &swapchainIdx);
		vkResetCommandBuffer(vk.commandBuffer, 0);
		printf("\x1b[32m%.3f \x1b[0m%d\n", (dt_abs()-vk.t0)/1e9, swapchainIdx);

		// ----------------------------------------------------------------
		vkchk(vkBeginCommandBuffer(vk.commandBuffer, &(VkCommandBufferBeginInfo){
			sType:            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			flags:            0,    // optional. VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT  VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT  VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
			pInheritanceInfo: NULL, // optional
		}));  // ini cmd buf recording. after call vkCmd*() fns

		vkCmdBeginRenderPass(vk.commandBuffer, &(VkRenderPassBeginInfo){  // start render pass. drawing starts by beginning the render pass with vkCmdBeginRenderPass
			sType:             VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			renderPass:        vk.renderPass,
			framebuffer:       swapchainFramebuffers[swapchainIdx],
			renderArea:        (VkRect2D){(VkOffset2D){0,0}, extent:vk.swapchainExtent},
			clearValueCount:   1,
			pClearValues:      &(VkClearValue){{{0.0f, 0.0f, 0.0f, 1.0f}}},
		}, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline( vk.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.graphicsPipeline);
		vkCmdSetViewport(  vk.commandBuffer, 0, 1, &(VkViewport){0.0,0.0, vk.swapchainExtent.width,vk.swapchainExtent.height, 0.0,0.0});
		vkCmdSetScissor(   vk.commandBuffer, 0, 1, &(VkRect2D){offset:{0,0}, extent:vk.swapchainExtent});
		vkCmdDraw(         vk.commandBuffer, 4, 1, 0, 0);  // IMPORTANT! draw 4 vertices
		vkCmdEndRenderPass(vk.commandBuffer);
		vkchk(vkEndCommandBuffer(vk.commandBuffer));  // end cmd buf recording

		// ----------------------------------------------------------------
		// VkCommandBuffer A, B = ... // record command buffers
		// VkSemaphore S = ... // create a semaphore
		// vkQueueSubmit(work: A, signal: S, wait: None)  // enqueue A, signal S when done - starts executing immediately
		// vkQueueSubmit(work: B, signal: None, wait: S)  // enqueue B, wait on S to start
		vkchk(vkQueueSubmit(vk.graphicsQueue, 1, &(VkSubmitInfo){
			sType:                VK_STRUCTURE_TYPE_SUBMIT_INFO,
			waitSemaphoreCount:   1,
			pWaitSemaphores:      &(VkSemaphore){vk.imgrdySemaphore},
			pWaitDstStageMask:    &(VkPipelineStageFlags){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
			commandBufferCount:   1,
			pCommandBuffers:      &vk.commandBuffer,
			signalSemaphoreCount: 1,
			pSignalSemaphores:    &(VkSemaphore){vk.drawdoneSemaphore},
		}, vk.drawFence));

		vkQueuePresentKHR(vk.presentQueue, &(VkPresentInfoKHR){
			sType:              VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			waitSemaphoreCount: 1,
			pWaitSemaphores:    &(VkSemaphore){vk.drawdoneSemaphore},
			swapchainCount:     1,
			pSwapchains:        &(VkSwapchainKHR){vk.swapchain},
			pImageIndices:      &swapchainIdx,
			pResults:           NULL,  // optional
		});
#endif

#if 1
		u32 imageIndex;
		vkWaitForFences(      vk.device, 1, &vk.drawFence, VK_TRUE, UINT64_MAX);  // at frame ini, wait until the prev frame has finished, so that the cmd buf and semaphores are available to use
		vkResetFences(        vk.device, 1, &vk.drawFence);  // after waiting, we need to manually reset the fence to the unsignaled state with the vkResetFences call:
		vkAcquireNextImageKHR(vk.device, vk.swapchain, UINT64_MAX, vk.imgrdySemaphore, VK_NULL_HANDLE, &imageIndex);
		printf("\x1b[32m%.3f \x1b[0m%d\n", (dt_abs()-vk.t0)/1e9, imageIndex);

		vkchk(vkQueueSubmit(vk.graphicsQueue, 1, &(VkSubmitInfo){
			sType:                VK_STRUCTURE_TYPE_SUBMIT_INFO,
			waitSemaphoreCount:   1,
			pWaitSemaphores:      &(VkSemaphore){vk.imgrdySemaphore},
			pWaitDstStageMask:    &(VkPipelineStageFlags){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
			commandBufferCount:   1,
			pCommandBuffers:      &swapchainCommandBuffers[imageIndex],
			signalSemaphoreCount: 1,
			pSignalSemaphores:    &(VkSemaphore){vk.drawdoneSemaphore},
		}, vk.drawFence));

		vkQueuePresentKHR(vk.presentQueue, &(VkPresentInfoKHR){
			sType:              VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			waitSemaphoreCount: 1,
			pWaitSemaphores:    &(VkSemaphore){vk.drawdoneSemaphore},
			swapchainCount:     1,
			pSwapchains:        &(VkSwapchainKHR){vk.swapchain},
			pImageIndices:      &imageIndex,
			pResults:           NULL,  // optional
		});
#endif
	}  // END  while(vk.is_running)

	vkDeviceWaitIdle(vk.device);  // vkQueueWaitIdle(vk.queue);

	vkDeviceWaitIdle(vk.device);  // vkQueueWaitIdle(vk.queue);

	// ----------------------------------------------------------------------------------------------------------------------------#
	// Wait for everything to be done, and free objects
	xcb_destroy_window(vk.xcb_connection, vk.xcb_window);
	xcb_disconnect(vk.xcb_connection);

	vkDestroySemaphore(vk.device, vk.imgrdySemaphore, NULL);
	vkDestroySemaphore(vk.device, vk.drawdoneSemaphore, NULL);
	vkDestroyFence(    vk.device, vk.drawFence, NULL);

	vkDestroyCommandPool(vk.device, vk.commandPool, NULL);

	vkDestroyPipeline(      vk.device, vk.graphicsPipeline, NULL);
	vkDestroyPipelineCache( vk.device, vk.graphicsPipelineCache, NULL);
	vkDestroyPipelineLayout(vk.device, vk.graphicsPipelineLayout, NULL);

	vkDestroyRenderPass(vk.device, vk.renderPass, NULL);
	foru(i, 0,vk.swapchainImageCount){
		vkDestroyImageView(  vk.device, swapchainImageViews[i],   NULL);
		vkDestroyFramebuffer(vk.device, swapchainFramebuffers[i], NULL);
	}

	vkDestroySwapchainKHR(vk.device, vk.swapchain, NULL);
	vkDestroyDevice(      vk.device, NULL);

	vkDestroySurfaceKHR(            vk.instance, vk.surface, NULL);  // it's not called @vkDestroyXcbSurfaceKHR()
	vkDestroyDebugUtilsMessengerEXT(vk.instance, vk.debugUtilsMessenger, NULL);  // vkDestroyDebugReportCallbackEXT(vk.instance, vk.debugReportCallback, NULL);
	vkDestroyInstance(              vk.instance, NULL);
	return 0;
}
