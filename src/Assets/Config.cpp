#include "Assets/Config.h"
#include "Utilities/IO/Text_IO.h"
#include "Engine.h"
#include <fstream>


constexpr char* EXT_CONFIG = ".cfg";
constexpr char* DIRECTORY_CONFIG = "\\Configs\\";

/** Attempts to retrieve a std::string between quotation marks "<std::string>".
@return	the std::string between quotation marks */
inline std::string get_between_quotes(std::string & s)
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

/** Checks if the supplied value is a parameter in the CFG_STRING list.
@param	s			the std::string to check for in the list
@param	m_strings	the list of strings to check for an occurrence of our value within
@return				the index of the value in the list if found, otherwise -1 */
inline int find_CFG_Property(const std::string & s, const std::vector<std::string> & m_strings)
{
	std::string upperCase(s);
	for (size_t x = 0, size = upperCase.size(); x < size; ++x)
		upperCase[x] = char(toupper(int(upperCase[x])));
	for (auto value = begin(m_strings); value != end(m_strings); value++)
		if ((*value) == upperCase)
			return (int)std::distance(m_strings.begin(), value);
	return -1;
}

Shared_Config::Shared_Config(Engine * engine, const std::string & filename, const std::vector<std::string> & strings, const bool & threaded)
{
	(*(std::shared_ptr<Config>*)(this)) = std::dynamic_pointer_cast<Config>(
		engine->getManager_Assets().shareAsset(
			typeid(Config).name(),
			filename,
			[engine, filename, strings]() { return std::make_shared<Config>(engine, filename, strings); },
			threaded
		));
}

Config::Config(Engine * engine, const std::string & filename, const std::vector<std::string> & strings) : Asset(engine, filename), m_strings(strings) {}

void Config::initialize()
{
	try {
		std::ifstream file_stream(Engine::Get_Current_Dir() + DIRECTORY_CONFIG + getFileName() + EXT_CONFIG);
		for (std::string line; std::getline(file_stream, line); ) {
			if (line.length()) {
				const std::string cfg_property = get_between_quotes(line);
				int spot = find_CFG_Property(cfg_property, m_strings);
				if (spot >= 0) {
					std::string cfg_value = get_between_quotes(line);
					setValue((unsigned int)spot, (float)atof(cfg_value.c_str()));
				}
			}
		}
	}
	catch (const std::ifstream::failure) {
		m_engine->getManager_Messages().error("Config \"" + m_filename + "\" failed to initialize.");
	}

	Asset::finalize();
}

void Config::setValue(const unsigned int & cfg_key, const float & cfg_value)
{
	// If the key doesn't exist in the map, [ ] will create it
	m_configuration[cfg_key] = cfg_value;
}

float Config::getValue(const unsigned int & cfg_key) const
{
	if (cfg_key >= 0 && m_configuration.find(cfg_key) != m_configuration.end())
		return m_configuration.at(cfg_key);
	return std::nanf("");
}

void Config::saveConfig()
{
	std::string output;
	for each (const auto &value in m_configuration) 
		output += "\"" + m_strings[value.first] + "\" \"" + std::to_string(value.second) + "\"\n";
	Text_IO::Export_Text(DIRECTORY_CONFIG + getFileName() + EXT_CONFIG, output);
}