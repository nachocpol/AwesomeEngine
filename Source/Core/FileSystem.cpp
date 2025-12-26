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
	static bool kInitialized = false;
	if (!kInitialized)
	{
		AddFileDevice({ "../../Data/", FileDevice::Type::Assets, "data" });
		AddFileDevice({ "../../Source/Graphics/Shaders/", FileDevice::Type::ShaderSource, "shadersrc" });
		kInitialized = true;
	}
}

void FileSystem::AddFileDevice(FileDevice device)
{
	INFO("Mounted file device: [%s] '%s'", device.Identifier.c_str(), device.RootPath.c_str());
	mDevices.push_back(device);
}

bool FileSystem::FixupPath(std::string& path, bool checkFileExist /*= true*/)
{
	// Nothing to do, already a valid path
	if (FileExists(path))
	{
		return true;
	}

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
			std::string tempPath = path;
			tempPath = tempPath.erase(0, charPos + 1);
			tempPath.insert(0, device.RootPath);
			if (checkFileExist && FileExists(tempPath))
			{
				path = tempPath;
				return true;
			}
			// Don't break, we can have more devices for this file type.

			// So, if we don't care if it exists and just want to resolve it... Return with the first resolution
			if (!checkFileExist)
			{
				path = tempPath;
				return true;
			}
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

void FileSystem::GetPathsForIdentifier(const char* identifier, std::vector<std::string>& paths)
{
	for (size_t i = 0; i < mDevices.size(); ++i)
	{
		const FileDevice& device = mDevices[i];
		if (device.Identifier == identifier)
		{
			paths.push_back(device.RootPath);
		}
	}
}
