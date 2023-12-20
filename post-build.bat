@echo OFF

echo "Compiling shaders..."
echo "Compiling: assets/shaders/Builtin.MaterialShader.vert.glsl ---> assets/shaders/Builtin.MaterialShader.vert.spv"
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=vert assets/shaders/Builtin.MaterialShader.vert.glsl -o assets/shaders/Builtin.MaterialShader.vert.spv
if %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

echo "Compiling: assets/shaders/Builtin.MaterialShader.frag.glsl ---> assets/shaders/Builtin.MaterialShader.frag.spv"
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=frag assets/shaders/Builtin.MaterialShader.frag.glsl -o assets/shaders/Builtin.MaterialShader.frag.spv
if %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

echo "Compiling: assets/shaders/Builtin.UIShader.vert.glsl ---> assets/shaders/Builtin.UIShader.vert.spv"
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=vert assets/shaders/Builtin.UIShader.vert.glsl -o assets/shaders/Builtin.UIShader.vert.spv
if %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

echo "Compiling: assets/shaders/Builtin.UIShader.frag.glsl ---> assets/shaders/Builtin.UIShader.frag.spv"
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=frag assets/shaders/Builtin.UIShader.frag.glsl -o assets/shaders/Builtin.UIShader.frag.spv
if %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

echo "Done."