# description

![vk0 fragment shader renderer](vk0.png "vk0 fragment shader renderer")

This is a standalone, minimal set of `Vulkan` examples in `pure C` for `Linux/XCB`.

This isn't (just) a `hello triangle`. It sets up a [`Shadertoy`](https://www.shadertoy.com/view/3ltSW2)-like **fragment shader renderer** (which isn't much more complicated than `hello triangle`: draw [2 triangles](https://reddit.com/r/twotriangles/) instead of 1).

Many Vulkan tutorials or minimal examples use way too much boilerplate, are not standalone, support too many platforms, don't implement the `Shadertoy` renderer, and are in C++.
This is an attempt to remedy that.

It follows:  
https://jhenriques.net/development.html  
https://vulkan-tutorial.com  
https://github.com/SaschaWillems/Vulkan  

To compile an example (eg. `vk0`), run:

```
gcc vk0.c -o vk0 -lxcb -lvulkan  -Wno-format-zero-length
```

[Repent of your sins and be baptized in the name of Jesus Christ for the remission of sins.](https://bible.com/bible/1/jhn.3)  

# high-level picture

In order to draw a fragment-shaded polygon with `N` vertices, you can use (roughly) the following Vulkan/XCB API calls:

```python
# initialization
vkCreateInstance()            # create a Vulkan instance
xcb_connect()
xcb_create_window()
vkCreateXcbSurfaceKHR()       # create an Vulkan-XCB surface
vkEnumeratePhysicalDevices()  # get a physical device
vkCreateDevice()              # create a logical device
vkCreateSwapchainKHR()
vkCreateImageView()
vkCreateRenderPass()
vkCreateShaderModule()
vkCreatePipelineCache()
vkCreateGraphicsPipelines()   # the most complicated call in the whole program
vkCreateFramebuffer()
vkCreateCommandPool()
vkAllocateCommandBuffers()
vkCreateSemaphore()
vkCreateFence()

# render loop
vkWaitForFences()
vkResetFences()
vkAcquireNextImageKHR()
vkResetCommandBuffer()
vkBeginCommandBuffer()
vkCmdBeginRenderPass()
vkCmdBindPipeline()
vkCmdSetViewport()
vkCmdSetScissor()
vkCmdDraw()
vkCmdEndRenderPass()
vkEndCommandBuffer()
vkQueueSubmit()
vkQueuePresentKHR()
vkDeviceWaitIdle()

# clean-up
xcb_destroy_window()
xcb_disconnect()
vkDestroySemaphore()
vkDestroySemaphore()
vkDestroyFence()
vkDestroyCommandPool()
vkDestroyPipeline()
vkDestroyPipelineCache()
vkDestroyPipelineLayout()
vkDestroyRenderPass()
vkDestroyImageView()
vkDestroyFramebuffer()
vkDestroySwapchainKHR()
vkDestroyDevice()
vkDestroySurfaceKHR()
vkDestroyDebugUtilsMessengerEXT()
vkDestroyInstance()
```

# 2 Triangles

A fragment-shader-only, 2-triangle-only render is equivalent to a massively parallel for-loop over all pixels. At each pixel, compute the pixel's color independently, where each pixel knows its own position within the screen, but very little about its neighbors.
This is a very general setup, and allows you to implement a variety of computer graphics algorithms, like 2D SDF drawing, ray marching, or path tracing.

--------------------------------------------------------------------------------------------------------------------------------
# sources

https://registry.khronos.org/vulkan/  
https://registry.khronos.org/SPIR-V/  
https://registry.khronos.org/vulkan/specs/1.3/registry.html  
https://renderdoc.org/vulkan-in-30-minutes.html  
https://registry.khronos.org/vulkan/specs/1.3-extensions/pdf/vkspec.pdf  
https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html  

https://github.com/KhronosGroup/Vulkan-Samples  
https://github.com/KhronosGroup/Vulkan-Guide  
https://github.com/KhronosGroup/Vulkan-Loader
https://github.com/KhronosGroup/Vulkan-ValidationLayers  
https://github.com/KhronosGroup/SPIRV-Headers  
https://github.com/KhronosGroup/Vulkan-Samples/tree/master/samples/performance/swapchain_images  

https://intel.com/content/www/us/en/developer/articles/training/api-without-secrets-introduction-to-vulkan-part-1.html  
https://harrylovescode.gitbooks.io/vulkan-api/content/chap06/chap06.html  
https://renderdoc.org/vulkan-in-30-minutes.html  
https://momentsingraphics.de/ToyRenderer4RayTracing.html  

# notes

- vk shaders are all in spir-v binary
- vk is a stateless API
- command buffers
- commend queues
- vulkan xml API registry and scripts for processing it
- is this how you pass variable arguments to functions in C? by using custom arg struct for each function? not too bad! it's like a function signature spec, or something. you can definitely implement that in a C abstraction layer

- vulkan is a layered API. There is a core layer that we are calling into, but inbetween the API calls and the loader other "layers" can intercept the API calls. The ones we are interested in here are the validation layers that will help us debug and track problems with our usage of the API.
- you want to develop your app with this layers on but when shipping you should disable them
- a physical device represents 1 GPU
- a logical device is how the app keeps track of the physical device
- each physical device defines the number &ss type of queues it admits (eg. compute and graphics queues)
- vulkan render-passes use attachments to describe input and output render targets.
- don't forget to disable layers when done debugging?

- validation layers are optional components that hook into Vulkan fn calls to apply extra ops. common ops in validation layers are:
	- checking the values of parameters against the specification to detect misuse
	- tracking creation and destruction of objects to find resource leaks
	- checking thread safety by tracking the threads that calls originate from
	- logging every call and its parameters to the standard output
	- tracing Vulkan calls for profiling and replaying

- vulkan rendering:
	- wait for the previous frame to finish
	- acquire an image from the swap chain
	- record a command buffer which draws the scene onto that image
	- submit the recorded command buffer
	- present the swap chain image

- 2 semaphores in vk: binary & timeline

- draw triangle strips: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#drawing-triangle-strips

--------------------------------------------------------------------------------------------------------------------------------
# vk vulkan

## VK_KHR_surface
An instance extension. introduces VkSurfaceKHR objects, which abstract native platform surface or window objects to use with Vulkan.  
Provides a way to determine whether a queue family in a physical device supports presenting to particular surface.  
Separate extensions for each platform provide the mechanisms for creating VkSurfaceKHR objects, but once created they may be used in this and other platform-independent extensions, in particular the VK_KHR_swapchain extension.  

## VK_KHR_xcb_surface

Create a `VkSurfaceKHR` surface (from the `VK_KHR_surface` extension) that refers to an XCB window.

----------------------------------------------------------------
# misc

```shell
git init
gitc
git remote add origin https://github.com/etale-cohomology/vulkan-examples.git
git push -uf origin master

----------------------------------------------------------------
cd $git
git clone --recursive https://github.com/SaschaWillems/Vulkan.git  &&  cd Vulkan

mkcd build && cmake .. && make -j 28
t py download_assets.py

----------------------------------------------------------------
cd $git
git clone --recurse-submodules https://github.com/KhronosGroup/Vulkan-Samples  &&  cd Vulkan-Samples

cmake -G "Unix Makefiles" -H. -Bbuild/linux -DCMAKE_BUILD_TYPE=Release  &&  cmake --build build/linux --config Release --target vulkan_samples -- -j28  &&  ./build/linux/app/bin/Release/x86_64/vulkan_samples --help
```
