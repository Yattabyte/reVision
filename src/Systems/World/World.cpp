#include "Systems\World\World.h"
#include "Systems\World\Visibility_Token.h"
#include "Engine.h"
#include "ECS\ECS_DEFINES.h"
#include "Utilities\Transform.h"
#include <algorithm>

#include "ECS\Components\Model_Animated.h"
#include "ECS\Components\Model_Static.h"
#include "ECS\Components\Light_Directional.h"
#include "ECS\Components\Light_Directional_Cheap.h"
#include "ECS\Components\Light_Spot.h"
#include "ECS\Components\Light_Spot_Cheap.h"
#include "ECS\Components\Light_Point.h"
#include "ECS\Components\Light_Point_Cheap.h"
#include "ECS\Components\Reflector.h"

System_World::~System_World()
{
}

System_World::System_World()
{
	m_loaded = false;
	m_worldChanged = false;
}

void System_World::initialize(Engine * engine)
{
	if (!m_Initialized) {
		m_engine = engine;

		m_Initialized = true;
	}
}

void System_World::update(const float & deltaTime)
{
	checkWorld();

	static bool temp_loaded = false;
	if (!temp_loaded) {
		loadWorld();
		temp_loaded = true;
	}
}

void System_World::updateThreaded(const float & deltaTime)
{
	std::shared_lock<std::shared_mutex> stateGuard(m_stateLock);
	//if (!m_worldChanged && m_loaded) {
		calcVisibility(*m_engine->getCamera());
		std::shared_lock<std::shared_mutex> viewerGuard(m_viewerLock);
		for each (auto &camera in m_viewers)
			calcVisibility(*camera);
	//}
}

void System_World::registerViewer(Camera * c)
{
	std::unique_lock<std::shared_mutex> writeGuard(m_viewerLock);
	m_viewers.push_back(c);
}

void System_World::unregisterViewer(Camera * c)
{
	std::unique_lock<std::shared_mutex> writeGuard(m_viewerLock);
	m_viewers.erase(std::remove_if(begin(m_viewers), end(m_viewers), [c](const auto * camera) {
		return (camera == c);
	}), end(m_viewers));
}

void System_World::notifyWhenLoaded(bool * notifyee)
{
	m_loadNotifiers.push_back(notifyee);
}

void System_World::calcVisibility(Camera & camera)
{
	const auto camBuffer = camera.getCameraBuffer();
	const float &radius = camBuffer.FarPlane;
	const glm::vec3 &eyePos = camBuffer.EyePosition;
	Visibility_Token vis_token;

	for each (const auto &type in std::vector<const char *>{ Model_Static_C::GetName(), Model_Animated_C::GetName(), Light_Directional_C::GetName(), Light_Directional_Cheap_C::GetName(), Light_Spot_C::GetName(), Light_Spot_Cheap_C::GetName(), Light_Point_C::GetName(), Light_Point_Cheap_C::GetName(), Reflector_C::GetName() }) {
		std::vector<Component*> visible_components;
		
		for each (auto component in m_engine->getECS().getSpecificComponents<Component>(type))
			if (component->isVisible(radius, eyePos))
				visible_components.push_back(component);

		vis_token.insertType(type);
		vis_token[type] = visible_components;
	}	

	camera.setVisibilityToken(vis_token);
}

inline bool find(const std::string & s1, const std::string & s2) {
	return (s1.find(s2) != std::string::npos);
}

/** Attempts to retrieve a std::string between quotation marks "<std::string>"
* @return	the std::string between quotation marks*/
inline std::string get_between_quotes(std::string & s)
{
	std::string output = s;
	int spot1 = s.find_first_of("\"");
	if (spot1 >= 0) {
		output = output.substr(spot1 + 1, output.length() - spot1 - 1);
		int spot2 = output.find_first_of("\"");
		if (spot2 >= 0) {
			output = output.substr(0, spot2);

			s = s.substr(spot2 + 2, s.length() - spot2 - 1);
		}
	}
	return output;
}

inline std::string getType_String(std::string & in) 
{
	return get_between_quotes(in);
}

inline int getType_UInt(std::string & in)
{
	return (unsigned int)std::stoi(get_between_quotes(in));
}

inline int getType_Int(std::string & in)
{
	return std::stoi(get_between_quotes(in));
}

inline int getType_Double(std::string & in)
{
	return std::stod(get_between_quotes(in));
}

inline int getType_Float(std::string & in)
{
	return std::stof(get_between_quotes(in));
}

inline glm::vec2 getType_Vec2(std::string & in)
{
	std::string vec2string = getType_String(in);
	int indices[1];
	indices[0] = vec2string.find(',');

	std::string number1(vec2string.substr(0, indices[0]));
	std::string number2(vec2string.substr(indices[0] + 1, (vec2string.size() - 1) - indices[0]));

	return glm::vec2(std::stof(number1), std::stof(number2));
}

inline glm::vec3 getType_Vec3(std::string & in)
{
	std::string vec3string = getType_String(in);
	int indices[2];
	indices[0] = vec3string.find(',');
	indices[1] = vec3string.find(',', indices[0]+1);

	std::string number1(vec3string.substr(0, indices[0]));
	std::string number2(vec3string.substr(indices[0]+1, indices[1] - (indices[0]+1)));
	std::string number3(vec3string.substr(indices[1]+1, (vec3string.size() - 1) - indices[1]));
	
	return glm::vec3(std::stof(number1), std::stof(number2), std::stof(number3));
}

