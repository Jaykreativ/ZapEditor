#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location=0) out vec4 outColor;

layout(set = 0, binding = 0, rgba8) uniform readonly image2D srcImage;

//float weights[15] = {
//	1,
//	1,
//	1,
//	1,
//	1,
//	1,
//	1,
//	1,
//	1,
//	1,
//	1,
//	1,
//	1,
//	1,
//	1
//};

void main(){
	int kernelSize = 25;
	vec4 sum = vec4(0);
	int samples = 2 * kernelSize + 1;
	for (float x = 0; x < samples; x++)
	{
	    vec2 offset = vec2(x - kernelSize, 0);
	    sum += imageLoad(srcImage, ivec2(gl_FragCoord.xy+offset));
	}
	outColor = sum / samples;
	//vec4  texCol0 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0, -7)))*weights[0];
	//vec4  texCol1 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0, -6)))*weights[1];
	//vec4  texCol2 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0, -5)))*weights[2];
	//vec4  texCol3 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0, -4)))*weights[3];
	//vec4  texCol4 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0, -3)))*weights[4];
	//vec4  texCol5 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0, -2)))*weights[5];
	//vec4  texCol6 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0, -1)))*weights[6];
	//vec4  texCol7 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0,  0)))*weights[7];
	//vec4  texCol8 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0,  1)))*weights[8];
	//vec4  texCol9 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0,  2)))*weights[9];
	//vec4 texCol10 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0,  3)))*weights[10];
	//vec4 texCol11 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0,  4)))*weights[11];
	//vec4 texCol12 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0,  5)))*weights[12];
	//vec4 texCol13 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0,  6)))*weights[13];
	//vec4 texCol14 = imageLoad(srcImage, ivec2(gl_FragCoord.xy+vec2(0,  7)))*weights[14];
	//
	//outColor = (texCol0+texCol1+texCol2+texCol3+texCol4+texCol5+texCol6+texCol7+texCol8+texCol9+texCol10+texCol11+texCol12+texCol13+texCol14)/14;
}