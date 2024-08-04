#version 460
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location=0) in vec3 vertPos;

layout( push_constant ) uniform PushConstants {
    uint instanceIndex;
} constants;

layout(binding=0) uniform UBO{
    mat4 perspective;
    mat4 view;
} ubo;

struct Material {
    vec3 albedo;
    uint albedoMap;
	float metallic;
    uint metallicMap;
	float roughness;
    uint roughnessMap;
	vec4 emissive;
	uint emissiveMap;
};

struct PerMeshInstanceData {
    mat4 transform;
    mat4 normalTransform;
    Material material;
    int unused[4];
};

layout(set=0, binding=1) readonly buffer PerMeshInstanceBuffer{
    PerMeshInstanceData data[];
} perMeshInstance;

void main(){
    vec4 worldPos = perMeshInstance.data[constants.instanceIndex].transform * vec4(vertPos, 1);
    vec4 viewPos = ubo.view * worldPos;
    gl_Position = ubo.perspective * vec4(viewPos.x, -viewPos.y, viewPos.z, 1);
}