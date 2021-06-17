#include "surface_details.hpp"
#include <algorithm>

// constructor
GameZero::SurfaceDetails::SurfaceDetails(
    const vk::SurfaceKHR& surface,
    const vk::PhysicalDevice& physicalDevice
){
    surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    SelectBestSurfaceDetails(
        physicalDevice.getSurfaceFormatsKHR(surface),
        physicalDevice.getSurfacePresentModesKHR(surface)
    );
}

// constructor
GameZero::SurfaceDetails::SurfaceDetails(
    const vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
    const std::vector<vk::SurfaceFormatKHR>& availableSurfaceFormats,
    const std::vector<vk::PresentModeKHR>& availablePresentModes
) : surfaceCapabilities(surfaceCapabilities){
    SelectBestSurfaceDetails(availableSurfaceFormats, availablePresentModes);
}

// select best surface details
void GameZero::SurfaceDetails::SelectBestSurfaceDetails(
    const std::vector<vk::SurfaceFormatKHR>& availableSurfaceFormats,
    const std::vector<vk::PresentModeKHR>& availablePresentModes
){
    // select the first format as default if we didn't find the required present mode
    surfaceFormat = availableSurfaceFormats[0];

    // select best surface format
    for(const auto& sFormat : availableSurfaceFormats){
        if(
            sFormat.format == vk::Format::eB8G8R8A8Srgb &&
            sFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear
        ){
            surfaceFormat = sFormat;
            break;
        }
    }

    // fifo is always guaranteed
    presentMode = vk::PresentModeKHR::eFifo;

    // select best present mode
    for(const auto& pMode : availablePresentModes){
        if(pMode == vk::PresentModeKHR::eMailbox){
            presentMode = pMode;
            break;
        }
    }
}