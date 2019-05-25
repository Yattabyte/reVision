#include "Utilities/IO/Level_IO.h"
#include "Engine.h"


bool Level_IO::Import_Level(Engine * engine, const std::string & relativePath, std::vector<LevelStruct_Entity> & entities)
{
	// Check if the file exists
	if (!Engine::File_Exists(relativePath)) {
		engine->getManager_Messages().error("The file \"" + relativePath + "\" does not exist.");
		return false;
	}
	// Try to load the file stream
	try {
		auto stream = std::ifstream(Engine::Get_Current_Dir() + relativePath);
		entities = parse_level(stream);
		return true;
	}
	// Catch failure state
	catch (const std::ifstream::failure e) {
		engine->getManager_Messages().error("The file \"" + relativePath + "\" exists, but is corrupted.");
		return false;
	}
}

std::vector<LevelStruct_Entity> Level_IO::parse_level(std::ifstream & file_stream)
{
	std::vector<LevelStruct_Entity> level;
	int bracketCount = 0;
	for (std::string line; std::getline(file_stream, line); ) {
		if (find(line, "{")) {
			bracketCount++;
			continue;
		}
		else if (find(line, "}")) {
			bracketCount--;
			if (bracketCount <= 0)
				break;
			continue;
		}
		else if (find(line, "entity"))
			level.push_back(parse_entity(file_stream));
	}
	return level;
}

LevelStruct_Entity Level_IO::parse_entity(std::ifstream & file_stream)
{
	LevelStruct_Entity entity;
	int bracketCount = 0;
	for (std::string line; std::getline(file_stream, line); ) {
		if (find(line, "{")) {
			bracketCount++;
			continue;
		}
		else if (find(line, "}")) {
			bracketCount--;
			if (bracketCount <= 0)
				break;
			continue;
		}
		else if (find(line, "Component")) {
			auto component = parse_component(file_stream);
			line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
			component.type = line;
			entity.components.push_back(component);
		}
	}
	return entity;
}

LevelStruct_Component Level_IO::parse_component(std::ifstream & file_stream)
{
	LevelStruct_Component component;
	int bracketCount = 0;
	for (std::string line; std::getline(file_stream, line); ) {
		if (line.length() && line != "" && line != " ") {
			if (find(line, "{")) {
				bracketCount++;
				continue;
			}
			else if (find(line, "}")) {
				bracketCount--;
				if (bracketCount <= 0)
					break;
				continue;
			}
			else {
				std::any parameter;
				if (find(line, "string"))
					parameter = getType_String(line);
				else if (find(line, "uint"))
					parameter = getType_UInt(line);
				else if (find(line, "int"))
					parameter = getType_Int(line);
				else if (find(line, "double"))
					parameter = getType_Double(line);
				else if (find(line, "float"))
					parameter = getType_Float(line);
				else if (find(line, "vec2"))
					parameter = getType_Vec2(line);
				else if (find(line, "vec3"))
					parameter = getType_Vec3(line);
				else if (find(line, "vec4"))
					parameter = getType_Vec4(line);
				else if (find(line, "quat"))
					parameter = getType_Quat(line);
				if (parameter.has_value())
					component.parameters.push_back(parameter);
			}
		}
	}
	return component;
}

std::string Level_IO::get_between_quotes(std::string & s)
{
	std::string output = s;
	size_t spot1 = s.find_first_of("\"");
	if (spot1 != std::string::npos) {
		output = output.substr(spot1 + 1, output.length() - spot1 - 1);
		size_t spot2 = output.find_first_of("\"");
		if (spot2 != std::string::npos) {
			output = output.substr(0, spot2);

			s = s.substr(spot2 + 2, s.length() - spot2 - 1);
		}
	}
	return output;
}

bool Level_IO::find(const std::string & s1, const std::string & s2) {
	return (s1.find(s2) != std::string::npos);
}

std::string Level_IO::getType_String(std::string & in) {
	return get_between_quotes(in);
}

unsigned int Level_IO::getType_UInt(std::string & in) {
	return (unsigned int)std::stoi(get_between_quotes(in));
}

int Level_IO::getType_Int(std::string & in) {
	return std::stoi(get_between_quotes(in));
}

double Level_IO::getType_Double(std::string & in) {
	return std::stod(get_between_quotes(in));
}

float Level_IO::getType_Float(std::string & in) {
	return std::stof(get_between_quotes(in));
}

glm::vec2 Level_IO::getType_Vec2(std::string & in) {
	std::string vec2string = getType_String(in);
	size_t indices[1];
	indices[0] = vec2string.find(',');

	std::string number1(vec2string.substr(0, indices[0]));
	std::string number2(vec2string.substr(indices[0] + 1, (vec2string.size() - 1) - indices[0]));

	return glm::vec2(std::stof(number1), std::stof(number2));
}

glm::vec3 Level_IO::getType_Vec3(std::string & in) {
	std::string vec3string = getType_String(in);
	size_t indices[2];
	indices[0] = vec3string.find(',');
	indices[1] = vec3string.find(',', indices[0] + 1);

	std::string number1(vec3string.substr(0, indices[0]));
	std::string number2(vec3string.substr(indices[0] + 1, indices[1] - (indices[0] + 1)));
	std::string number3(vec3string.substr(indices[1] + 1, (vec3string.size() - 1) - indices[1]));

	return glm::vec3(std::stof(number1), std::stof(number2), std::stof(number3));
}

glm::vec4 Level_IO::getType_Vec4(std::string & in) {
	std::string vec4string = getType_String(in);
	size_t indices[3];
	indices[0] = vec4string.find(',');
	indices[1] = vec4string.find(',', indices[0] + 1);
	indices[2] = vec4string.find(',', indices[1] + 1);

	std::string number1(vec4string.substr(0, indices[0]));
	std::string number2(vec4string.substr(indices[0] + 1, indices[1] - (indices[0] + 1)));
	std::string number3(vec4string.substr(indices[1] + 1, indices[2] - (indices[1] + 1)));
	std::string number4(vec4string.substr(indices[2] + 1, (vec4string.size() - 1) - indices[2]));

	return glm::vec4(std::stof(number1), std::stof(number2), std::stof(number3), std::stof(number4));
}

glm::quat Level_IO::getType_Quat(std::string & in) {
	std::string vec4string = getType_String(in);
	size_t indices[3];
	indices[0] = vec4string.find(',');
	indices[1] = vec4string.find(',', indices[0] + 1);
	indices[2] = vec4string.find(',', indices[1] + 1);

	std::string number1(vec4string.substr(0, indices[0]));
	std::string number2(vec4string.substr(indices[0] + 1, indices[1] - (indices[0] + 1)));
	std::string number3(vec4string.substr(indices[1] + 1, indices[2] - (indices[1] + 1)));
	std::string number4(vec4string.substr(indices[2] + 1, (vec4string.size() - 1) - indices[2]));

	return glm::quat(std::stof(number1), std::stof(number2), std::stof(number3), std::stof(number4));
}