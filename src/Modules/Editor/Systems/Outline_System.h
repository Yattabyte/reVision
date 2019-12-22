#pragma once
#ifndef OUTLINE_SYSTEM_H
#define OUTLINE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Assets/Mesh.h"
#include "Assets/Shader.h"
#include "Utilities/GL/DynamicBuffer.h"


// Forward Declarations
class Engine;
class LevelEditor_Module;

/** An ECS system responsible for rendering outlines of selected entities bounding objects. */
class Outline_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	~Outline_System();
	/** Construct this system.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	Outline_System(Engine& engine, LevelEditor_Module& editor);


	// Public Interface Implementation
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Methods
	/** Attempt to insert the mesh supplied into the mesh map, failing only if it is already present.
	@param	mesh		the mesh to insert only 1 copy of. */
	void tryInsertModel(const Shared_Mesh& mesh);
	/** Wait on the prop fence if it still exists. */
	void waitOnFence() noexcept;
	/** Attempt to expand the props' vertex buffer if it isn't large enough.
	@param	arraySize	the new size to use. */
	void tryToExpand(const size_t& arraySize) noexcept;


	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	float m_renderScale = 0.02f;
	Shared_Mesh m_cube, m_sphere, m_hemisphere;
	Shared_Shader m_shader;
	DynamicBuffer<> m_indirectGeometry, m_ssboTransforms, m_ssboScales;
	GLuint m_vaoID = 0, m_vboID = 0;
	size_t m_currentSize = 0ull, m_maxCapacity = 256ull;
	GLsync m_fence = nullptr;
	std::map<ComponentHandle, std::pair<GLuint, GLuint>> m_geometryParams;
	std::map<Shared_Mesh, std::pair<GLuint, GLuint>> m_meshMap;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // OUTLINE_SYSTEM_H