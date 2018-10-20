#include "Utilities\IO\Text_IO.h"
#include "Engine.h"
#include <fstream>


bool Text_IO::Import_Text(Engine * engine, const std::string & relativePath, std::string & data_container, std::ios_base::openmode mode)
{
	if (!Engine::File_Exists(relativePath)) {
		engine->getMessageManager().error(MessageManager::FILE_MISSING, relativePath);
		return false;
	}

	std::ifstream file(Engine::Get_Current_Dir() + relativePath, mode);
	std::string temp;
	while (!file.eof()) {
		std::getline(file, temp);
		data_container.append(temp + '\n');
	}

	return true;
}

void Text_IO::Export_Text(const std::string & relativePath, const std::string & exportedData)
{
	std::ofstream out(Engine::Get_Current_Dir() + relativePath);
	out << exportedData.c_str();
}
