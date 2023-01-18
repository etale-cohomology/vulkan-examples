/*
t tcc vk1.c -o vk1  -lxcb -lvulkan  &&  t ./vk1
*/
// ----------------------------------------------------------------------------------------------------------------------------# @blk1  util
#include <stdio.h>
#include <string.h>

#define fdef   static           // function definition
#define fdefi  static __inline  // function definition, inline
#define fdefe  extern           // function definition, extern
#define tdef   typedef struct   // type     definition
#define edef   typedef enum     // enum     definition
#define udef   typedef union    // union    definition
#define cdef   static const     // constant definition

// ---------------------------------------------------------------- types
#include <stdint.h>
typedef    uint8_t    u8;
typedef    uint16_t   u16;
typedef    uint32_t   u32;
typedef    uint64_t   u64;
#if defined(__GNUC__) || defined(__clang__)
typedef  __uint128_t  u128;  // gcc/clang/ icc admit `__int128_t` and `__uint128_t`! tcc/msvc don't!
#endif

typedef    int8_t     i8;
typedef    int16_t    i16;
typedef    int32_t    i32;
typedef    int64_t    i64;
#if defined(__GNUC__) || defined(__clang__)
typedef  __int128_t   i128;  // gcc/clang/icc admit `__int128_t` and `__uint128_t`! tcc/mmsvc don't!
#endif

typedef  float        f32;
typedef  double       f64;
typedef  float        float32_t;
typedef  double       float64_t;
#if M_FP
typedef  __float128   f128;
typedef  __float128   float128_t;
#endif

// ---------------------------------------------------------------- misc
#include <errno.h>  // A @do{}while(0) expands into a `regular statement`, not a `compound statement` (as a @{} macro would)
#define M_SEP                            "-------------------------------------------------------------------------------------------------------------------------------\x1b[91m#\x1b[0m\n"
#define chk( st,...)do{  if(     (st)<= 0){    printf("\x1b[91mFAIL  \x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  ",                                       __FILE__,__LINE__,__func__);                        printf(""__VA_ARGS__); putchar(0x0a); }  }while(0)  // @meta  C     error convention ( 0:false, 1:true), value itself   for error codes. use a statement-expression so it can be used in bracket-less IF or IF-ELSE statements
#define chks(st,...)do{  if((i64)(st)==-1ll){  printf("\x1b[91mFAIL  \x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  \x1b[31m%d\x1b[91m:\x1b[33m%s\x1b[0m  ", __FILE__,__LINE__,__func__, errno,strerror(errno)); printf(""__VA_ARGS__); putchar(0x0a); }  }while(0)  // @meta  Linux error convention (-1:false, 0:true), implicit errno for error codes. use a statement-expression so it can be used in bracket-less IF or IF-ELSE statements
#define fail(...)   do{                        printf("\x1b[91mFAIL  \x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  ",                                       __FILE__,__LINE__,__func__);                        printf(""__VA_ARGS__); putchar(0x0a);    }while(0)  // @meta
#define warn(...)   do{                        printf("\x1b[31mWARN  \x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  ",                                       __FILE__,__LINE__,__func__);                        printf(""__VA_ARGS__); putchar(0x0a);    }while(0)  // @meta
#define meta(...)   do{                        printf(              "\x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  ",                                       __FILE__,__LINE__,__func__);                        printf(""__VA_ARGS__); putchar(0x0a);    }while(0)  // @meta
#define sep()                                  printf("\n"M_SEP)

#define mfor(i, a,b)  for(i64 (i)=(a); (i)<(b); ++(i))  // default `for` loop! flexibility over speed, ie. i64 vs i32? As always, of course, u64 is out of the question for looping
#define fori(i, a,b)  for(i32 (i)=(a); (i)<(b); ++(i))  // fastest `for` loop: signed yields a faster loop than unsigned because there's no need for overflow checks (or something), and i32 is faster than i64
#define foru(i, a,b)  for(u32 (i)=(a); (i)<(b); ++(i))  // 2nd fastest `for` loop

#define mmin( a,b)     ({  typeof(a) _a=(a);  typeof(b) _b=(b);  _a<_b ?  _a : _b;  })  // @typeof() is useful w/ `statement expressions`. Here's how they can be used to define a safe macro which operates on any arithmetic type and `evaluates each of its arguments exactly once`
#define mmax( a,b)     ({  typeof(a) _a=(a);  typeof(b) _b=(b);  _a<_b ?  _b : _a;  })  // @typeof() is useful w/ `statement expressions`. Here's how they can be used to define a safe macro which operates on any arithmetic type and `evaluates each of its arguments exactly once`
#define mabs( a)       ({  typeof(a) _a=(a);                     _a< 0 ? -_a : _a;  })  // @typeof() is useful w/ `statement expressions`. Here's how they can be used to define a safe macro which operates on any arithmetic type and `evaluates each of its arguments exactly once`
#define clamp(x, a,b)  mmin(mmax((x), (a)), (b))

