#pragma once
#ifndef REFLECTOR_TECHNIQUE_H
#define REFLECTOR_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Reflector/FBO_Env_Reflector.h"
#include "Modules/World/ECS/components.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "Utilities/PriorityList.h"
#include "Engine.h"
#include <vector>


class Engine;

/** A core lighting technique responsible for all parallax reflectors. */
class Reflector_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Reflector_Technique() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Reflector_Technique(Engine * engine)
		: m_engine(engine), Graphics_Technique(PRIMARY_LIGHTING) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);

		// Asset Loading
		m_shaderLighting = Shared_Shader(m_engine, "Core\\Reflector\\IBL_Parallax");
		m_shaderStencil = Shared_Shader(m_engine, "Core\\Reflector\\Stencil");
		m_shaderCopy = Shared_Shader(m_engine, "Core\\Reflector\\2D_To_Cubemap");
		m_shaderConvolute = Shared_Shader(m_engine, "Core\\Reflector\\Cube_Convolution");
		m_shapeCube = Shared_Primitive(m_engine, "cube");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_ENVMAP_SIZE, m_envmapSize);
		preferences.addCallback(PreferenceState::C_ENVMAP_SIZE, m_aliveIndicator, [&](const float &f) {
			m_envmapSize = std::max(1u, (unsigned int)f);
			m_envmapFBO.resize(m_envmapSize, m_envmapSize, m_envmapCount * 6);
			m_reflectorViewport->resize(glm::vec2((float)m_envmapSize));
			m_outOfDate = true;
		});
		// Environment Map
		m_envmapSize = std::max(1u, (unsigned int)m_envmapSize);
		m_envmapFBO.resize(m_envmapSize, m_envmapSize, 6);

		// Camera Setup
		CameraBuffer::BufferStructure cameraData;
		cameraData.pMatrix = glm::mat4(1.0f);
		cameraData.vMatrix = glm::mat4(1.0f);
		cameraData.EyePosition = glm::vec3(0.0f);
		cameraData.Dimensions = glm::vec2((float)m_envmapSize);
		cameraData.FarPlane = 100.0f;
		cameraData.FOV = 90.0f;
		m_reflectorViewport = std::make_shared<Viewport>(engine, glm::ivec2(0), glm::ivec2((float)m_envmapSize), cameraData);

		GLuint data[] = { 0,0,0,0 };
		m_indirectCube.write(0, sizeof(GLuint) * 4, &data);
		m_indirectQuad.write(0, sizeof(GLuint) * 4, &data);
		m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, &data);


		// Asset-Finished Callbacks		
		m_shapeCube->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint data = { (GLuint)m_shapeCube->getSize() };
			m_indirectCube.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_indirectQuad.write(0, sizeof(GLuint) * 4, quadData);
			const GLuint quad6Data[4] = { (GLuint)m_shapeQuad->getSize(), 6, 0, 0 };
			m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, quad6Data);
		});

		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_envmapFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine->getManager_Messages().error("Reflector_Technique Environment Map Framebuffer has encountered an error.");

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addNotifyOnComponentType(Reflector_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			((Reflector_Component*)c)->m_reflectorIndex = m_reflectorBuffer.newElement();
		});

		// World-Changed Callback
		world.addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded) {
				clear();
				m_outOfDate = false;
			}
			else if (state == World_Module::finishLoading || state == World_Module::updated)
				m_outOfDate = true;
		});
	}


	// Public Interface Implementations
	inline virtual void beginFrame(const float & deltaTime) override {
		m_reflectorBuffer.beginWriting();
		m_visLights.beginWriting();
	}
	inline virtual void endFrame(const float & deltaTime) override {
		m_reflectorBuffer.endWriting();
		m_visLights.endWriting();
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		PriorityList<float, Reflector_Component*, std::less<float>> oldest;
		std::vector<GLint> reflectionIndicies;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
			const auto & index = reflectorComponent->m_reflectorIndex;

			// Synchronize the component if it is visible
			if (renderableComponent->m_visible) {
				reflectionIndicies.push_back(GLuint(*reflectorComponent->m_reflectorIndex));

				// Sync Transform Attributes
				if (transformComponent) {
					const auto & position = transformComponent->m_transform.m_position;
					const auto & orientation = transformComponent->m_transform.m_orientation;
					const auto & scale = transformComponent->m_transform.m_scale;
					const auto & modelMatrix = transformComponent->m_transform.m_modelMatrix;
					const auto matRot = glm::mat4_cast(orientation);
					const float largest = pow(std::max(std::max(scale.x, scale.y), scale.z), 2.0f);
					m_reflectorBuffer[index].mMatrix = modelMatrix;
					m_reflectorBuffer[index].rotMatrix = glm::inverse(matRot);
					m_reflectorBuffer[index].BoxCamPos = position;
					m_reflectorBuffer[index].BoxScale = scale;
					const glm::mat4 vMatrices[6] = {
						glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
						glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
						glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
						glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
						glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
						glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
					};
					const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, largest);
					for (int x = 0; x < 6; ++x) {
						reflectorComponent->m_cameraData[x].Dimensions = glm::vec2((float)m_envmapSize);
						reflectorComponent->m_cameraData[x].FOV = 90.0f;
						reflectorComponent->m_cameraData[x].FarPlane = largest;
						reflectorComponent->m_cameraData[x].EyePosition = position;
						reflectorComponent->m_cameraData[x].pMatrix = pMatrix;
						reflectorComponent->m_cameraData[x].vMatrix = vMatrices[x];
					}
				}

				// Envmap logic
				if (reflectorComponent->m_cubeSpot == -1) {
					// Assign envmap spot
					int envSpot = (int)(m_envmapCount) * 6;
					reflectorComponent->m_cubeSpot = envSpot;
					m_envmapCount++;
					m_envmapFBO.resize(m_envmapSize, m_envmapSize, m_envmapCount * 6);
				}

				// Sync Buffer Attributes
				m_reflectorBuffer[index].CubeSpot = reflectorComponent->m_cubeSpot;
			}
			
			// Find oldest envmaps always (may see shadow but not light source)
			oldest.insert(reflectorComponent->m_updateTime, reflectorComponent);
		}

		// Update Draw Buffers
		m_visLightCount = reflectionIndicies.size();
		m_visLights.write(0, sizeof(GLuint) *m_visLightCount, reflectionIndicies.data());
		m_indirectCube.write(sizeof(GLuint), sizeof(GLuint), &m_visLightCount); // update primCount (2nd param)

		// Envmaps are updated in rendering pass, determined once a frame
		m_reflectorsToUpdate = PQtoVector(oldest);
	}
	inline virtual void updateTechnique(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shaderCopy->existsYet() || !m_shaderConvolute->existsYet())
			return;

		if (m_engine->getActionState().isAction(ActionState::FIRE2) && !m_outOfDate)
			m_outOfDate = true;

		// Render important environment maps
		updateReflectors(deltaTime);
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) override {
		// Exit Early
		if (!m_enabled || !m_shapeCube->existsYet() || !m_shaderLighting->existsYet() || !m_shaderStencil->existsYet())
			return;

		// Render parallax reflectors
		renderReflectors(deltaTime, viewport);
	}
	

