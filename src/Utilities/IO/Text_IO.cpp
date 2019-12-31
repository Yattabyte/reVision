#include "Utilities/IO/Text_IO.h"
#include "Engine.h"
#include <fstream>


bool Text_IO::Import_Text(Engine& engine, const std::string& relativePath, std::string& importedData, const std::ios_base::openmode& mode)
{
	if (!Engine::File_Exists(relativePath)) {
		engine.getManager_Messages().error("The file \"" + relativePath + "\" does not exist.");
		return false;
	}

	std::ifstream file(Engine::Get_Current_Dir() + relativePath, mode);
	std::string temp;
	while (!file.eof()) {
		std::getline(file, temp);
		importedData.append(temp + '\n');
	}

	return true;
}

void Text_IO::Export_Text(const std::string& relativePath, const std::string& exportedData)
{
	std::ofstream out(Engine::Get_Current_Dir() + relativePath);
	out << exportedData.c_str();
}