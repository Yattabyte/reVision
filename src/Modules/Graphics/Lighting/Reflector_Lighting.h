#pragma once
#ifndef REFLECTOR_LIGHTING_H
#define REFLECTOR_LIGHTING_H

#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/PriorityList.h"
#include <vector>


class Engine;

/***/
class Reflector_Lighting : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	~Reflector_Lighting();
	/** Constructor. */
	Reflector_Lighting(Engine * engine);


	// Public Interface Implementations
	virtual void applyTechnique(const float & deltaTime) override;
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override;
	

	// Public Methods
	/***/
	inline auto & getReflectorBuffer() {
		return m_reflectorBuffer;
	}


private:
	// Protected Methods
	/** Render all the geometry for each reflector */
	void renderScene(const float & deltaTime);
	/** Render all the lights */
	void renderReflectors(const float & deltaTime);
	/***/
	void updateCamera();
	/** Converts a priority queue into an stl vector.*/
	static std::vector<Reflector_Component*> PQtoVector(PriorityList<float, Reflector_Component*, std::less<float>> oldest);


	// Private Attributes
	Engine * m_engine = nullptr;	
	Shared_Primitive m_shapeCube, m_shapeQuad;
	Shared_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	StaticBuffer m_indirectCube = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad6Faces = StaticBuffer(sizeof(GLuint) * 4);
	GLuint m_envmapSize = 512u;
	DynamicBuffer m_visLights;
	std::vector<Reflector_Component*> m_reflectorsToUpdate;
	bool m_outOfDate = true;
	VectorBuffer<Reflector_Component::GL_Buffer> m_reflectorBuffer;
	FBO_EnvMap m_envmapFBO;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	int m_notifyReflector = -1;

	// Reflector POV rendering
	std::shared_ptr<CameraBuffer> m_reflectorCamera;
	std::shared_ptr<Graphics_Framebuffers> m_reflectorFBOS;
	std::shared_ptr<RH_Volume> m_reflectorVRH;
	bool m_renderingSelf = false; // used to avoid calling self infinitely
};

#endif // REFLECTOR_LIGHTING_H