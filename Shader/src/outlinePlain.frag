#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location=0) out vec4 outColor;

void main(){
	outColor = vec4(1, 0.5, 0.1, 1);
}