#!/bin/sh

# configure this variable before building
PROJECT_ROOT_DIR=/home/brightprogrammer/Projects/GameEngine/GameZero

# make build directory if not present
cd $PROJECT_ROOT_DIR
mkdir -pv build # project bin dir

# first argument  : input shader filename
# second argument : output spirv filename
CompileShader(){
    InputName=$1
    OutputName=$2

    mkdir -pv $PROJECT_ROOT_DIR/build/shaders

    echo "compiling shader [ $InputName ]"
    glslc $PROJECT_ROOT_DIR/shaders/$InputName -o $PROJECT_ROOT_DIR/build/shaders/$OutputName
    echo "compiling shader [ $InputName ] - done"
}

# compile vertex shader
CompileShader shader.vert vert.spv

# compile fragment shader
CompileShader shader.frag frag.spv

# build
echo "building..."
cd build # in project root dir
rm -fv CMakeCache.txt # from project bin dir

# generate makefile
cmake .. -G Ninja
ninja
cd .. # project root

# make symbolic links
ln -sfv $PROJECT_ROOT_DIR/build/compile_commands.json $PROJECT_ROOT_DIR/compile_commands.json
ln -sfv $PROJECT_ROOT_DIR/build/GameZero $PROJECT_ROOT_DIR/GameZero