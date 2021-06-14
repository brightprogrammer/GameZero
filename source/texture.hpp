#ifndef GAMEZERO_TEXTURE_HPP
#define GAMEZERO_TEXTURE_HPP

#include "common.hpp"
#include "vulkan_types.hpp"

namespace GameZero{

    bool LoadImageFromFile(struct Renderer *renderer, const char* file, AllocatedImage& outImage);

    struct Texture{
        AllocatedImage image;
        VkImageView imageView;
    };

}

#endif//GAMEZERO_TEXTURE_HPP