#pragma once

#include "Graphics/GraphicsInterface.h"
#include "Core/Math.h"
#include "Component.h"

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
		Math::AABBData AABB;
		Math::BSData BS;
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
		Model* LoadFromFile(std::string path, GraphicsInterface* graphicsInterface);
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

		Math::AABBData GetWorldAABB(uint32_t meshIdx = 0)const;
		Math::BSData GetWorldBS(uint32_t meshIdx = 0)const;

	private:
		Graphics::Model* mModel;
		std::vector<Math::AABBData> mWorldAABB;
		std::vector<Math::BSData> mWorldBS;
	};
}