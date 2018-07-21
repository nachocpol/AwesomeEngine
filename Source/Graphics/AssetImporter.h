#pragma once

namespace Graphics 
{
	class GraphicsInterface;
	class AssetImporter
	{
	public:
		AssetImporter(GraphicsInterface* graphics);
		~AssetImporter();
		bool Load(const char* path);

	private:
		GraphicsInterface* mGraphicsInterface;
	};
}