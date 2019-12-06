#pragma once
#ifndef PROPUPLOAD_SYSTEM_H
#define PROPUPLOAD_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Assets/Model.h"
#include <array>

#define NUM_VERTEX_ATTRIBUTES 8


// Forward Declarations
class Engine;
struct PropData;

/** An ECS system responsible for uploading prop data to the GPU, such as geometrical data and material textures. */
class PropUpload_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	~PropUpload_System() noexcept;
	/** Construct this system.
	@param	engine			reference to the engine to use.
	@param	frameData		reference to of common data that changes frame-to-frame. */
	PropUpload_System(Engine& engine, PropData& frameData) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Methods
	/** Attempt to insert the model supplied into the model map, failing only if it is already present.
	@param	model		the model to insert only 1 copy of. */
	void tryInsertModel(const Shared_Model& model) noexcept;
	/** Wait on the prop fence if it still exists. */
	void waitOnFence() noexcept;
	/** Attempt to expand the props' vertex buffer if it isn't large enough.
	@param	arraySize	the new size to use. */
	void tryToExpand(const size_t& arraySize) noexcept;
	/** Attempt to insert the material supplied into the material map, failing only if it is already present.
	@param	material	the material to insert only 1 copy of. */
	void tryInsertMaterial(const Shared_Material& material) noexcept;
	/***/
	std::pair<GLuint*, GLsync*> getFreePBO() noexcept;
	/** Clear all data held by this system. */
	void clear() noexcept;


	// Private Attributes
	Engine& m_engine;
	PropData& m_frameData;
	GLuint m_vaoID = 0, m_vboID = 0, m_matID;
	size_t m_currentSize = 0ull, m_maxCapacity = 256ull, m_matCount = 0ull;
	GLsizei m_materialSize = 512u;
	GLint m_maxTextureLayers = 6, m_maxMips = 1;
	GLsync m_fence = nullptr;
	std::map<Shared_Model, std::pair<GLuint, GLuint>> m_modelMap;
	std::map<Shared_Material, GLuint> m_materialMap;
	std::array<std::pair<GLuint, GLsync>, 3> m_pixelBuffers;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PROPUPLOAD_SYSTEM_H