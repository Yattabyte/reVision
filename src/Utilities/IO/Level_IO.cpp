#include "Utilities/IO/Level_IO.h"
#include "Engine.h"
#include <filesystem>


bool Level_IO::Import_Level(Engine * engine, const std::string & relativePath, std::vector<LevelStruct_Entity> & entities)
{
	// Check if the file exists
	if (!Engine::File_Exists(relativePath)) {
		engine->getManager_Messages().error("The file \"" + relativePath + "\" does not exist.");
		return false;
	}
	// Try to load the file stream
	try {
		const auto path = Engine::Get_Current_Dir() + relativePath;
		auto mapStream = std::ifstream(path, std::ios::binary | std::ios::beg);
		std::vector<char> ecsData(std::filesystem::file_size(path));
		mapStream.read(ecsData.data(), (std::streamsize)ecsData.size());
		mapStream.close();
		entities = parse_level(ecsData);
		return true;
	}
	// Catch failure state
	catch (const std::ifstream::failure e) {
		engine->getManager_Messages().error("The file \"" + relativePath + "\" exists, but is corrupted.");
		return false;
	}
}

std::vector<LevelStruct_Entity> Level_IO::parse_level(const std::vector<char> & ecsData)
{
	std::vector<LevelStruct_Entity> entities;
	// Find all entities
	size_t dataRead(0ull);
	while (dataRead < ecsData.size()) {
		LevelStruct_Entity entity;

		// Find the end of this entity
		size_t entityDataCount(0ull), entityDataRead(0ull);
		std::memcpy(&entityDataCount, &ecsData[dataRead], sizeof(size_t));
		dataRead += sizeof(size_t);

		// Find all components between the beginning and end of this entity
		while (entityDataRead < entityDataCount) {
			// Find how large the component data is
			size_t componentDataSize(0ull);
			std::memcpy(&componentDataSize, &ecsData[dataRead], sizeof(size_t));
			entityDataRead += sizeof(size_t);
			dataRead += sizeof(size_t);

			// Retrieve component Data
			std::vector<char> componentData(componentDataSize);
			std::memcpy(&componentData[0], &ecsData[dataRead], componentDataSize);
			entityDataRead += componentDataSize;
			dataRead += componentDataSize;

			// Read component name from the data
			size_t componentDataRead(0ull);
			int nameCount(0);
			std::memcpy(&nameCount, &componentData[0], sizeof(int));
			componentDataRead += sizeof(int);
			char *chars = new char[size_t(nameCount) + 1ull];
			std::fill(&chars[0], &chars[nameCount + 1], '\0');
			std::memcpy(chars, &componentData[sizeof(int)], nameCount);
			componentDataRead += sizeof(char) * nameCount;
			const auto stringifiedName = std::string(chars);
			delete[] chars;

			std::vector<char> serializedComponentData;
			if (componentDataSize - componentDataRead)
				serializedComponentData = std::vector<char>(componentData.begin() + componentDataRead, componentData.end());
			entity.components.push_back(LevelStruct_Component{ stringifiedName, serializedComponentData });
		}
		entities.push_back(entity);
	}
	return entities;
}