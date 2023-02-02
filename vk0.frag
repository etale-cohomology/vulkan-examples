#version 450
layout(location=0) in  vec2 fragCoord;  // @in values need not have the same same as in the vshdr; yet they MUST have same @location
layout(location=0) out vec4 fragColor;

vec4 iResolution = vec4(1920,1080, 0,0);

// https://shadertoy.com/view/3ltSW2
float sdcircle(in vec2 uv, in vec2 center,in float radius){
	return length(center-uv) - radius;
}

// vec4 sdf_disk(vec2 uv, vec2 center,float radius, vec3 rgb){
// 	float d = length(center-uv) - radius;
// 	float t = clamp(d, 0.0,1.0);
// 	return vec4(rgb, 1.0-t);
// }

void main(){
	// vec2 uv = fragCoord.xy;  // [-1..+1] x [-1..+1] in (x,-y) coordinates with origin (0,0) in the middle of the screen (ie. (-1,-1) is the top left corner of the screen)

	// vec4 bg = vec4(0.1,0.1,0.1, 1.0);

	// vec2  center = vec2(0.0,0.0);
	// float radius = 0.01;
	// vec4  disk   = sdf_disk(uv, center,radius, vec3(0.0,0.5,1.0));

	// fragColor = mix(bg,disk, disk.a);

	vec2  p      = fragCoord * iResolution.xy/min(iResolution.x,iResolution.y);
	float circle = sdcircle(p, vec2(0,0),0.5);

	vec3 col = (circle>0.0) ? vec3(0.9,0.6,0.3) : vec3(0.65,0.85,1.0);  // coloring
	col     *= 1.0 - exp(-6.0*abs(circle));
	col     *= 0.8 + 0.2*cos(150.0*circle);
	col      = mix( col, vec3(1.0), 1.0-smoothstep(0.0,0.01,abs(circle)) );

	fragColor = vec4(col,1.0);
}
