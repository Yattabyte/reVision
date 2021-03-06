#include "Utilities/IO/Level_IO.h"
#include "Engine.h"
#include <filesystem>
#include <fstream>


constexpr auto LevelPrefix = "\\Maps\\";

inline static auto Get_Full_Path(const std::string& relativePath)
{
	return Engine::Get_Current_Dir() + LevelPrefix + relativePath;
}

bool Level_IO::Level_Exists(const std::string& relativePath)
{
	return std::filesystem::exists(Get_Full_Path(relativePath));
}

bool Level_IO::Import_BMap(const std::string& relativePath, ecsWorld& world)
{
	// Try to get file first
	const auto path = Get_Full_Path(relativePath);
	std::ifstream mapFile(path, std::ios::binary | std::ios::in | std::ios::beg);
	if (!mapFile.is_open())
		return false;

	// Read ecsData from disk
	std::vector<char> ecsData(std::filesystem::file_size(path));
	mapFile.read(ecsData.data(), static_cast<std::streamsize>(ecsData.size()));
	mapFile.close();
	world = ecsWorld(ecsData);
	return true;
}

bool Level_IO::Export_BMap(const std::string& relativePath, const ecsWorld& world)
{
	// Try to get file first
	std::fstream mapFile(Get_Full_Path(relativePath), std::ios::binary | std::ios::out);
	if (!mapFile.is_open())
		return false;

	// Write ECS data to disk
	EntityHandle rootHandle;
	const auto data = world.serializeEntities(world.getEntityHandles(rootHandle));
	mapFile.write(data.data(), static_cast<std::streamsize>(data.size()));
	mapFile.close();
	return true;
}