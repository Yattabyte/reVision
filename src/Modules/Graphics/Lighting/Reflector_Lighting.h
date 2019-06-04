#pragma once
#ifndef REFLECTOR_LIGHTING_H
#define REFLECTOR_LIGHTING_H

#include "Modules/Graphics/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Geometry/Prop_Shadow.h"
#include "Modules/Graphics/Common/FBO_Shadow_Spot.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/FBO.h"
#include "Utilities/PriorityList.h"
#include "Engine.h"
#include <vector>


/***/
class Reflector_Lighting : public Graphics_Technique, public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Reflector_Lighting() {
		// Update indicator
		m_aliveIndicator = false;
		auto & world = m_engine->getModule_World(); 
		world.removeComponentType("Reflector_Component");
	}
	/** Constructor. */
	inline Reflector_Lighting(Engine * engine, FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO)
		: m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_reflectionFBO(reflectionFBO) {
		// Asset Loading
		m_shaderLighting = Shared_Shader(m_engine, "Core\\Reflector\\IBL_Parallax");
		m_shaderStencil = Shared_Shader(m_engine, "Core\\Reflector\\Stencil");
		m_shaderCopy = Shared_Shader(m_engine, "Core\\Reflector\\2D_To_Cubemap");
		m_shaderConvolute = Shared_Shader(m_engine, "Core\\Reflector\\Cube_Convolution");
		m_shapeCube = Shared_Primitive(m_engine, "cube");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) { m_renderSize = glm::ivec2(f, m_renderSize.y); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) { m_renderSize = glm::ivec2(m_renderSize.x, f); });
		preferences.getOrSetValue(PreferenceState::C_ENVMAP_SIZE, m_envmapSize);
		preferences.addCallback(PreferenceState::C_ENVMAP_SIZE, m_aliveIndicator, [&](const float &f) {
			m_envmapSize = std::max(1u, (unsigned int)f);
			m_envmapFBO.resize(m_envmapSize, m_envmapSize, 6);
		});

		// Environment Map
		m_envmapFBO.resize(m_envmapSize, m_envmapSize, 6);

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
			m_engine->getManager_Messages().error("Reflector_FX Environment Map Framebuffer has encountered an error.");		
		
		// Declare component types used
		addComponentType(Reflector_Component::ID);
		GLuint data[] = { 0,0,0,0 };
		m_indirectCube.write(0, sizeof(GLuint) * 4, &data);
		m_indirectQuad.write(0, sizeof(GLuint) * 4, &data);
		m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, &data);

		// Error Reporting
		if (!isValid())
			engine->getManager_Messages().error("Invalid ECS System: Reflector_Lighting");
		
		auto & world = m_engine->getModule_World();
		world.addLevelListener(&m_outOfDate);
		world.addComponentType("Reflector_Component", [&, engine](const ParamList & parameters) {
			auto envCount = (int)(m_reflectorBuffer.getCount() * 6);
			auto * component = new Reflector_Component();
			component->m_data = m_reflectorBuffer.newElement();
			component->m_data->data->CubeSpot = envCount;
			component->m_cubeSpot = envCount;
			m_envmapFBO.resize(m_envmapFBO.m_size.x, m_envmapFBO.m_size.y, envCount + 6);
			component->m_outOfDate = true;
			for (int x = 0; x < 6; ++x) {
				component->m_Cameradata[x].Dimensions = m_envmapFBO.m_size;
				component->m_Cameradata[x].FOV = 90.0f;
			}
			return std::make_pair(component->ID, component);
		});
	}


	// Public Interface Implementations
	inline virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shapeCube->existsYet() || !m_shapeQuad->existsYet() || !m_shaderLighting->existsYet() || !m_shaderStencil->existsYet() || !m_shaderCopy->existsYet() || !m_shaderConvolute->existsYet())
			return;

		// Render scene
		renderScene(deltaTime);
		// Render reflectors
		renderReflectors(deltaTime);
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate Reflector Data		
		std::vector<GLint> reflectionIndicies;
		PriorityList<float, Reflector_Component*, std::less<float>> oldest;
		for each (const auto & componentParam in components) {
			Reflector_Component * component = (Reflector_Component*)componentParam[0];
			reflectionIndicies.push_back(component->m_data->index);
			oldest.insert(component->m_updateTime, component);
		}

		// Update Draw Buffers
		const size_t & refSize = reflectionIndicies.size();
		m_visLights.write(0, sizeof(GLuint) *refSize, reflectionIndicies.data());
		m_indirectCube.write(sizeof(GLuint), sizeof(GLuint), &refSize); // update primCount (2nd param)
		m_reflectorsToUpdate = PQtoVector(oldest);
	}


private:
	// Protected Methods
	/** Render all the geometry for each reflector */
	inline void renderScene(const float & deltaTime) {
		auto & preferences = m_engine->getPreferenceState();
		auto & graphics = m_engine->getModule_Graphics();
		bool didAnything = false, update = m_outOfDate;
		m_outOfDate = false;
		for each (auto reflector in std::vector<Reflector_Component*>(m_reflectorsToUpdate)) {
			if (update || reflector->m_outOfDate) {
				if (!didAnything) {
					auto copySize = m_renderSize;
					preferences.setValue(PreferenceState::C_WINDOW_WIDTH, m_envmapSize);
					preferences.setValue(PreferenceState::C_WINDOW_HEIGHT, m_envmapSize);
					glViewport(0, 0, m_envmapSize, m_envmapSize);
					m_renderSize = copySize;
					didAnything = true;
				}
				reflector->m_outOfDate = false;
				for (int x = 0; x < 6; ++x) {
					// For line below, figure out a way to copy data from the reflector data into main camera, temporarily
					// We removed the camera buffer array, as it only served this one purpose ever, and slowed everything down
					//graphics.setActiveCamera(reflector->m_Cameradata[x]->index);
					graphics.frameTick(deltaTime);

					// Copy lighting frame into cube-face
					m_lightingFBO->bindForReading();
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
			}
		}
		if (didAnything) {
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			Shader::Release();
			glViewport(0, 0, m_renderSize.x, m_renderSize.y);
			preferences.setValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
			preferences.setValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		}
	}
	/** Render all the lights */
	inline void renderReflectors(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shaderStencil->bind();													// Shader (reflector)
		m_reflectionFBO->bindForWriting();											// Ensure writing to reflection FBO
		m_geometryFBO->bindForReading();											// Read from Geometry FBO
		glBindTextureUnit(4, m_envmapFBO.m_textureID);								// Reflection map (environment texture)
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_reflectorBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);				// Reflection buffer
		m_indirectCube.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_shapeCube->m_vaoID);								 	// Quad VAO
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
	/** Converts a priority queue into an stl vector.*/
	inline std::vector<Reflector_Component*> PQtoVector(PriorityList<float, Reflector_Component*, std::less<float>> oldest) const {
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


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Primitive m_shapeCube, m_shapeQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	Shared_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	FBO_Base * m_geometryFBO = nullptr, *m_lightingFBO = nullptr, *m_reflectionFBO = nullptr;
	VectorBuffer<Reflector_Component::GL_Buffer> m_reflectorBuffer;
	FBO_EnvMap m_envmapFBO;
	StaticBuffer m_indirectCube = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad6Faces = StaticBuffer(sizeof(GLuint) * 4);
	GLuint m_envmapSize = 512u;
	DynamicBuffer m_visLights;
	std::vector<Reflector_Component*> m_reflectorsToUpdate;
	bool m_outOfDate = true;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // REFLECTOR_LIGHTING_H