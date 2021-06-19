#include "mesh.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "utils/assert.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"
#include "cstring"

// get vertex description
GameZero::VertexInputDescription GameZero::Vertex::GetVertexDescription(){
	VertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	vk::VertexInputBindingDescription mainBinding = {};
	// this data will be available from binding 0
	mainBinding.binding = 0;
	// after what no of bytes do you get next vertex 
	mainBinding.stride = sizeof(Vertex);
	// you get a vertex on moving stride number of bytes
	mainBinding.inputRate = vk::VertexInputRate::eVertex;

	description.bindings.push_back(mainBinding);

	//Position will be stored at Location 0
	vk::VertexInputAttributeDescription positionAttribute = {};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	// vec3
	positionAttribute.format = vk::Format::eR32G32B32Sfloat;
	positionAttribute.offset = offsetof(Vertex, position);

	//Normal will be stored at Location 1
	vk::VertexInputAttributeDescription normalAttribute = {};
	normalAttribute.binding = 0;
	normalAttribute.location = 1;
	// vec3
	normalAttribute.format = vk::Format::eR32G32B32Sfloat;;
	normalAttribute.offset = offsetof(Vertex, normal);

	//Color will be stored at Location 2
	vk::VertexInputAttributeDescription colorAttribute = {};
	colorAttribute.binding = 0;
	colorAttribute.location = 2;
	// vec3
	colorAttribute.format = vk::Format::eR32G32B32Sfloat;;
	colorAttribute.offset = offsetof(Vertex, color);

	//UV will be stored at Location 3
	vk::VertexInputAttributeDescription uvAttribute = {};
	uvAttribute.binding = 0;
	uvAttribute.location = 3;
	uvAttribute.format = vk::Format::eR32G32Sfloat;;
	uvAttribute.offset = offsetof(Vertex, uv);

	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(normalAttribute);
	description.attributes.push_back(colorAttribute);
	description.attributes.push_back(uvAttribute);

	return description;
}

// load mesh from an obj file
bool GameZero::Mesh::LoadMeshFromOBJ(const char *filename){
	//attrib will contain the vertex arrays of the file
	tinyobj::attrib_t attrib;
    //shapes contains the info for each separate object in the file
	std::vector<tinyobj::shape_t> shapes;
    //materials contains the information about the material of each shape, but we won't use it.
    std::vector<tinyobj::material_t> materials;

    //error and warning output from the load function
	std::string warn;
	std::string err;

    //load the OBJ file
	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, "../mesh/");
    //make sure to output the warnings to the console, in case there are issues with the file
	if (!warn.empty()) {
		LOG(WARNING, "%s", warn.c_str());
	}
    //if we have any error, print it to the console, and break the mesh loading.
    //This happens if the file can't be found or is malformed
	if (!err.empty()) {
		LOG(ERROR, "%s", err.c_str());
		return false;
	}

	LOG(INFO, "OBJ Mesh [%s] has %lu shapes and %lu materials", filename, shapes.size(), materials.size());

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

            //hardcode loading to triangles
			int fv = 3;

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                //vertex position
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                //vertex normal
            	tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

				// tinyobj::real_t cx = attrib.colors[3 * idx.vertex_index + 0];
				// tinyobj::real_t cy = attrib.colors[3 * idx.vertex_index + 1];
				// tinyobj::real_t cz = attrib.colors[3 * idx.vertex_index + 2];

                //copy it into our vertex
				Vertex new_vert;
				new_vert.position = glm::vec3(vx, vy, vz);
				new_vert.normal = glm::vec3(nx, ny, nz);
                // new_vert.color = glm::vec3(cx, cy, cz);
				// new_vert.color = glm::normalize(glm::vec3(rand(), rand(), rand()));
				new_vert.color = new_vert.normal;

				tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];

				new_vert.uv.x = ux;
				new_vert.uv.y = 1-uy;

				vertices.push_back(new_vert);
			}
			index_offset += fv;
		}
	}

    return true;
}