private:
	// Private Methods
	/** Render all the geometry for each reflector.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void updateReflectors(const float & deltaTime) {
		if (m_reflectorsToUpdate.size()) {
			if (m_renderingSelf)
				return;
			m_renderingSelf = true;
			// Copy the list of reflectors, because nested rendering may change the vector!
			for (auto * reflector : std::vector<Reflector_Component*>(m_reflectorsToUpdate)) {
				if (reflector->m_outOfDate || m_outOfDate) {
					reflector->m_outOfDate = false;
					for (int x = 0; x < 6; ++x) {
						// Set view-specific camera data
						m_reflectorViewport->m_cameraBuffer->beginWriting();
						m_reflectorViewport->m_cameraBuffer->replace(reflector->m_cameraData[x]);
						m_reflectorViewport->m_cameraBuffer->pushChanges();

						// Render Graphics Pipeline
						constexpr const unsigned int flags = Graphics_Technique::GEOMETRY | Graphics_Technique::PRIMARY_LIGHTING | Graphics_Technique::SECONDARY_LIGHTING;
						m_engine->getModule_Graphics().render(deltaTime, m_reflectorViewport, flags);
						m_reflectorViewport->m_cameraBuffer->endWriting();

						// Copy lighting frame into cube-face
						m_reflectorViewport->m_gfxFBOS->bindForReading("LIGHTING", 0);
						m_envmapFBO.bindForWriting();
						m_shaderCopy->bind();
						m_shaderCopy->setUniform(0, x + reflector->m_cubeSpot);
						m_indirectQuad.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
						glBindVertexArray(m_shapeQuad->m_vaoID);
						glDrawArraysIndirect(GL_TRIANGLES, 0);
					}

					// Once cubemap is generated, convolute it
					m_shaderConvolute->bind();
					m_shaderConvolute->setUniform(0, reflector->m_cubeSpot);
					m_envmapFBO.bindForReading();
					m_indirectQuad6Faces.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
					for (unsigned int r = 1; r < 6; ++r) {
						// Ensure we are writing to MIP level r
						const unsigned int write_size = (unsigned int)std::max(1.0f, (floor(m_envmapSize / pow(2.0f, (float)r))));
						glViewport(0, 0, write_size, write_size);
						m_shaderConvolute->setUniform(1, (float)r / 5.0f);
						glNamedFramebufferTexture(m_envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_envmapFBO.m_textureID, r);

						// Ensure we are reading from MIP level r - 1
						glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, r - 1);
						glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, r - 1);

						// Convolute the 6 faces for this roughness level (RENDERS 6 TIMES)
						glDrawArraysIndirect(GL_TRIANGLES, 0);
					}

					// Reset texture, so it can be used for other component reflections
					glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, 0);
					glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, 5);
					glNamedFramebufferTexture(m_envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_envmapFBO.m_textureID, 0);
					reflector->m_updateTime = m_engine->getTime();
					Shader::Release();
				}
			}

			m_outOfDate = false;
			m_renderingSelf = false;
		}
	}
	/** Render all the lights 
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderReflectors(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		if (m_visLightCount) {
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			// Draw only into depth-stencil buffer
			m_shaderStencil->bind();										// Shader (reflector)
			viewport->m_gfxFBOS->bindForWriting("REFLECTION");				// Ensure writing to reflection FBO
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);				// Read from Geometry FBO
			glBindTextureUnit(4, m_envmapFBO.m_textureID);					// Reflection map (environment texture)
			m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
			m_reflectorBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);	// Reflection buffer
			m_indirectCube.bindBuffer(GL_DRAW_INDIRECT_BUFFER);				// Draw call buffer
			glBindVertexArray(m_shapeCube->m_vaoID);						// Quad VAO
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilFunc(GL_ALWAYS, 0, 0);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthMask(GL_FALSE);
			glFrontFace(GL_CW);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
			glFrontFace(GL_CCW);

			// Now draw into color buffers
			m_shaderLighting->bind();
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glCullFace(GL_FRONT);
			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			glDepthMask(GL_TRUE);
			glCullFace(GL_BACK);
			glBlendFunc(GL_ONE, GL_ZERO);
			glDisable(GL_STENCIL_TEST);
		}
	}
	/** Converts a priority queue into an stl vector.*/
	inline static std::vector<Reflector_Component*> PQtoVector(PriorityList<float, Reflector_Component*, std::less<float>> oldest) {
		PriorityList<float, Reflector_Component*, std::greater<float>> m_closest(2);
		std::vector<Reflector_Component*> outList;
		outList.reserve(2);

		for each (const auto &element in oldest.toList()) {
			if (outList.size() < 2)
				outList.push_back(element);
			else
				m_closest.insert(element->m_updateTime, element);
		}

		for each (const auto &element in m_closest.toList()) {
			if (outList.size() >= 2)
				break;
			outList.push_back(element);
		}

		return outList;
	}
	/** Clear out the reflectors queued up for rendering. */
	inline void clear() {
		const size_t reflectorSize = 0;
		m_indirectCube.write(sizeof(GLuint), sizeof(GLuint), &reflectorSize); // update primCount (2nd param)
		m_reflectorsToUpdate.clear();
		m_reflectorBuffer.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;	
	Shared_Primitive m_shapeCube, m_shapeQuad;
	Shared_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	StaticBuffer m_indirectCube = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad6Faces = StaticBuffer(sizeof(GLuint) * 4);
	GLuint m_envmapSize = 512u;
	std::vector<Reflector_Component*> m_reflectorsToUpdate;
	bool m_outOfDate = true;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);

	// Visibility
	size_t m_visLightCount = 0ull;
	DynamicBuffer m_visLights;

	// Core Lighting Data
	/** OpenGL buffer for Parallax reflectors. */
	struct Reflector_Buffer {
		glm::mat4 mMatrix;
		glm::mat4 rotMatrix;
		glm::vec3 BoxCamPos; float padding1;
		glm::vec3 BoxScale; float padding2;
		int CubeSpot; glm::vec3 padding3;
	};
	size_t m_envmapCount = 0ull;
	GL_ArrayBuffer<Reflector_Buffer> m_reflectorBuffer;
	FBO_Env_Reflector m_envmapFBO;

	// Reflector POV rendering
	std::shared_ptr<Viewport> m_reflectorViewport;
	bool m_renderingSelf = false; // used to avoid calling self infinitely
};

#endif // REFLECTOR_TECHNIQUE_H