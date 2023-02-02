CC  = tcc
TCC = tcc
GCC = gcc-8
T   = 

all: vk0 vk1 vk2

clean:
	rm -f vk0 vk1

vk0: vk0.c vk0.h  makefile
	$(T) $(CC) $< -o $@  -lvulkan  -lxcb
vk0tcc: vk0.c vk0.h  makefile
	$(T) $(TCC) $< -o $@  -lvulkan  -lxcb  # ${cflags} ${cnopie} ${cfast}
vk0gcc: vk0.c vk0.h  makefile
	$(T) $(GCC) $< -o $@  -lvulkan  -lxcb  # ${cflags} ${cnopie} ${cfast}
vk0shdr:
	$(T) /mnt/ssd2/vk/1.3.236.0/x86_64/bin/glslc vk0.vert -o vk0.vert.spv
	$(T) /mnt/ssd2/vk/1.3.236.0/x86_64/bin/glslc vk0.frag -o vk0.frag.spv

vk1: vk1.c vk1.h  makefile
	$(T) $(CC) $< -o $@  -lvulkan  -lxcb
vk1tcc: vk1.c vk1.h  makefile
	$(T) $(TCC) $< -o $@  -lvulkan  -lxcb  # ${cflags} ${cnopie} ${cfast}
vk1gcc: vk1.c vk1.h  makefile
	$(T) $(GCC) $< -o $@  -lvulkan  -lxcb  # ${cflags} ${cnopie} ${cfast}
vk1shdr:
	$(T) /mnt/ssd2/vk/1.3.236.0/x86_64/bin/glslc vk1.vert -o vk1.vert.spv
	$(T) /mnt/ssd2/vk/1.3.236.0/x86_64/bin/glslc vk1.frag -o vk1.frag.spv

vk2: vk2.c vk2.h vk2.vert vk2.frag  makefile
	$(T) $(CC) $< -o $@  -lvulkan  -lxcb
vk2tcc: vk2.c vk2.h vk2.vert vk2.frag  makefile
	$(T) $(TCC) $< -o $@  -lvulkan  -lxcb  # ${cflags} ${cnopie} ${cfast}
vk2gcc: vk2.c vk2.h vk2.vert vk2.frag  makefile
	$(T) $(GCC) $< -o $@  -lvulkan  -lxcb  # ${cflags} ${cnopie} ${cfast}
vk2shdr:
	$(T) /mnt/ssd2/vk/1.3.236.0/x86_64/bin/glslc vk2.vert -o vk2.vert.spv
	$(T) /mnt/ssd2/vk/1.3.236.0/x86_64/bin/glslc vk2.frag -o vk2.frag.spv
