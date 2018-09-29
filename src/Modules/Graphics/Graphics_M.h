#pragma once
#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#include "Modules\Engine_Module.h"
#include "Modules\Graphics\Effects\Effect_Base.h"
#include "Modules\Graphics\Common\FBO_Geometry.h"
#include "Modules\Graphics\Common\FBO_Lighting.h"
#include "Modules\Graphics\Common\FBO_Reflection.h"
#include "Modules\Graphics\Common\FBO_LightBounce.h"
#include "Modules\Graphics\Common\RH_Volume.h"
#include "Modules\Graphics\Common\VisualFX.h"
#include "Assets\Asset_Shader.h"
#include "ECS\Systems\ecsSystem.h"
#include "ECS\Components\Camera_C.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\MappedChar.h"


class ECS;

/** A module responsible for rendering.
@note	performs physically based rendering techniques using deferred rendering. */
class Graphics_Module : public Engine_Module {
public:
	// (de)Constructors
	~Graphics_Module() = default;
	Graphics_Module(Engine * engine);


	// Public Interface Implementation
	/** Initialize the module. */
	virtual void initialize() override;


	// Public Methods
	/** Render a single frame. 
	@param	deltaTime	the amount of time passed since last frame */
	void renderFrame(const float & deltaTime);
	/** Add a single system to this module. */
	template <typename T>
	void addSystem(T * t) {
		m_mappedGFXSystems[typeid(T).name()] = t;
		m_renderingSystems.addSystem(t);
	}
	/** Add a single effect to this module. */
	template <typename T>
	void addEffect(T * t) {
		m_mappedFX[typeid(T).name()] = t;
		m_fxTechs.push_back(t);
	}
	/** Returns a pointer to the system type requested. 
	@param	return		system of type specified. */
	template <typename T>
	T * getSystem() {
		return (T*)m_mappedGFXSystems[typeid(T).name()];
	}
	/** Returns a pointer to the effect type requested.
	@param	return		system of type specified. */
	template <typename T>
	T * getEffect() {
		return (T*)m_mappedFX[typeid(T).name()];
	}
	/** Update the data for the specified camera. 
	@param	camera			the camera to update */
	void updateCamera(Camera_Buffer * camera);
	/** Set the specified camera index as the active camera. 
	@param	newCameraID		the camera index to use */
	void setActiveCamera(const GLuint & newCameraID);
	/** Return the index of the camera last active. 
	@return					the camera index of the last active camera */
	const GLuint getActiveCamera() const;

	
	// Public Attributes
	// Frame Buffers
	FBO_Geometry				m_geometryFBO;
	FBO_Lighting				m_lightingFBO;
	FBO_Reflection				m_reflectionFBO;
	FBO_LightBounce				m_bounceFBO;
	VB_Element<Camera_Buffer> *	m_defaultCamera;
	StaticBuffer				m_cameraIndexBuffer;
	VectorBuffer<Camera_Buffer>	m_cameraBuffer;
	MappedChar<BaseECSSystem*>	m_mappedGFXSystems;

	
private:
	// Private Attributes
	ECS *						m_ecs = nullptr;
	GLuint						m_activeCamera = 0;
	glm::ivec2					m_renderSize = glm::ivec2(1);
	ECSSystemList				m_renderingSystems;
	std::vector<Effect_Base*>	m_fxTechs;
	MappedChar<Effect_Base*>	m_mappedFX;
	std::shared_ptr<RH_Volume>	m_volumeRH;
	VisualFX					m_visualFX;
	Shared_Asset_Shader			m_shaderCull, m_shaderGeometry;
	std::shared_ptr<bool>		m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // GRAPHICS_MODULE_H