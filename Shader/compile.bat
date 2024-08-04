cd %~dp0/..
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\outlinePlain.vert -o .\outlinePlain.vert.spv
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\outlinePlain.frag -o .\outlinePlain.frag.spv
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\outlineBlur.vert -o .\outlineBlur.vert.spv
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\outlineBlurA.frag -o .\outlineBlurA.frag.spv
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\outlineBlurB.frag -o .\outlineBlurB.frag.spv
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\outline.frag -o .\outline.frag.spv
pause