#version 450
layout(location=0) in  vec3 fragColor;  // the input variable does not necessarily have to use the same name as in the vshdr; they will be linked using the indexes specified by the @location directives
layout(location=0) out vec4 outColor;

void main() {
	outColor = vec4(fragColor, 1.0);
}