#define arridim(ARR)  (sizeof((ARR)) / sizeof((*(ARR))))

// ---------------------------------------------------------------- time
#include <time.h>
tdef{ u64 t0,t1; }dt_t;
fdefi u64  dt_abs()         {  struct timespec ep; clock_gettime(CLOCK_REALTIME, &ep);  return ep.tv_sec*1000000000ull + ep.tv_nsec;  }  // Get time in NANOSECONDS (@dt_abs()) or SECONDS w/ NANOSECOND-resolution (@dt_del())! *Must* use 64-bit counters!  NOTE! clock_gettime() is very slow (50ns); time(NULL) is 25s)! CLOCK_MONOTONIC_COARSE is too coarse!  NOTE! Use CLOCK_REALTIME, not CLOCK_MONOTONIC, for @dt_abs(), since @loop depends on it for a precise Unix time
fdefi f64  dt_del( dt_t  dt){  return (dt.t1 - dt.t0) / 1e9;  }                                                                          // Get `relative time`, ie. a `time differential/delta/difference` between 2 absolute times! The time delta is returned in seconds, and its resolution is in nanoseconds!
fdefi dt_t dt_ini()         {  return (dt_t){t0:dt_abs(), t1:0ull};  }
fdefi void dt_end( dt_t* dt){  dt->t1 = dt_abs();  }
fdefi void dt_show(dt_t  dt){  printf("  \x1b[32m%0.6f \x1b[0ms\n", dt_del(dt));  }

// ---------------------------------------------------------------- file
#include <fcntl.h>     // @open()
#include <unistd.h>    // @ftruncate()
#include <sys/mman.h>  // @mmap()
#include <sys/stat.h>
#include <stdlib.h>    // @free()
#include <limits.h>
tdef{
	char*       path;
	struct stat fs;
	i64         bdim;  // @bdim is the main quantity, not @fs. @fs is only offered as a nicety, but, for consistency, @bdim should always exist
	u8*         data;
}file_t;

fdef file_t file_ini(char* path){  // NOTE! cuMemHostRegister() / cudaHostRegister() need the mem map to be read/write!
	int file_exists = access(path,F_OK);  if(file_exists==-1)  printf("\x1b[91mFAIL  \x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  \x1b[37m%s  \x1b[33m%p:\x1b[91m%s\x1b[0m\n", __FILE__,__LINE__,__func__, strerror(errno),path,path);  // NOTE! Follow slinks!
	if(file_exists<0) return (file_t){0x00};

	file_t file = {0x00};
	file.path   = realpath(path, NULL);
	chks(stat(path,&file.fs));  // NOTE! Follow slinks: @stat(), not @lstat()!
	int fd      = open(file.path, O_RDONLY);  if(fd<0) return file;  // O_RDONLY O_WRONLY O_RDWR
	file.bdim   = file.fs.st_size;
	if(!S_ISDIR(file.fs.st_mode) && file.bdim>0){
		file.data = (u8*)mmap(NULL,file.bdim, PROT_READ,MAP_SHARED, fd,0);  chks(file.data);  // PROT_READ PROT_WRITE PROT_EXEC PROT_NONE,  MAP_SHARED MAP_PRIVATE MAP_ANONYMOUS
	}
	chks(close(fd));  // `mmap` adds a reference to the file associated w/ the fd which is not removed by a `close` on that fd!
	return file;
}

fdef void file_end(file_t* file){  // printf("%016lx %016lx %ld %016lx\n", file, file->path, file->bdim,file->data);
	if(file==NULL) return;
	free(file->path);  // NOTE!
	if(file->data!=NULL)  chks(munmap(file->data,file->bdim));
	*file=(file_t){0x00};  // memset(file,0x00,sizeof(file_t));
}

// ----------------------------------------------------------------------------------------------------------------------------# @blk1  imports
#include <xcb/xcb.h>
#include <vulkan/vulkan.h>  // /usr/include/vulkan/vulkan.h /usr/include/vulkan/vulkan_core.h /usr/include/vulkan/vulkan_xcb.h
#include <vulkan/vulkan_xcb.h>

// ----------------------------------------------------------------------------------------------------------------------------# @blk1  xcb
fdefi xcb_screen_t* xcb_get_screen(xcb_connection_t* connection, int screen_idx){  // @meta  return a screen from its number!
	const xcb_setup_t* setup = xcb_get_setup(connection);
	for(xcb_screen_iterator_t screen_it = xcb_setup_roots_iterator(setup);  screen_it.rem;  --screen_idx, xcb_screen_next(&screen_it))
		if(screen_idx==0) return screen_it.data;
	return NULL;
}

