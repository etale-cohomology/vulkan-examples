CC  = tcc
TCC = tcc
GCC = gcc-8
T   = 

all: vk0 vk1

clean:
	rm -f vk0 vk1

vk0: vk0.c vk0.h  makefile
	$(T) $(CC) vk0.c -o vk0  -lvulkan  -lxcb
vk0gcc: vk0.c vk0.h  makefile
	$(T) $(GCC) vk0.c -o vk0  -lvulkan  -lxcb  # ${cflags} ${cnopie} ${cfast}
vk0shdr:
	$(T) ${git}/vk/1.3.236.0/x86_64/bin/glslc vk0.vert -o vk0.vert.spv
	$(T) ${git}/vk/1.3.236.0/x86_64/bin/glslc vk0.frag -o vk0.frag.spv

vk1: vk1.c vk1.h  makefile
	$(T) $(CC) vk1.c -o vk1  -lvulkan  -lxcb
vk1gcc: vk1.c vk1.h  makefile
	$(T) $(GCC) vk1.c -o vk1  -lvulkan  -lxcb  # ${cflags} ${cnopie} ${cfast}
vk1shdr:
	$(T) ${git}/vk/1.3.236.0/x86_64/bin/glslc vk1.vert -o vk1.vert.spv
	$(T) ${git}/vk/1.3.236.0/x86_64/bin/glslc vk1.frag -o vk1.frag.spv
