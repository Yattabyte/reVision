#pragma once
#ifndef LIGHT_TECH_H
#define LIGHT_TECH_H

#include "GL\glew.h"
#include "Systems\World\Visibility_Token.h"
#include "ECS\Components\Lighting.h"
#include <vector>


/**
 * An interface for specific deferred shading lighting techniques.
 * To be used only by the DS_Lighting class.
 **/
class Light_Tech {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Light_Tech() {}
	/** Constructor. */
	Light_Tech() {}


	// Interface Declarations
	/** Return std::string name of this technique.
	 * @return	std::string name of this technique */
	virtual const char * getName() const = 0;
	/** Perform updates, calculations, and memory writes for direct lighting
	 * @param	vis_token		the visibility token
	 * @param	updateQuality	the number of lights to allow per update
	 * @param	camPos			the position of the viewer in 3D space */
	virtual void updateData(const Visibility_Token & vis_token, const int & updateQuality, const glm::vec3 & camPos) = 0;
	/** Perform updates, calculations, and memory writes for indirect lighting
	 * @param	vis_token			the visibility token
	 * @param	bounceResolution	the resolution of the bounce buffer */
	virtual void updateDataGI(const Visibility_Token & vis_token, const unsigned int & bounceResolution) = 0;
	/** Apply occlusion culling for this category of lights. */
	virtual void renderOcclusionCulling() = 0;
	/** Apply a shadowing pass for this category of lights. */
	virtual void renderShadows() = 0;
	/** Apply a bounce pass for this category of lights. */
	virtual void renderLightBounce() = 0;
	/** Apply a lighting pass for this category of lights. */
	virtual void renderLighting() = 0;
};


#include "Utilities\PriorityList.h"
#include <functional>
/** This is used to prioritize the oldest AND closest lights to the viewer (position) */
class PriorityLightList {
public:
	// (de)Constructors
	/** Default destructor. */
	~PriorityLightList() {}
	/** Construct a priority light list with the given quality and position.
	* @param	quality		the max number of final elements
	* @param	position	the position of the viewer */
	PriorityLightList(const unsigned int & quality, const glm::vec3 & position) : m_quality(quality), m_oldest(quality), m_position(position) {}


	// Public Methods
	/** Fill the oldest light list with a new light, and have it sorted.
	* @param	light		the light to insert */
	void insert(Lighting_C * light) {
		m_oldest.insert(light->getShadowUpdateTime(), light);
	}
	/** Return a list composed of the oldest and the closest lights.
	* @return				a double sorted list with the oldest lights and closest lights */
	const std::vector<Lighting_C*> toList() const {
		PriorityList<float, Lighting_C*, std::greater<float>> m_closest(m_quality / 2);
		std::vector<Lighting_C*> outList;
		outList.reserve(m_quality);

		for each (const auto &element in m_oldest.toList()) {
			if (outList.size() < (m_quality / 2))
				outList.push_back(element);
			else
				m_closest.insert(element->getImportance(m_position), element);
		}

		for each (const auto &element in m_closest.toList()) {
			if (outList.size() >= m_quality)
				break;
			outList.push_back(element);
		}

		return outList;
	}


private:
	// Private Attributes
	unsigned int m_quality;
	glm::vec3 m_position;
	PriorityList<float, Lighting_C*, std::less<float>> m_oldest;
};

#endif // LIGHT_TECH_H