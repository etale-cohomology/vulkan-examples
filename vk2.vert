#version 450
layout(location=0) in  vec2 position;
layout(location=0) out vec2 fragCoord;

vec2 positions[] = vec2[](  // [-1..+1] x [-1..+1] in (x,-y) coordinates with origin (0,0) in the middle of the screen (ie. (-1,-1) is the top left corner of the screen)
	vec2(-1.0, +1.0),  // bottom left
	vec2(+1.0, +1.0),  // bottom right
	vec2(-1.0, -1.0),  // top    left
	vec2(+1.0, -1.0)   // top    right
);

void main(){
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	fragCoord   = gl_Position.xy;
}
