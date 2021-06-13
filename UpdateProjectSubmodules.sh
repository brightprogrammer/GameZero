#!/bin/zsh

# --------------- WARNING --------- WARNING -----------------#
#           run this script from project root only           #
# -----------------------------------------------------------#

# set project root directory
PROJECT_ROOT_DIRECTORY=$PWD
# project submodule directory
PROJECT_SUBMODULE_DIRECTORY=$PROJECT_ROOT_DIRECTORY/3rdparty
# number of threads that make command will use
THREADS_PER_BUILD=2

# change to project submodule directory
cd $PROJECT_SUBMODULE_DIRECTORY
git submodule update --recursive
cd $PROJECT_ROOT_DIRECTORY

# function to build submodules
# first param : submodule name
# following params are cmake defines
BuildSubmodule(){
    # store submodule name
    SUBMODULE_NAME=$1
    shift; # shift to get cmake defines

    echo "Building Submodule $SUBMODULE_NAME"

    # change to submodule dir
    cd $PROJECT_SUBMODULE_DIRECTORY

    # change to submodule directory
    CURRENT_SUBMODULE_DIRECTORY=$PROJECT_SUBMODULE_DIRECTORY/$SUBMODULE_NAME
    cd $CURRENT_SUBMODULE_DIRECTORY

    # make build directory
    mkdir -pv build
    cd build
    rm -fv CMakeCache.txt

    # store cmake command in one var
    CMAKE_GENERATE_COMMAND="cmake .. "
    
    # append defines 
    for CMAKE_DEFINE in "$*"
    do
        CMAKE_GENERATE_COMMAND="$CMAKE_GENERATE_COMMAND $CMAKE_DEFINE"
    done

    # execute cmake command
    echo "Generated CMake Comamnd : $CMAKE_GENERATE_COMMAND"
    eval $CMAKE_GENERATE_COMMAND

    # build 
    echo "Starting Submodule $SUBMODULE_NAME Build"
    make -j$THREADS_PER_BUILD
    echo "$SUBMODULE_NAME Built"

    # install
    echo "Installing Submodule $SUBMODULE_NAME"
    make install
    echo "Installing Submodule $SUBMODULE_NAME -- DONE"

    # go back to submodule dir
    cd $CURRENT_SUBMODULE_DIRECTORY

    # remove build dir
    rm -rv build

    # go back to root directory
    cd $PROJECT_ROOT_DIRECTORY

    echo "Building Submodule $SUBMODULE_NAME -- DONE"
}

# # build Vulkan-Header
# BuildSubmodule Vulkan-Headers -DCMAKE_INSTALL_PREFIX=$PROJECT_ROOT_DIRECTORY

# # build Vulkan-Loader
# BuildSubmodule Vulkan-Loader -DCMAKE_INSTALL_PREFIX=$PROJECT_ROOT_DIRECTORY -DVULKAN_HEADERS_INSTALL_DIR=$PROJECT_ROOT_DIRECTORY

# # build SDL
# BuildSubmodule SDL -DCMAKE_INSTALL_PREFIX=$PROJECT_ROOT_DIRECTORY

# build vk-bootstrap
# BuildSubmodule vk-bootstrap -DCMAKE_INSTALL_PREFIX=$PROJECT_ROOT_DIRECTORY

# build glm
# BuildSubmodule glm -DCMAKE_INSTALL_PREFIX=$PROJECT_ROOT_DIRECTORY