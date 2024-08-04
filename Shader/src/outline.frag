#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location=0) out vec4 outColor;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput srcImage;

void main(){
	vec4 texCol = subpassLoad(srcImage);
	outColor = vec4(1, 0.5, 0.1, texCol.a);
}