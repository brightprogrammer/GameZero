/**
 * @file image.hpp
 * @author Siddharth Mishra (bshock665@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-17
 * 
 * @copyright Copyright 2021 Siddharth Mishra. All Rights Reserved
 * 
 */

#ifndef GAMEZERO_VULKAN_IMAGE_HPP
#define GAMEZERO_VULKAN_IMAGE_HPP

#include "vk_mem_alloc.hpp"
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.hpp"

namespace GameZero{

	/// a simple image wrapper with extra information
	struct Image{
		vk::Format format;
		vk::Extent3D extent;
		vk::Image image;
		vk::ImageView view;
	};
	
	/// represents manually allocated image
	struct AllocatedImage : public Image{
		vma::Allocation allocation;
	};
}

#endif//GAMEZERO_VULKAN_IMAGE_HPP