#version 460
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
	vec4 gl_Position;
};

vec3 rect[6] = {
	vec3(-1, -1, 0),
	vec3( 1, -1, 0),
	vec3(-1,  1, 0),
	vec3(-1,  1, 0),
	vec3( 1, -1, 0),
	vec3( 1,  1, 0)
};

void main(){
	gl_Position = vec4(rect[gl_VertexIndex], 1);
}