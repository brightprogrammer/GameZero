#!/bin/sh

# make build directory if not present
mkdir -pv build

# first argument  : input shader filename
# second argument : output spirv filename
CompileShader(){
    InputName=$1
    OutputName=$2

    mkdir -pv build/shaders

    echo "compiling shader [ $InputName ]"
    glslc shaders/$InputName -o build/shaders/$OutputName
    echo "compiling shader [ $InputName ] - done"
}

# compile vertex shader
CompileShader shader.vert vert.spv

# compile fragment shader
CompileShader shader.frag frag.spv

# build game
cd build
rm -fv CMakeLists.txtecho "building game"

# generate makefile
cmake .. -G Ninja
cd ..
ln -sfv build/compile_commands.json compile_commands.json
# build
# ninja
# run game
# ./GameZero
# cd ..
