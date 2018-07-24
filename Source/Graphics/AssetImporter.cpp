#include "AssetImporter.h"
#include "GraphicsInterface.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace Graphics
{
	AssetImporter::AssetImporter(GraphicsInterface* graphics):
		mGraphicsInterface(graphics)
	{
	}

	AssetImporter::~AssetImporter()
	{
	}

	struct FullVertex
	{
		glm::vec4 Position;	
		glm::vec4 Normal;	
		glm::vec4 Tangent;	
		glm::vec2 Texcoords;
	};

	bool AssetImporter::Load(const char* path,Mesh* outMeshes, uint8_t& numMeshes)
	{
		std::string fullPath = "..\\..\\Assets\\Meshes\\" + std::string(path);
		tinyobj::attrib_t attribs;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string log;

		if (!tinyobj::LoadObj(&attribs, &shapes, &materials, &log, fullPath.c_str()))
		{
			std::cout << log << std::endl;
		}
		if (log.size())
		{
			std::cout << log << std::endl;
		}

		std::vector<std::vector<FullVertex>> vertexDataShapes;
		vertexDataShapes.resize(shapes.size());
		int shapeIdx = 0;
		for (auto shapeIt = shapes.begin(); shapeIt != shapes.end(); ++shapeIt)
		{
			vertexDataShapes[shapeIdx].resize(shapeIt->mesh.indices.size() / 3);
			FullVertex* cur = &vertexDataShapes[shapeIdx][0];
			for (int i = 0; i < shapeIt->mesh.indices.size(); i += 3)
			{
				cur->Position.x = attribs.vertices[shapeIt->mesh.indices[i + 0].vertex_index];
				cur->Position.y = attribs.vertices[shapeIt->mesh.indices[i + 1].vertex_index];
				cur->Position.z = attribs.vertices[shapeIt->mesh.indices[i + 2].vertex_index];
				cur->Position.w = 0.0f;

				cur->Normal.x = attribs.normals[shapeIt->mesh.indices[i + 0].normal_index];
				cur->Normal.y = attribs.normals[shapeIt->mesh.indices[i + 1].normal_index];
				cur->Normal.z = attribs.normals[shapeIt->mesh.indices[i + 2].normal_index];
				cur->Normal.w = 0.0f;

				cur->Texcoords.x = attribs.texcoords[shapeIt->mesh.indices[i + 0].texcoord_index];
				cur->Texcoords.y = attribs.texcoords[shapeIt->mesh.indices[i + 1].texcoord_index];

				cur++;
			}
			shapeIdx++;
		}

		// Create the meshes 
		auto eleSize = sizeof(FullVertex);
		numMeshes = shapes.size();
		outMeshes = new Mesh[numMeshes];
		for (int i = 0; i < numMeshes; i++)
		{
			auto numEles = vertexDataShapes[i].size();
			outMeshes[i].VertexBuffer = mGraphicsInterface->CreateBuffer
			(
				BufferType::VertexBuffer, 
				CPUAccess::None, 
				numEles * eleSize, 
				vertexDataShapes[i].data());
		}

		return true;
	} 
}