@echo off
"%VULKAN_SDK%\Bin\glslc.exe" shader_base.vert -o vert.spv
"%VULKAN_SDK%\Bin\glslc.exe" shader_base.frag -o frag.spv
pause