#include <poll.h>
fdefi xcb_generic_event_t* xcb_ev_poll(xcb_connection_t* connection, int timeout_ms){  // TODO! `poll()` seems kinda buggy... maybe we should go back to `select()`? Although I seem to recall that `select()` is even worse... Maybe `epoll()`, then? But `epoll()` is so verbose...
	struct pollfd pfd = {0x00};
	pfd.events        = POLLIN;  // POLLIN: there's data to read!
	pfd.fd            = xcb_get_file_descriptor(connection);
	chks(poll(&pfd,1, timeout_ms));  // WARN! We CANNOT wait for ntriggers (the return value of @poll())! Otherwise we'll wait processing on events and the screen will go blank because glViewport() will not trigger! Hard to explain, but it happens to me!
	return xcb_poll_for_event(connection);
}

// ----------------------------------------------------------------------------------------------------------------------------# @blk1  vk
struct{  // shaders
	char*                 path;
	file_t                file;
	VkShaderStageFlagBits shaderStageFlagBits;  // vshdr, gshdr, tshdr, fshdr, etc.
	VkShaderModule        shaderModule;
}SHDRS[] = {
	{path:"vk0.vert.spv", shaderStageFlagBits:VK_SHADER_STAGE_VERTEX_BIT},
	{path:"vk0.frag.spv", shaderStageFlagBits:VK_SHADER_STAGE_FRAGMENT_BIT},
};

tdef{
	xcb_connection_t*          xcb_connection;
	xcb_screen_t*              xcb_screen;
	xcb_window_t               xcb_window;

	VkInstance                 instance;
	VkDebugUtilsMessengerEXT   debugUtilsMessenger;  // VK_EXT_debug_utils  // VkDebugReportCallbackEXT   debugReportCallback;  // VK_EXT_debug_report: deprecated
	VkPhysicalDevice           physicalDevice;
	u32                        graphicsQueueFamilyIndex;
	VkSurfaceKHR               surface;  // a window, or render target?
	VkDevice                   device;
	VkQueue                    graphicsQueue;
	VkQueue                    presentQueue;

	VkSwapchainKHR             swapchain;
	VkFormat                   swapchainFormat;
	VkExtent2D                 swapchainExtent;
	u32                        swapchainImageCount;
	VkRenderPass               renderPass;
	VkPipelineLayout           graphicsPipelineLayout;
	VkPipelineCache            graphicsPipelineCache;
	VkPipeline                 graphicsPipeline;
	// VkImageView                swapchainImageViews[];
	// VkFramebuffer              swapchainFramebuffers[];

	VkCommandPool              commandPool;
	VkCommandBuffer            commandBuffer;
	VkSemaphore                imgrdySemaphore;
	VkSemaphore                drawdoneSemaphore;
	VkFence                    drawFence;

	i32                        is_running;
	xcb_keycode_t              keycode;
	u8                         mouse_key;
	u16                        mouse_state;
	i16                        mouse_x, mouse_y;
}vk_t;

#if 0
static VkBool32 fnDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, u32 object, size_t location, i32 messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData){  // VK_EXT_debug_report
	printf("\x1b[31mVKDEBUG  \x1b[31m%s \x1b[32m%s\x1b[0m\n", pLayerPrefix,pMessage); return VK_FALSE;
}
#endif

static VkBool32 fnDebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){  // VK_EXT_debug_utils
	printf("\x1b[31mVKDEBUG  \x1b[35m%02x %02x \x1b[33m%s\x1b[0m\n", messageSeverity,messageType, pCallbackData->pMessage); return VK_FALSE;
}

