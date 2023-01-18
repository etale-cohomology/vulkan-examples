#version 450
layout(location=0) in  vec3 fragColor;  // @in values need not have the same same as in the vshdr; yet they MUST have same @location
layout(location=0) out vec4 outColor;

void main() {
	outColor = vec4(fragColor, 1.0);
}
