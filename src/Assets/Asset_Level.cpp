#include "Assets\Asset_Level.h"
#include "Engine.h"
#include <fstream>

#define DIRECTORY_LEVEL Engine::Get_Current_Dir() + "\\Maps\\"
#define ABS_DIRECTORY_LEVEL(filename) DIRECTORY_LEVEL + filename 


Asset_Level::~Asset_Level()
{
}

Asset_Level::Asset_Level(const std::string & filename) : Asset(filename) {}

Shared_Asset_Level Asset_Level::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Level>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Level>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_LEVEL(filename);
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		if (!Engine::File_Exists(fullDirectory)) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Level::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative	
}

/** Attempts to retrieve a std::string between quotation marks "<std::string>"
* @return	the std::string between quotation marks*/
static std::string const get_between_quotes(std::string & s)
{
	std::string output = s;
	size_t spot1 = s.find_first_of("\"");
	if (spot1 >= 0) {
		output = output.substr(spot1 + 1, output.length() - spot1 - 1);
		size_t spot2 = output.find_first_of("\"");
		if (spot2 >= 0) {
			output = output.substr(0, spot2);

			s = s.substr(spot2 + 2, s.length() - spot2 - 1);
		}
	}
	return output;
}

static bool const find(const std::string & s1, const std::string & s2) {
	return (s1.find(s2) != std::string::npos);
}
static std::string const getType_String(std::string & in) {
	return get_between_quotes(in);
}
static unsigned int const getType_UInt(std::string & in) {
	return (unsigned int)std::stoi(get_between_quotes(in));
}
static int const getType_Int(std::string & in) {
	return std::stoi(get_between_quotes(in));
}
static double const getType_Double(std::string & in){
	return std::stod(get_between_quotes(in));
}
static float const getType_Float(std::string & in) {
	return std::stof(get_between_quotes(in));
}
static glm::vec2 const getType_Vec2(std::string & in) {
	std::string vec2string = getType_String(in);
	size_t indices[1];
	indices[0] = vec2string.find(',');

	std::string number1(vec2string.substr(0, indices[0]));
	std::string number2(vec2string.substr(indices[0] + 1, (vec2string.size() - 1) - indices[0]));

	return glm::vec2(std::stof(number1), std::stof(number2));
}
static glm::vec3 const getType_Vec3(std::string & in) {
	std::string vec3string = getType_String(in);
	size_t indices[2];
	indices[0] = vec3string.find(',');
	indices[1] = vec3string.find(',', indices[0] + 1);

	std::string number1(vec3string.substr(0, indices[0]));
	std::string number2(vec3string.substr(indices[0] + 1, indices[1] - (indices[0] + 1)));
	std::string number3(vec3string.substr(indices[1] + 1, (vec3string.size() - 1) - indices[1]));

	return glm::vec3(std::stof(number1), std::stof(number2), std::stof(number3));
}
static glm::vec4 const getType_Vec4(std::string & in) {
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
static glm::quat const getType_Quat(std::string & in) {
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

static LevelStruct_Component parse_component(std::ifstream & file_stream)
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

static LevelStruct_Entity parse_entity(std::ifstream & file_stream)
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
		else if (find(line, "Component") ) {
			auto component = parse_component(file_stream);
			component.type = line;
			entity.components.push_back(component);
		}
	}
	return entity;
}

static std::vector<LevelStruct_Entity> parse_level(std::ifstream & file_stream) 
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

void Asset_Level::initialize(Engine * engine, const std::string & fullDirectory)
{
	try {
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);		;
		m_entities = parse_level(std::ifstream(fullDirectory));
	}
	catch (const std::ifstream::failure) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Level");
		initializeDefault(engine);
		return;
	}
}

void Asset_Level::finalize(Engine * engine)
{
	Asset::finalize(engine);
}
