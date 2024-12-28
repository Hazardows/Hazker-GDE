#!/bin/bash

# Run from root directory!
mkdir -p bin/assets
mkdir -p bin/assets/shaders

echo "Compiling shaders..."

echo "assets/shaders/Builtin.objShader.vert.glsl -> bin\assets\shaders\Builtin.objShader.vert.spv"
%VULKAN_SDK%\bin\glslc.exe -fshader-stage=vert assets/shaders/Builtin.objShader.vert.glsl -o bin\assets\shaders\Builtin.objShader.vert.spv
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0] 
then
echo "Error:"$ERRORLEVEL && exit
fi

echo "assets/shaders/Builtin.objShader.frag.glsl -> bin\assets\shaders\Builtin.objShader.frag.spv"
%VULKAN_SDK%\bin\glslc.exe -fshader-stage=vert assets/shaders/Builtin.objShader.frag.glsl -o bin\assets\shaders\Builtin.objShader.frag.spv
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0] 
then
echo "Error:"$ERRORLEVEL && exit
fi

echo "Copying assets..."
echo cp -R "assets" "bin"
cp -R "assets" "bin"

echo "Done."