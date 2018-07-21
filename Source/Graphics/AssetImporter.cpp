#include "AssetImporter.h"
#include "GraphicsInterface.h"
#include "nlohmann/json.hpp"
#include <fstream>

using nlohmann::json;

namespace Graphics
{
	AssetImporter::AssetImporter(GraphicsInterface* graphics):
		mGraphicsInterface(graphics)
	{
	}

	AssetImporter::~AssetImporter()
	{
	}

	bool AssetImporter::Load(const char* path)
	{
		std::string cpath = "..\\..\\Assets\\Meshes\\" + std::string(path);
		std::ifstream assetStream(cpath);
		if (!assetStream.is_open())
		{
			return false;
		}
		json assetJson;
		assetStream >> assetJson;
		auto scene = assetJson["scenes"];
		auto nodes = scene["nodes"];
		for (auto node = scene["nodes"].begin(); node != scene.end(); ++node)
		{
			int a = 1;
		}
		return true;
	} 
}