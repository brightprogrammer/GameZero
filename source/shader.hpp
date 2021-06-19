#ifndef GAMEZERO_SHADER_HPP
#define GAMEZERO_SHADER_HPP

#include "common.hpp"
#include "utils/assert.hpp"
#include "utils/file.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/device.hpp"

namespace GameZero{

    /**
     * @brief Loads a precompiled spir-v shader as a vulkan shader module
     * 
     * @param device : device that will use this shader
     * @param filename : filename/path of compiled spir-v shader code
     * @return vk::ShaderModule : cratedShaderModule
     */
    [[nodiscard]] inline vk::ShaderModule LoadShaderModule(const Device& device, const char* filename){
        // vulkan expects code to be in uint32_t* format
        std::vector<uint32_t> code;
        if(!ReadFile(code, filename, true)) return VK_NULL_HANDLE;

        // shader module create info
        vk::ShaderModuleCreateInfo shaderInfo(
            {}, /* flags */
            code.size() * sizeof(uint32_t), /* code size */
            code.data() /* code */
        );
        
        // create shader module
        return device.logical.createShaderModule(shaderInfo);
    }

};

#endif//GAMEZERO_SHADER_HPP