inline glm::vec4 getType_Vec4(std::string & in)
{
	std::string vec4string = getType_String(in);
	int indices[3];
	indices[0] = vec4string.find(',');
	indices[1] = vec4string.find(',', indices[0] + 1);
	indices[2] = vec4string.find(',', indices[1] + 1);

	std::string number1(vec4string.substr(0, indices[0]));
	std::string number2(vec4string.substr(indices[0] + 1, indices[1] - (indices[0] + 1)));
	std::string number3(vec4string.substr(indices[1] + 1, indices[2] - (indices[1] + 1)));
	std::string number4(vec4string.substr(indices[2] + 1, (vec4string.size() - 1) - indices[2]));

	return glm::vec4(std::stof(number1), std::stof(number2), std::stof(number3), std::stof(number4));
}

inline glm::quat getType_Quat(std::string & in)
{
	std::string vec4string = getType_String(in);
	int indices[3];
	indices[0] = vec4string.find(',');
	indices[1] = vec4string.find(',', indices[0] + 1);
	indices[2] = vec4string.find(',', indices[1] + 1);

	std::string number1(vec4string.substr(0, indices[0]));
	std::string number2(vec4string.substr(indices[0] + 1, indices[1] - (indices[0] + 1)));
	std::string number3(vec4string.substr(indices[1] + 1, indices[2] - (indices[1] + 1)));
	std::string number4(vec4string.substr(indices[2] + 1, (vec4string.size() - 1) - indices[2]));

	return glm::quat(std::stof(number1), std::stof(number2), std::stof(number3), std::stof(number4));
}



#include <fstream>
void System_World::loadWorld()
{
	ECS & ecs = m_engine->getECS();

	// Open Level file		
	std::ifstream file_stream(m_engine->Get_Current_Dir() + "\\Maps\\test_static.map");
	for (std::string entityLine; std::getline(file_stream, entityLine); ) {
		// Search for entities
		if (find(entityLine, "entity")) {
			std::vector<Component*> components;
			std::vector<const char*> types;
			int curlyBraceCount = 0;
			for (std::string componentLine; std::getline(file_stream, componentLine); ) {
				// Count curly braces
				if (find(componentLine, "{"))
					curlyBraceCount++;
				else if (find(componentLine, "}"))
					curlyBraceCount--;
				// Search for components
				else if (find(componentLine, "component")) {
					std::string componentType = "";
					ArgumentList list;

					int curlyBraceCount = 0;
					for (std::string paramLine; std::getline(file_stream, paramLine); ) {
						// Count curly braces
						if (find(paramLine, "{"))
							curlyBraceCount++;
						else if (find(paramLine, "}"))
							curlyBraceCount--;

						// Fetch Component Type
						else if (find(paramLine, "type"))
							componentType = getType_String(paramLine);

						// Fetch Arguments
						// Maybe we can use some sort of creator pattern here too, like with the component creation?
						else if (find(paramLine, "string"))
							list.dataPointers.push_back(new std::string(getType_String(paramLine)));
						else if (find(paramLine, "uint"))
							list.dataPointers.push_back(new unsigned int(getType_UInt(paramLine)));
						else if (find(paramLine, "int"))
							list.dataPointers.push_back(new int(getType_Int(paramLine)));
						else if (find(paramLine, "double"))
							list.dataPointers.push_back(new double(getType_Double(paramLine)));
						else if (find(paramLine, "float"))
							list.dataPointers.push_back(new float(getType_Float(paramLine)));
						else if (find(paramLine, "vec2"))
							list.dataPointers.push_back(new glm::vec2(getType_Vec2(paramLine)));
						else if (find(paramLine, "vec3"))
							list.dataPointers.push_back(new glm::vec3(getType_Vec3(paramLine)));
						else if (find(paramLine, "vec4"))
							list.dataPointers.push_back(new glm::vec4(getType_Vec4(paramLine)));
						else if (find(paramLine, "quat"))
							list.dataPointers.push_back(new glm::quat(getType_Quat(paramLine)));
						else if (find(paramLine, "transform")) {
							std::string positionLine, orientationLine, scaleLine;
							std::getline(file_stream, positionLine);
							std::getline(file_stream, orientationLine);
							std::getline(file_stream, scaleLine);

							list.dataPointers.push_back(new Transform(getType_Vec3(positionLine), getType_Quat(orientationLine), getType_Vec3(scaleLine)));
						}
						if (curlyBraceCount == 0)
							break;
					}
					components.push_back(ecs.createComponent(componentType.c_str(), list));
					types.push_back(componentType.c_str());
				}
				if (curlyBraceCount == 0)
					break;
			}
			ecs.createEntity_Manual(components.size(), types.data(), components.data());
		}
	}

	m_loaded = false;
	m_worldChanged = true;
}

void System_World::unloadWorld()
{
	std::lock_guard<std::shared_mutex> state_writeGuard(m_stateLock);
	m_worldChanged = true;
	m_loaded = false;

	m_engine->getECS().flush();

	std::lock_guard<std::shared_mutex> view_writeGuard(m_viewerLock);
	m_engine->getCamera()->setVisibilityToken(Visibility_Token());
	m_viewers.clear();
}

void System_World::checkWorld()
{
	if (m_worldChanged && !m_loaded) {
		for each (const auto pair in m_engine->getECS().getComponents()) 
			for each (auto component in pair.second) 
				if (!component->isLoaded())
					return;		
		
		m_loaded = true;
		m_worldChanged = false;

		for each (bool * notifyee in m_loadNotifiers) 
			*notifyee = true;	
	}
}

#include <fstream>
void System_World::saveWorld()
{
}
