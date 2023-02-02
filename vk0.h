/*
t tcc vk0.c -o vk0  -lxcb -lvulkan  &&  t ./vk0
*/

// ----------------------------------------------------------------------------------------------------------------------------# @blk1  kw
#include <stdio.h>
#include <string.h>

#define fdef   static           // function definition
#define fdefi  static __inline  // function definition, inline
#define fdefe  extern           // function definition, extern
#define tdef   typedef struct   // type     definition
#define edef   typedef enum     // enum     definition
#define udef   typedef union    // union    definition
#define cdef   static const     // constant definition

// ----------------------------------------------------------------------------------------------------------------------------# @blk1  types
#include <stdint.h>

typedef    uint8_t    u8;
typedef    uint16_t   u16;
typedef    uint32_t   u32;
typedef    uint64_t   u64;

typedef    int8_t     i8;
typedef    int16_t    i16;
typedef    int32_t    i32;
typedef    int64_t    i64;

typedef  float        f32;
typedef  double       f64;
typedef  float        float32_t;
typedef  double       float64_t;

// ----------------------------------------------------------------------------------------------------------------------------# @blk1
#include <errno.h>  // A @do{}while(0) expands into a `regular statement`, not a `compound statement` (as a @{} macro would)
#define M_SEP                            "-------------------------------------------------------------------------------------------------------------------------------\x1b[91m#\x1b[0m\n"
#define chk( st,...)do{  if(     (st)<= 0){    printf("\x1b[91mFAIL  \x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  ",                                       __FILE__,__LINE__,__func__);                        printf(""__VA_ARGS__); putchar(0x0a); }  }while(0)  // @meta  C     error convention ( 0:false, 1:true), value itself   for error codes. use a statement-expression so it can be used in bracket-less IF or IF-ELSE statements
#define chks(st,...)do{  if((i64)(st)==-1ll){  printf("\x1b[91mFAIL  \x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  \x1b[31m%d\x1b[91m:\x1b[33m%s\x1b[0m  ", __FILE__,__LINE__,__func__, errno,strerror(errno)); printf(""__VA_ARGS__); putchar(0x0a); }  }while(0)  // @meta  Linux error convention (-1:false, 0:true), implicit errno for error codes. use a statement-expression so it can be used in bracket-less IF or IF-ELSE statements

#define mfor(i, a,b)  for(i64 (i)=(a); (i)<(b); ++(i))  // default `for` loop! flexibility over speed, ie. i64 vs i32? As always, of course, u64 is out of the question for looping
#define fori(i, a,b)  for(i32 (i)=(a); (i)<(b); ++(i))  // fastest `for` loop: signed yields a faster loop than unsigned because there's no need for overflow checks (or something), and i32 is faster than i64
#define foru(i, a,b)  for(u32 (i)=(a); (i)<(b); ++(i))  // 2nd fastest `for` loop

#define mmin( a,b)     ({  typeof(a) _a=(a);  typeof(b) _b=(b);  _a<_b ?  _a : _b;  })  // @typeof() is useful w/ `statement expressions`. Here's how they can be used to define a safe macro which operates on any arithmetic type and `evaluates each of its arguments exactly once`
#define mmax( a,b)     ({  typeof(a) _a=(a);  typeof(b) _b=(b);  _a<_b ?  _b : _a;  })  // @typeof() is useful w/ `statement expressions`. Here's how they can be used to define a safe macro which operates on any arithmetic type and `evaluates each of its arguments exactly once`
#define clamp(x, a,b)  mmin(mmax((x), (a)), (b))

#define arridim(ARR)  (sizeof((ARR)) / sizeof((*(ARR))))

// ----------------------------------------------------------------------------------------------------------------------------# @blk1
#include <time.h>
tdef{ u64 t0,t1; }dt_t;
fdefi u64  dt_abs()         {  struct timespec ep; clock_gettime(CLOCK_REALTIME, &ep);  return ep.tv_sec*1000000000ull + ep.tv_nsec;  }  // Get time in NANOSECONDS (@dt_abs()) or SECONDS w/ NANOSECOND-resolution (@dt_del())! *Must* use 64-bit counters!  NOTE! clock_gettime() is very slow (50ns); time(NULL) is 25s)! CLOCK_MONOTONIC_COARSE is too coarse!  NOTE! Use CLOCK_REALTIME, not CLOCK_MONOTONIC, for @dt_abs(), since @loop depends on it for a precise Unix time
fdefi f64  dt_del( dt_t  dt){  return (dt.t1 - dt.t0) / 1e9;  }                                                                          // Get `relative time`, ie. a `time differential/delta/difference` between 2 absolute times! The time delta is returned in seconds, and its resolution is in nanoseconds!
fdefi dt_t dt_ini()         {  return (dt_t){t0:dt_abs(), t1:0ull};  }
fdefi void dt_end( dt_t* dt){  dt->t1 = dt_abs();  }
fdefi void dt_show(dt_t  dt){  printf("  \x1b[32m%0.6f \x1b[0ms\n", dt_del(dt));  }

// ----------------------------------------------------------------------------------------------------------------------------# @blk1
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
	u64                        t0;
}vk_t;

static VkBool32 fnDebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){  // VK_EXT_debug_utils
	printf("\x1b[31mVKDEBUG  \x1b[35m%02x %02x \x1b[33m%s\x1b[0m\n", messageSeverity,messageType, pCallbackData->pMessage); return VK_FALSE;
}

#define vkchk(st,...)do{  if((st)!=VK_SUCCESS){  printf("\x1b[91mFAIL  \x1b[31m%s\x1b[91m:\x1b[0mL\x1b[32m%d\x1b[91m:\x1b[34m%s\x1b[0m()  \x1b[31m%s\x1b[0m  ", __FILE__,__LINE__,__func__, st); printf(""__VA_ARGS__); putchar(0x0a); }  }while(0)  // @meta  Linux error convention (-1:false, 0:true), implicit errno for error codes. use a statement-expression so it can be used in bracket-less IF or IF-ELSE statements
