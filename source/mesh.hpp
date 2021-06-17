/**
 * @file mesh.hpp
 * @author Siddharth Mishra
 * @brief contains mesh class and some extra types
 * @version 0.1
 * @date 2021-06-13
 * 
 * @copyright Copyright (c) 2021 Siddharth Mishra, All Rights Reserved
 * 
 */

#ifndef GAMEZERO_MESH_HPP
#define GAMEZERO_MESH_HPP

#include "utils.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan_types.hpp"

namespace GameZero {

    /// vertex input description describes vertex buffer data
    struct VertexInputDescription{
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    
        VkPipelineVertexInputStateCreateFlags flags = 0;
    };

    /// vertex
    struct Vertex{
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv;

        VertexInputDescription static GetVertexDescription();
    };

    /// mesh
    class Mesh{
    public:
        /// vertices of this mesh
        std::vector<Vertex> vertices;

        /// allocated buffer containing vertex data of this mesh
        AllocatedBuffer vertexBuffer;

        /**
        * @brief load mesh from obj file
        * 
        * @param filename : input filename
        */
        bool LoadMeshFromOBJ(const char* filename);
    };

}

#endif//GAMEZERO_MESH_HPP