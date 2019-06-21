#pragma once
#ifndef REFLECTOR_LIGHTING_H
#define REFLECTOR_LIGHTING_H

#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/FBO_Env_Reflector.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "Utilities/PriorityList.h"
#include <vector>


class Engine;

/** A core lighting technique responsible for all parallax reflectors. */
class Reflector_Lighting : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	~Reflector_Lighting();
	/** Constructor. */
	Reflector_Lighting(Engine * engine);


	// Public Interface Implementations
	virtual void beginFrame(const float & deltaTime) override;
	virtual void endFrame(const float & deltaTime) override;
	virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) override;
	

private:
	// Private Methods
	/***/
	void syncComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components);
	/***/
	void updateVisibility(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components);
	/** Render all the geometry for each reflector.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	void renderScene(const float & deltaTime, const std::shared_ptr<Viewport> & viewport);
	/** Render all the lights 
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	void renderReflectors(const float & deltaTime, const std::shared_ptr<Viewport> & viewport);
	/** Converts a priority queue into an stl vector.*/
	static std::vector<Reflector_Component*> PQtoVector(PriorityList<float, Reflector_Component*, std::less<float>> oldest);
	/** Clear out the reflectors queued up for rendering. */
	void clear();


	// Private Attributes
	Engine * m_engine = nullptr;	
	Shared_Primitive m_shapeCube, m_shapeQuad;
	Shared_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	StaticBuffer m_indirectCube = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad6Faces = StaticBuffer(sizeof(GLuint) * 4);
	GLuint m_envmapSize = 512u;
	DynamicBuffer m_visLights;
	std::vector<Reflector_Component*> m_reflectorsToUpdate;
	bool m_outOfDate = true;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);

	// Core Lighting Data
	/** OpenGL buffer for Parallax reflectors. */
	struct Reflector_Buffer {
		glm::mat4 mMatrix;
		glm::mat4 rotMatrix;
		glm::vec3 BoxCamPos; float padding1;
		glm::vec3 BoxScale; float padding2;
		int CubeSpot; glm::vec3 padding3;
	};
	GL_ArrayBuffer<Reflector_Buffer> m_reflectorBuffer;
	FBO_Env_Reflector m_envmapFBO;


	// Reflector POV rendering
	std::shared_ptr<Viewport> m_reflectorViewport;
	bool m_renderingSelf = false; // used to avoid calling self infinitely
};

#endif // REFLECTOR_LIGHTING_H