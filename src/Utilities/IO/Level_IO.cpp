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
	// Find all entities
	std::vector<LevelStruct_Entity> entities;
	size_t dataRead(0ull);
	std::function<LevelStruct_Entity()> readInEntity = [&]() ->LevelStruct_Entity {
		LevelStruct_Entity entity;
		/* ENTITY DATA STRUCTURE {
			name char count
			name chars
			component data count
			component data
			entity child count
			--nested entity children--
		} */

		// Read the header for this entity
		char entityNameChars[256] = { '\0' };
		unsigned int nameSize(0u), entityChildCount(0u);
		size_t componentDataCount(0ull);

		// Read name char count
		std::memcpy(&nameSize, &ecsData[dataRead], sizeof(unsigned int));
		dataRead += sizeof(unsigned int);
		// Read name chars
		std::memcpy(entityNameChars, &ecsData[dataRead], size_t(nameSize) * sizeof(char));
		dataRead += nameSize * sizeof(char);
		// Read entity component data count
		std::memcpy(&componentDataCount, &ecsData[dataRead], sizeof(size_t));
		entity.name = std::string(entityNameChars, nameSize);
		dataRead += sizeof(size_t);
		// Read enitity child count
		std::memcpy(&entityChildCount, &ecsData[dataRead], sizeof(unsigned int));
		dataRead += sizeof(unsigned int);
		// Find all components between the beginning and end of this entity
		size_t componentDataRead(0ull);
		while (componentDataRead < componentDataCount) {
			// Find how large the component data is
			size_t componentDataSize(0ull);
			std::memcpy(&componentDataSize, &ecsData[dataRead], sizeof(size_t));
			componentDataRead += sizeof(size_t);
			dataRead += sizeof(size_t);

			// Retrieve component Data
			std::vector<char> componentData(componentDataSize);
			std::memcpy(&componentData[0], &ecsData[dataRead], componentDataSize);
			componentDataRead += componentDataSize;
			dataRead += componentDataSize;

			// Read component name from the data
			size_t nameDataRead(0ull);
			int nameCount(0);
			std::memcpy(&nameCount, &componentData[0], sizeof(int));
			nameDataRead += sizeof(int);
			char* chars = new char[size_t(nameCount) + 1ull];
			std::fill(&chars[0], &chars[nameCount + 1], '\0');
			std::memcpy(chars, &componentData[sizeof(int)], nameCount);
			nameDataRead += sizeof(char) * nameCount;
			const auto stringifiedName = std::string(chars);
			delete[] chars;

			std::vector<char> serializedComponentData;
			if (componentDataSize - nameDataRead)
				serializedComponentData = std::vector<char>(componentData.begin() + nameDataRead, componentData.end());
			entity.components.push_back(LevelStruct_Component{ stringifiedName, serializedComponentData });
		}
		// Find all child entities
		unsigned int childEntitiesRead(0ull);
		while (childEntitiesRead < entityChildCount && dataRead < ecsData.size()) {
			entity.children.push_back(readInEntity());
			childEntitiesRead++;
		}
		return entity;
	};
	while (dataRead < ecsData.size()) 
		entities.push_back(readInEntity());	
	return entities;
}