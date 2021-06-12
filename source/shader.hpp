#ifndef GAMEZERO_SHADER_HPP
#define GAMEZERO_SHADER_HPP

#include "common.hpp"
#include "utils/assert.hpp"
#include "utils/file.hpp"
#include "vulkan/vulkan_core.h"

namespace GameZero{

    [[nodiscard]] inline VkShaderModule LoadShaderModule(const VkDevice& device, const char* filename){
        // vulkan expects code to be in uin32_t* format
        std::vector<uint32_t> code;
        if(!ReadFile(code, filename, true)) return VK_NULL_HANDLE;

        VkShaderModuleCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.codeSize = code.size() * sizeof(uint32_t);
        shaderInfo.pCode = code.data();

        // create shader module
        VkShaderModule shader;
        CHECK_VK_RESULT(vkCreateShaderModule(device, &shaderInfo, nullptr, &shader), "Failed to create Vulkan Shader Module");

        return shader;
    }

};

#endif//GAMEZERO_SHADER_HPP