#define vkchk(st,...)do{  if((st)!=VK_SUCCESS){  printf("\x1b[91mFAIL  \x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  \x1b[31m%s\x1b[0m  ", __FILE__,__LINE__,__func__, VkResultCstr(st)); printf(""__VA_ARGS__); putchar(0x0a); }  }while(0)  // @meta  Linux error convention (-1:false, 0:true), implicit errno for error codes. use a statement-expression so it can be used in bracket-less IF or IF-ELSE statements
fdefi char* VkResultCstr(VkResult st){
	switch(st){
		case VK_SUCCESS:                                             return "VK_SUCCESS";                                            break;
		case VK_NOT_READY:                                           return "VK_NOT_READY";                                          break;
		case VK_TIMEOUT:                                             return "VK_TIMEOUT";                                            break;
		case VK_EVENT_SET:                                           return "VK_EVENT_SET";                                          break;
		case VK_EVENT_RESET:                                         return "VK_EVENT_RESET";                                        break;
		case VK_INCOMPLETE:                                          return "VK_INCOMPLETE";                                         break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:                            return "VK_ERROR_OUT_OF_HOST_MEMORY";                           break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:                          return "VK_ERROR_OUT_OF_DEVICE_MEMORY";                         break;
		case VK_ERROR_INITIALIZATION_FAILED:                         return "VK_ERROR_INITIALIZATION_FAILED";                        break;
		case VK_ERROR_DEVICE_LOST:                                   return "VK_ERROR_DEVICE_LOST";                                  break;
		case VK_ERROR_MEMORY_MAP_FAILED:                             return "VK_ERROR_MEMORY_MAP_FAILED";                            break;
		case VK_ERROR_LAYER_NOT_PRESENT:                             return "VK_ERROR_LAYER_NOT_PRESENT";                            break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:                         return "VK_ERROR_EXTENSION_NOT_PRESENT";                        break;
		case VK_ERROR_FEATURE_NOT_PRESENT:                           return "VK_ERROR_FEATURE_NOT_PRESENT";                          break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:                           return "VK_ERROR_INCOMPATIBLE_DRIVER";                          break;
		case VK_ERROR_TOO_MANY_OBJECTS:                              return "VK_ERROR_TOO_MANY_OBJECTS";                             break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:                          return "VK_ERROR_FORMAT_NOT_SUPPORTED";                         break;
		case VK_ERROR_FRAGMENTED_POOL:                               return "VK_ERROR_FRAGMENTED_POOL";                              break;
		case VK_ERROR_UNKNOWN:                                       return "VK_ERROR_UNKNOWN";                                      break;
		case VK_ERROR_OUT_OF_POOL_MEMORY:                            return "VK_ERROR_OUT_OF_POOL_MEMORY";                           break;
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:                       return "VK_ERROR_INVALID_EXTERNAL_HANDLE";                      break;
		case VK_ERROR_FRAGMENTATION:                                 return "VK_ERROR_FRAGMENTATION";                                break;
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:                return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";               break;
		case VK_ERROR_SURFACE_LOST_KHR:                              return "VK_ERROR_SURFACE_LOST_KHR";                             break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                      return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";                     break;
		case VK_SUBOPTIMAL_KHR:                                      return "VK_SUBOPTIMAL_KHR";                                     break;
		case VK_ERROR_OUT_OF_DATE_KHR:                               return "VK_ERROR_OUT_OF_DATE_KHR";                              break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                      return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";                     break;
		case VK_ERROR_VALIDATION_FAILED_EXT:                         return "VK_ERROR_VALIDATION_FAILED_EXT";                        break;
		case VK_ERROR_INVALID_SHADER_NV:                             return "VK_ERROR_INVALID_SHADER_NV";                            break;
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:  return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT"; break;
		case VK_ERROR_NOT_PERMITTED_EXT:                             return "VK_ERROR_NOT_PERMITTED_EXT";                            break;
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:           return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";          break;
		// case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:                        return "VK_ERROR_OUT_OF_POOL_MEMORY_KHR";                       break;
		// case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:                   return "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR";                  break;
		// case VK_ERROR_FRAGMENTATION_EXT:                             return "VK_ERROR_FRAGMENTATION_EXT";                            break;
		// case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:                    return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";                   break;
		// case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR:            return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR";           break;
		// case VK_RESULT_BEGIN_RANGE:                                  return "VK_RESULT_BEGIN_RANGE";                                 break;
		// case VK_RESULT_END_RANGE:                                    return "VK_RESULT_END_RANGE";                                   break;
		// case VK_RESULT_RANGE_SIZE:                                   return "VK_RESULT_RANGE_SIZE";                                  break;
		case VK_RESULT_MAX_ENUM:                                     return "VK_RESULT_MAX_ENUM";                                    break;
	}
	return "VK_UNKNOWN";
}

