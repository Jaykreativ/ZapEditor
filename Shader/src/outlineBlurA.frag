#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location=0) out vec4 outColor;

layout(set = 0, binding = 0, rgba8) uniform readonly image2D srcImage;

void main(){
	int kernelSize = 25;
	vec4 sum = vec4(0);
	int samples = 2 * kernelSize + 1;
	for (float y = 0; y < samples; y++)
	{
	    vec2 offset = vec2(0, y - kernelSize);
	    sum += imageLoad(srcImage, ivec2(gl_FragCoord.xy+offset));
	}
	outColor = sum / samples;
}