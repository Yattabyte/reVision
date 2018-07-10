#include "Utilities\IO\Text_IO.h"
#include "Engine.h"
#include <fstream>


bool Text_IO::Import_Text(Engine * engine, const string & fulldirectory, string & data_container)
{
	if (!Engine::File_Exists(fulldirectory)) {
		engine->reportError(MessageManager::FILE_MISSING, fulldirectory);
		return false;
	}

	ifstream file(fulldirectory);
	while (!file.eof()) {
		string temp;
		std::getline(file, temp);
		data_container.append(temp + '\n');
	}

	return true;
}

void Text_IO::Export_Text(const string & fulldirectory, const string & exportedData)
{
	ofstream out(fulldirectory);
	out << exportedData.c_str();
}
