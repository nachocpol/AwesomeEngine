#include "FileSystem.h"
#include "Logging.h"

using namespace Core;

FileSystem::FileSystem()
{
}

FileSystem::FileSystem(const FileSystem& other)
{
}

FileSystem::~FileSystem()
{
}

FileSystem* FileSystem::GetInstance()
{
	static FileSystem* kInstance = nullptr;
	if (!kInstance)
	{
		kInstance = new	FileSystem;
	}
	return kInstance;
}

void FileSystem::Initialize()
{
	AddFileDevice({ "../../Assets/", FileDevice::Type::Assets, "assets" });
	AddFileDevice({ "../../Assets/Shaders/HLSL/", FileDevice::Type::ShaderSource, "shadersrc" });
}

void FileSystem::AddFileDevice(FileDevice device)
{
	INFO("Mounted file device: [%s] '%s'", device.Identifier.c_str(), device.RootPath.c_str());
	mDevices.push_back(device);
}

bool FileSystem::FixupPath(std::string& path)
{
	// Get path identifier:
	size_t charPos = path.find(':');
	if (charPos == std::string::npos)
	{
		return false;
	}
	std::string pathIdentifier;
	pathIdentifier = std::string(path.c_str(), charPos);

	// Try to match with a file device:
	for (const FileDevice& device : mDevices)
	{
		if (device.Identifier == pathIdentifier)
		{
			path = path.erase(0, charPos + 1);
			path.insert(0, device.RootPath);
			if (FileExists(path))
			{
				return true;
			}
			// Don't break, we can have more devices for this file type.
		}
	}

	return false;
}

bool FileSystem::FileExists(const std::string& fixeupFile)
{
#if defined(PLATFORM_WINDOWS)

	DWORD res;
	res = GetFileAttributesA(fixeupFile.c_str());
	if (res == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}
	return true;

#else

	#error Platform not implemented
	return false;

#endif
}
