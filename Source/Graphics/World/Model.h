#pragma once

#include "Graphics/GraphicsInterface.h"
#include "Core/Math.h"
#include "Component.h"
#include "Materialnfo.h"

#include "glm/glm.hpp"

#include <string>
#include <map>
#include <vector>

namespace Graphics
{
	struct PosNorTanTexc_Vertex
	{
		float Position[3];
		float Normal[3];
		float Tangent[3];
		float Texcoord[2];
	};

	struct Mesh
	{
		BufferHandle VertexBuffer;
		uint32_t NumVertex;
		uint32_t VertexSize;
		BufferHandle IndexBuffer;
		uint32_t NumIndices;
		std::string Name;

		Math::AABBData AABB;
		Math::BSData BS;
		MaterialInfo DefaultMaterial;
	};

	struct Model
	{
		Mesh* Meshes;
		uint32_t NumMeshes;
	};

	class ModelFactory
	{
		friend Model;

	private:
		ModelFactory();
		~ModelFactory();
		ModelFactory(const ModelFactory& other);

	public:
		static ModelFactory* Get();
		// Load from source file (.fbx,.obj...)
		Model* LoadFromFile(std::string path, GraphicsInterface* graphicsInterface, glm::mat3 transform = glm::mat3());
		// Load model from asset file (binary)
		Model* LoadFromAsset(std::string path, GraphicsInterface* graphicsInterface);

	private:
		std::map<std::string, Model*> mModelCache;
	};
}

namespace World
{
	class ModelComponent : public Component
	{
	public:
		ModelComponent();
		void Update(float deltaTime);
		void UpdateLate();

		void SetModel(Graphics::Model* model);
		Graphics::Model* GetModel()const;

		void SetMaterial(const MaterialInfo& materialInfo, uint32_t meshIdx = 0);
		const MaterialInfo& GetMaterial(uint32_t meshIdx = 0)const;

		Math::AABBData GetWorldAABB(uint32_t meshIdx = 0)const;
		Math::BSData GetWorldBS(uint32_t meshIdx = 0)const;

	private:
		Graphics::Model* mModel;
		struct MaterialSlot
		{
			bool Overriden = false;
			MaterialInfo Material;
		};
		std::vector<MaterialSlot> mMaterials;
		std::vector<Math::AABBData> mWorldAABB;
		std::vector<Math::BSData> mWorldBS;
	};
}