char* VkFormatCstr(VkFormat fmt){
	switch(fmt){
		case VK_FORMAT_UNDEFINED:                                       return "VK_FORMAT_UNDEFINED";
		case VK_FORMAT_R4G4_UNORM_PACK8:                                return "VK_FORMAT_R4G4_UNORM_PACK8";
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:                           return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:                           return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
		case VK_FORMAT_R5G6B5_UNORM_PACK16:                             return "VK_FORMAT_R5G6B5_UNORM_PACK16";
		case VK_FORMAT_B5G6R5_UNORM_PACK16:                             return "VK_FORMAT_B5G6R5_UNORM_PACK16";
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:                           return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:                           return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16:                           return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
		case VK_FORMAT_R8_UNORM:                                        return "VK_FORMAT_R8_UNORM";
		case VK_FORMAT_R8_SNORM:                                        return "VK_FORMAT_R8_SNORM";
		case VK_FORMAT_R8_USCALED:                                      return "VK_FORMAT_R8_USCALED";
		case VK_FORMAT_R8_SSCALED:                                      return "VK_FORMAT_R8_SSCALED";
		case VK_FORMAT_R8_UINT:                                         return "VK_FORMAT_R8_UINT";
		case VK_FORMAT_R8_SINT:                                         return "VK_FORMAT_R8_SINT";
		case VK_FORMAT_R8_SRGB:                                         return "VK_FORMAT_R8_SRGB";
		case VK_FORMAT_R8G8_UNORM:                                      return "VK_FORMAT_R8G8_UNORM";
		case VK_FORMAT_R8G8_SNORM:                                      return "VK_FORMAT_R8G8_SNORM";
		case VK_FORMAT_R8G8_USCALED:                                    return "VK_FORMAT_R8G8_USCALED";
		case VK_FORMAT_R8G8_SSCALED:                                    return "VK_FORMAT_R8G8_SSCALED";
		case VK_FORMAT_R8G8_UINT:                                       return "VK_FORMAT_R8G8_UINT";
		case VK_FORMAT_R8G8_SINT:                                       return "VK_FORMAT_R8G8_SINT";
		case VK_FORMAT_R8G8_SRGB:                                       return "VK_FORMAT_R8G8_SRGB";
		case VK_FORMAT_R8G8B8_UNORM:                                    return "VK_FORMAT_R8G8B8_UNORM";
		case VK_FORMAT_R8G8B8_SNORM:                                    return "VK_FORMAT_R8G8B8_SNORM";
		case VK_FORMAT_R8G8B8_USCALED:                                  return "VK_FORMAT_R8G8B8_USCALED";
		case VK_FORMAT_R8G8B8_SSCALED:                                  return "VK_FORMAT_R8G8B8_SSCALED";
		case VK_FORMAT_R8G8B8_UINT:                                     return "VK_FORMAT_R8G8B8_UINT";
		case VK_FORMAT_R8G8B8_SINT:                                     return "VK_FORMAT_R8G8B8_SINT";
		case VK_FORMAT_R8G8B8_SRGB:                                     return "VK_FORMAT_R8G8B8_SRGB";
		case VK_FORMAT_B8G8R8_UNORM:                                    return "VK_FORMAT_B8G8R8_UNORM";
		case VK_FORMAT_B8G8R8_SNORM:                                    return "VK_FORMAT_B8G8R8_SNORM";
		case VK_FORMAT_B8G8R8_USCALED:                                  return "VK_FORMAT_B8G8R8_USCALED";
		case VK_FORMAT_B8G8R8_SSCALED:                                  return "VK_FORMAT_B8G8R8_SSCALED";
		case VK_FORMAT_B8G8R8_UINT:                                     return "VK_FORMAT_B8G8R8_UINT";
		case VK_FORMAT_B8G8R8_SINT:                                     return "VK_FORMAT_B8G8R8_SINT";
		case VK_FORMAT_B8G8R8_SRGB:                                     return "VK_FORMAT_B8G8R8_SRGB";
		case VK_FORMAT_R8G8B8A8_UNORM:                                  return "VK_FORMAT_R8G8B8A8_UNORM";
		case VK_FORMAT_R8G8B8A8_SNORM:                                  return "VK_FORMAT_R8G8B8A8_SNORM";
		case VK_FORMAT_R8G8B8A8_USCALED:                                return "VK_FORMAT_R8G8B8A8_USCALED";
		case VK_FORMAT_R8G8B8A8_SSCALED:                                return "VK_FORMAT_R8G8B8A8_SSCALED";
		case VK_FORMAT_R8G8B8A8_UINT:                                   return "VK_FORMAT_R8G8B8A8_UINT";
		case VK_FORMAT_R8G8B8A8_SINT:                                   return "VK_FORMAT_R8G8B8A8_SINT";
		case VK_FORMAT_R8G8B8A8_SRGB:                                   return "VK_FORMAT_R8G8B8A8_SRGB";
		case VK_FORMAT_B8G8R8A8_UNORM:                                  return "VK_FORMAT_B8G8R8A8_UNORM";
		case VK_FORMAT_B8G8R8A8_SNORM:                                  return "VK_FORMAT_B8G8R8A8_SNORM";
		case VK_FORMAT_B8G8R8A8_USCALED:                                return "VK_FORMAT_B8G8R8A8_USCALED";
		case VK_FORMAT_B8G8R8A8_SSCALED:                                return "VK_FORMAT_B8G8R8A8_SSCALED";
		case VK_FORMAT_B8G8R8A8_UINT:                                   return "VK_FORMAT_B8G8R8A8_UINT";
		case VK_FORMAT_B8G8R8A8_SINT:                                   return "VK_FORMAT_B8G8R8A8_SINT";
		case VK_FORMAT_B8G8R8A8_SRGB:                                   return "VK_FORMAT_B8G8R8A8_SRGB";
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:                           return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:                           return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:                         return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:                         return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:                            return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:                            return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:                            return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:                        return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:                        return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:                      return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:                      return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:                         return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:                         return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:                        return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:                        return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:                      return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:                      return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:                         return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:                         return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
		case VK_FORMAT_R16_UNORM:                                       return "VK_FORMAT_R16_UNORM";
		case VK_FORMAT_R16_SNORM:                                       return "VK_FORMAT_R16_SNORM";
		case VK_FORMAT_R16_USCALED:                                     return "VK_FORMAT_R16_USCALED";
		case VK_FORMAT_R16_SSCALED:                                     return "VK_FORMAT_R16_SSCALED";
		case VK_FORMAT_R16_UINT:                                        return "VK_FORMAT_R16_UINT";
		case VK_FORMAT_R16_SINT:                                        return "VK_FORMAT_R16_SINT";
		case VK_FORMAT_R16_SFLOAT:                                      return "VK_FORMAT_R16_SFLOAT";
		case VK_FORMAT_R16G16_UNORM:                                    return "VK_FORMAT_R16G16_UNORM";
		case VK_FORMAT_R16G16_SNORM:                                    return "VK_FORMAT_R16G16_SNORM";
		case VK_FORMAT_R16G16_USCALED:                                  return "VK_FORMAT_R16G16_USCALED";
		case VK_FORMAT_R16G16_SSCALED:                                  return "VK_FORMAT_R16G16_SSCALED";
		case VK_FORMAT_R16G16_UINT:                                     return "VK_FORMAT_R16G16_UINT";
		case VK_FORMAT_R16G16_SINT:                                     return "VK_FORMAT_R16G16_SINT";
		case VK_FORMAT_R16G16_SFLOAT:                                   return "VK_FORMAT_R16G16_SFLOAT";
		case VK_FORMAT_R16G16B16_UNORM:                                 return "VK_FORMAT_R16G16B16_UNORM";
		case VK_FORMAT_R16G16B16_SNORM:                                 return "VK_FORMAT_R16G16B16_SNORM";
		case VK_FORMAT_R16G16B16_USCALED:                               return "VK_FORMAT_R16G16B16_USCALED";
		case VK_FORMAT_R16G16B16_SSCALED:                               return "VK_FORMAT_R16G16B16_SSCALED";
		case VK_FORMAT_R16G16B16_UINT:                                  return "VK_FORMAT_R16G16B16_UINT";
		case VK_FORMAT_R16G16B16_SINT:                                  return "VK_FORMAT_R16G16B16_SINT";
		case VK_FORMAT_R16G16B16_SFLOAT:                                return "VK_FORMAT_R16G16B16_SFLOAT";
		case VK_FORMAT_R16G16B16A16_UNORM:                              return "VK_FORMAT_R16G16B16A16_UNORM";
		case VK_FORMAT_R16G16B16A16_SNORM:                              return "VK_FORMAT_R16G16B16A16_SNORM";
		case VK_FORMAT_R16G16B16A16_USCALED:                            return "VK_FORMAT_R16G16B16A16_USCALED";
		case VK_FORMAT_R16G16B16A16_SSCALED:                            return "VK_FORMAT_R16G16B16A16_SSCALED";
		case VK_FORMAT_R16G16B16A16_UINT:                               return "VK_FORMAT_R16G16B16A16_UINT";
		case VK_FORMAT_R16G16B16A16_SINT:                               return "VK_FORMAT_R16G16B16A16_SINT";
		case VK_FORMAT_R16G16B16A16_SFLOAT:                             return "VK_FORMAT_R16G16B16A16_SFLOAT";
		case VK_FORMAT_R32_UINT:                                        return "VK_FORMAT_R32_UINT";
		case VK_FORMAT_R32_SINT:                                        return "VK_FORMAT_R32_SINT";
		case VK_FORMAT_R32_SFLOAT:                                      return "VK_FORMAT_R32_SFLOAT";
		case VK_FORMAT_R32G32_UINT:                                     return "VK_FORMAT_R32G32_UINT";
		case VK_FORMAT_R32G32_SINT:                                     return "VK_FORMAT_R32G32_SINT";
		case VK_FORMAT_R32G32_SFLOAT:                                   return "VK_FORMAT_R32G32_SFLOAT";
		case VK_FORMAT_R32G32B32_UINT:                                  return "VK_FORMAT_R32G32B32_UINT";
		case VK_FORMAT_R32G32B32_SINT:                                  return "VK_FORMAT_R32G32B32_SINT";
		case VK_FORMAT_R32G32B32_SFLOAT:                                return "VK_FORMAT_R32G32B32_SFLOAT";
		case VK_FORMAT_R32G32B32A32_UINT:                               return "VK_FORMAT_R32G32B32A32_UINT";
		case VK_FORMAT_R32G32B32A32_SINT:                               return "VK_FORMAT_R32G32B32A32_SINT";
		case VK_FORMAT_R32G32B32A32_SFLOAT:                             return "VK_FORMAT_R32G32B32A32_SFLOAT";
		case VK_FORMAT_R64_UINT:                                        return "VK_FORMAT_R64_UINT";
		case VK_FORMAT_R64_SINT:                                        return "VK_FORMAT_R64_SINT";
		case VK_FORMAT_R64_SFLOAT:                                      return "VK_FORMAT_R64_SFLOAT";
		case VK_FORMAT_R64G64_UINT:                                     return "VK_FORMAT_R64G64_UINT";
		case VK_FORMAT_R64G64_SINT:                                     return "VK_FORMAT_R64G64_SINT";
		case VK_FORMAT_R64G64_SFLOAT:                                   return "VK_FORMAT_R64G64_SFLOAT";
		case VK_FORMAT_R64G64B64_UINT:                                  return "VK_FORMAT_R64G64B64_UINT";
		case VK_FORMAT_R64G64B64_SINT:                                  return "VK_FORMAT_R64G64B64_SINT";
		case VK_FORMAT_R64G64B64_SFLOAT:                                return "VK_FORMAT_R64G64B64_SFLOAT";
		case VK_FORMAT_R64G64B64A64_UINT:                               return "VK_FORMAT_R64G64B64A64_UINT";
		case VK_FORMAT_R64G64B64A64_SINT:                               return "VK_FORMAT_R64G64B64A64_SINT";
		case VK_FORMAT_R64G64B64A64_SFLOAT:                             return "VK_FORMAT_R64G64B64A64_SFLOAT";
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:                         return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:                          return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
		case VK_FORMAT_D16_UNORM:                                       return "VK_FORMAT_D16_UNORM";
		case VK_FORMAT_X8_D24_UNORM_PACK32:                             return "VK_FORMAT_X8_D24_UNORM_PACK32";
		case VK_FORMAT_D32_SFLOAT:                                      return "VK_FORMAT_D32_SFLOAT";
		case VK_FORMAT_S8_UINT:                                         return "VK_FORMAT_S8_UINT";
		case VK_FORMAT_D16_UNORM_S8_UINT:                               return "VK_FORMAT_D16_UNORM_S8_UINT";
		case VK_FORMAT_D24_UNORM_S8_UINT:                               return "VK_FORMAT_D24_UNORM_S8_UINT";
		case VK_FORMAT_D32_SFLOAT_S8_UINT:                              return "VK_FORMAT_D32_SFLOAT_S8_UINT";
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:                             return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:                              return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:                            return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:                             return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
		case VK_FORMAT_BC2_UNORM_BLOCK:                                 return "VK_FORMAT_BC2_UNORM_BLOCK";
		case VK_FORMAT_BC2_SRGB_BLOCK:                                  return "VK_FORMAT_BC2_SRGB_BLOCK";
		case VK_FORMAT_BC3_UNORM_BLOCK:                                 return "VK_FORMAT_BC3_UNORM_BLOCK";
		case VK_FORMAT_BC3_SRGB_BLOCK:                                  return "VK_FORMAT_BC3_SRGB_BLOCK";
		case VK_FORMAT_BC4_UNORM_BLOCK:                                 return "VK_FORMAT_BC4_UNORM_BLOCK";
		case VK_FORMAT_BC4_SNORM_BLOCK:                                 return "VK_FORMAT_BC4_SNORM_BLOCK";
		case VK_FORMAT_BC5_UNORM_BLOCK:                                 return "VK_FORMAT_BC5_UNORM_BLOCK";
		case VK_FORMAT_BC5_SNORM_BLOCK:                                 return "VK_FORMAT_BC5_SNORM_BLOCK";
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:                               return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:                               return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
		case VK_FORMAT_BC7_UNORM_BLOCK:                                 return "VK_FORMAT_BC7_UNORM_BLOCK";
		case VK_FORMAT_BC7_SRGB_BLOCK:                                  return "VK_FORMAT_BC7_SRGB_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:                         return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:                          return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:                       return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:                        return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:                       return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:                        return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
		case VK_FORMAT_EAC_R11_UNORM_BLOCK:                             return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
		case VK_FORMAT_EAC_R11_SNORM_BLOCK:                             return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
		case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:                          return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
		case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:                          return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:                            return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
		case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:                             return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:                            return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
		case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:                             return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:                            return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:                             return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:                            return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:                             return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:                            return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
		case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:                             return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:                            return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:                             return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:                            return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
		case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:                             return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:                            return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
		case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:                             return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:                           return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:                            return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:                           return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:                            return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:                           return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:                            return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:                          return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:                           return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:                          return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
		case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:                           return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:                          return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
		case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:                           return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
		case VK_FORMAT_G8B8G8R8_422_UNORM:                              return "VK_FORMAT_G8B8G8R8_422_UNORM";
		case VK_FORMAT_B8G8R8G8_422_UNORM:                              return "VK_FORMAT_B8G8R8G8_422_UNORM";
		case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:                       return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM";
		case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:                        return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM";
		case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:                       return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM";
		case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:                        return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM";
		case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:                       return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM";
		case VK_FORMAT_R10X6_UNORM_PACK16:                              return "VK_FORMAT_R10X6_UNORM_PACK16";
		case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:                        return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16";
		case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:              return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16";
		case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:          return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
		case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:          return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:      return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:       return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:      return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:       return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:      return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
		case VK_FORMAT_R12X4_UNORM_PACK16:                              return "VK_FORMAT_R12X4_UNORM_PACK16";
		case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:                        return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16";
		case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:              return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16";
		case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:          return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
		case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:          return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:      return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:       return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:      return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:       return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:      return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
		case VK_FORMAT_G16B16G16R16_422_UNORM:                          return "VK_FORMAT_G16B16G16R16_422_UNORM";
		case VK_FORMAT_B16G16R16G16_422_UNORM:                          return "VK_FORMAT_B16G16R16G16_422_UNORM";
		case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:                    return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM";
		case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:                     return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM";
		case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:                    return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM";
		case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:                     return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM";
		case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:                    return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM";
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:                     return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:                     return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:                     return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:                     return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:                      return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:                      return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:                      return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:                      return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:                       return "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:                       return "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:                       return "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:                       return "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:                       return "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:                       return "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:                       return "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:                       return "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:                      return "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:                      return "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:                      return "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:                     return "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:                     return "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT:                     return "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT";
		// case VK_FORMAT_G8B8G8R8_422_UNORM_KHR:                          return "VK_FORMAT_G8B8G8R8_422_UNORM_KHR";
		// case VK_FORMAT_B8G8R8G8_422_UNORM_KHR:                          return "VK_FORMAT_B8G8R8G8_422_UNORM_KHR";
		// case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR:                   return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR";
		// case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR:                    return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR";
		// case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR:                   return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR";
		// case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR:                    return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR";
		// case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR:                   return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR";
		// case VK_FORMAT_R10X6_UNORM_PACK16_KHR:                          return "VK_FORMAT_R10X6_UNORM_PACK16_KHR";
		// case VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR:                    return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR";
		// case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR:          return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR";
		// case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR:      return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR";
		// case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR:      return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR";
		// case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR:  return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR";
		// case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR:   return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR";
		// case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR:  return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR";
		// case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR:   return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR";
		// case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR:  return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR";
		// case VK_FORMAT_R12X4_UNORM_PACK16_KHR:                          return "VK_FORMAT_R12X4_UNORM_PACK16_KHR";
		// case VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR:                    return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR";
		// case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR:          return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR";
		// case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR:      return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR";
		// case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR:      return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR";
		// case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR:  return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR";
		// case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR:   return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR";
		// case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR:  return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR";
		// case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR:   return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR";
		// case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR:  return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR";
		// case VK_FORMAT_G16B16G16R16_422_UNORM_KHR:                      return "VK_FORMAT_G16B16G16R16_422_UNORM_KHR";
		// case VK_FORMAT_B16G16R16G16_422_UNORM_KHR:                      return "VK_FORMAT_B16G16R16G16_422_UNORM_KHR";
		// case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR:                return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR";
		// case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR:                 return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR";
		// case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR:                return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR";
		// case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR:                 return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR";
		// case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR:                return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR";
		// case VK_FORMAT_BEGIN_RANGE:                                     return "VK_FORMAT_BEGIN_RANGE";
		// case VK_FORMAT_END_RANGE:                                       return "VK_FORMAT_END_RANGE";
		// case VK_FORMAT_RANGE_SIZE:                                      return "VK_FORMAT_RANGE_SIZE";
	}
	return "VK_FORMAT_UNKNOWN";
}

