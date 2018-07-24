#pragma once

#include "GraphicsInterface.h"

namespace Graphics 
{
	class GraphicsInterface;
	class AssetImporter
	{
	public:
		AssetImporter(GraphicsInterface* graphics);
		~AssetImporter();
		bool Load(const char* path, Mesh* outMeshes,uint8_t& numMeshes);

	private:
		GraphicsInterface* mGraphicsInterface;
	};
}