#ifndef GAMEZERO_UTILS_VK_RESULT_STRING_HPP
#define GAMEZERO_UTILS_VK_RESULT_STRING_HPP

#include <vulkan/vulkan.hpp>

namespace GameZero{

    /**
     * @brief Converts vk::Result to it's string value according to vkspec
     * 
     * @param result : vk::Result returned by a function
     * @return const char* 
     */
    inline const char* VkResultString(vk::Result result){
        switch (result) {
            case vk::Result::eSuccess:
                return "SUCCESS";
            case vk::Result::eNotReady:
                return "NOT_READY";
            case vk::Result::eTimeout:
                return "TIMEOUT";
            case vk::Result::eEventSet:
                return "EVENT_SET";
            case vk::Result::eEventReset:
                return "EVENT_RESET";
            case vk::Result::eIncomplete:
                return "INCOMPLETE";
            case vk::Result::eErrorOutOfHostMemory:
                return "ERROR_OUT_OF_HOST_MEMORY";
            case vk::Result::eErrorOutOfDeviceMemory:
                return "ERROR_OUT_OF_DEVICE_MEMORY";
            case vk::Result::eErrorInitializationFailed:
                return "ERROR_INITIALIZATION_FAILED";
            case vk::Result::eErrorDeviceLost:
                return "ERROR_DEVICE_LOST";
            case vk::Result::eErrorMemoryMapFailed:
                return "ERROR_MEMORY_MAP_FAILED";
            case vk::Result::eErrorLayerNotPresent:
                return "ERROR_LAYER_NOT_PRESENT";
            case vk::Result::eErrorExtensionNotPresent:
                return "ERROR_EXTENSION_NOT_PRESENT";
            case vk::Result::eErrorFeatureNotPresent:
                return "ERROR_FEATURE_NOT_PRESENT";
            case vk::Result::eErrorIncompatibleDriver:
                return "ERROR_INCOMPATIBLE_DRIVER";
            case vk::Result::eErrorTooManyObjects:
                return "ERROR_TOO_MANY_OBJECTS";
            case vk::Result::eErrorFormatNotSupported:
                return "ERROR_FORMAT_NOT_SUPPORTED";
            case vk::Result::eErrorFragmentedPool:
                return "ERROR_FRAGMENTED_POOL";
            case vk::Result::eErrorUnknown:
                return "ERROR_UNKNOWN";
            case vk::Result::eErrorOutOfPoolMemory:
                return "ERROR_OUT_OF_POOL_MEMORY";
            case vk::Result::eErrorInvalidExternalHandle:
                return "ERROR_INVALID_EXTERNAL_HANDLE";
            case vk::Result::eErrorFragmentation:
                return "ERROR_FRAGMENTATION";
            case vk::Result::eErrorInvalidOpaqueCaptureAddress:
                return "ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            case vk::Result::eErrorSurfaceLostKHR:
                return "ERROR_SURFACE_LOST_KHR";
            case vk::Result::eErrorNativeWindowInUseKHR:
                return "ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case vk::Result::eSuboptimalKHR:
                return "SUBOPTIMAL_KHR";
            case vk::Result::eErrorOutOfDateKHR:
                return "ERROR_OUT_OF_DATE_KHR";
            case vk::Result::eErrorIncompatibleDisplayKHR:
                return "ERROR_INCOMPATIBLE_DISPLAY_KHR";
            case vk::Result::eErrorValidationFailedEXT:
                return "ERROR_VALIDATION_FAILED_EXT";
            case vk::Result::eErrorInvalidShaderNV:
                return "ERROR_INVALID_SHADER_NV";
    
    // #if VK_HEADER_VERSION >= 135 && VK_HEADER_VERSION < 162
            // case vk::Result::eErrorIncompatibleVersionKHR:
                // return "VK_ERROR_INCOMPATIBLE_VERSION_KHR";
    // #endif
    
            case vk::Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT:
                return "ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            case vk::Result::eErrorNotPermittedEXT:
                return "ERROR_NOT_PERMITTED_EXT";
                // return "ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            case vk::Result::eThreadIdleKHR:
                return "THREAD_IDLE_KHR";
            case vk::Result::eThreadDoneKHR:
                return "THREAD_DONE_KHR";
            case vk::Result::eOperationDeferredKHR:
                return "OPERATION_DEFERRED_KHR";
            case vk::Result::eOperationNotDeferredKHR:
                return "OPERATION_NOT_DEFERRED_KHR";
            case vk::Result::ePipelineCompileRequiredEXT:
                return "PIPELINE_COMPILE_REQUIRED_EXT";         
            default:
                return "UNKNOWN_ERROR_VALUE";
        }
    }

} // canvas namespace

#endif//GAMEZERO_UTILS_VK_RESULT_STRING_HPP