char* VkColorSpaceKHRCstr(VkColorSpaceKHR colorSpace){
	switch(colorSpace){
		case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:          return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR";
		case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:    return "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT";
		case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:    return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT";
		case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:       return "VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT";
		case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:        return "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT";
		case VK_COLOR_SPACE_BT709_LINEAR_EXT:            return "VK_COLOR_SPACE_BT709_LINEAR_EXT";
		case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:         return "VK_COLOR_SPACE_BT709_NONLINEAR_EXT";
		case VK_COLOR_SPACE_BT2020_LINEAR_EXT:           return "VK_COLOR_SPACE_BT2020_LINEAR_EXT";
		case VK_COLOR_SPACE_HDR10_ST2084_EXT:            return "VK_COLOR_SPACE_HDR10_ST2084_EXT";
		case VK_COLOR_SPACE_DOLBYVISION_EXT:             return "VK_COLOR_SPACE_DOLBYVISION_EXT";
		case VK_COLOR_SPACE_HDR10_HLG_EXT:               return "VK_COLOR_SPACE_HDR10_HLG_EXT";
		case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:         return "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT";
		case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:      return "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT";
		case VK_COLOR_SPACE_PASS_THROUGH_EXT:            return "VK_COLOR_SPACE_PASS_THROUGH_EXT";
		case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT: return "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT";
		case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:          return "VK_COLOR_SPACE_DISPLAY_NATIVE_AMD";
	}
	return "VK_COLOR_SPACE_UNKNOWN";
}
