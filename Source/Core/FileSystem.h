#pragma once

#include <string>
#include <vector>

namespace Core
{
	struct FileDevice
	{
		struct Type
		{
			enum T
			{
				Assets,		
				ShaderSource,
				COUNT
			};
		};
		std::string RootPath;
		Type::T DeviceType;
		std::string Identifier; // assets
	};

	class FileSystem
	{
	private:
		FileSystem();
		FileSystem(const FileSystem& other);
		~FileSystem();

	public:
		static FileSystem* GetInstance();
		void Initialize();
		void AddFileDevice(FileDevice device);
		bool FixupPath(std::string& path);
		bool FileExists(const std::string& fixeupFile);

	private:

		std::vector<FileDevice> mDevices;
	};
}