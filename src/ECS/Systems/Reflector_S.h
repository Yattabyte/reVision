#pragma once
#ifndef REFLECTOR_S_H
#define REFLECTOR_S_H 

#include "ECS\Systems\ecsSystem.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Texture.h"
#include "ECS\Components\Reflector_C.h"
#include "ECS\Resources\FBO_EnvMap.h"
#include "ECS\Resources\FBO_Geometry.h"
#include "ECS\Resources\FBO_Lighting.h"
#include "Utilities\PriorityList.h"
#include "Engine.h"
#include "GLFW\glfw3.h"


/** A system that performs lighting operations for spot lights. */
class Reflector_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Reflector_System() {
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_engine->removePrefCallback(PreferenceState::C_ENVMAP_SIZE, this);
		if (m_shapeCube.get()) m_shapeCube->removeCallback(this);
		glDeleteVertexArrays(1, &m_cubeVAO);
		glDeleteVertexArrays(1, &m_quadVAO);
	}
	Reflector_System(
		Engine * engine,
		FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO,
		GL_Vector * propBuffer, GL_Vector * skeletonBuffer
	) : BaseECSSystem() {
		// Declare component types used
		addComponentType(Reflector_Component::ID);

		// Shared Parameters
		m_engine = engine;
		m_geometryFBO = geometryFBO;
		m_lightingFBO = lightingFBO;
		m_reflectionFBO = reflectionFBO;

		// Asset Loading
		m_shaderLighting = Asset_Shader::Create(m_engine, "Core\\Reflector\\IBL_Parallax");
		m_shaderCopy = Asset_Shader::Create(m_engine, "Core\\Reflector\\2D_To_Cubemap");
		m_shaderConvolute = Asset_Shader::Create(m_engine, "Core\\Reflector\\Cube_Convolution");
		m_brdfMap = Asset_Texture::Create(m_engine, "brdfLUT.png", GL_TEXTURE_2D, false, false);
		m_shapeCube = Asset_Primitive::Create(m_engine, "cube");
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {
			m_renderSize = glm::ivec2(f, m_renderSize.y);
		});
		m_renderSize.y = m_engine->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {
			m_renderSize = glm::ivec2(m_renderSize.x, f);
		});
		m_envmapSize = m_engine->addPrefCallback(PreferenceState::C_ENVMAP_SIZE, this, [&](const float &f) { 
			m_envmapSize = max(1.0f, f);
		});
	
		// Environment Map
		m_envCount = 0;
		m_envmapFBO.resize(m_envmapSize, m_envmapSize, 6);

		// Primitive Construction
		m_cubeVAOLoaded = false;
		m_cubeVAO = Asset_Primitive::Generate_VAO();
		m_indirectCube = StaticBuffer(sizeof(GLuint) * 4, 0);
		m_shapeCube->addCallback(this, [&]() mutable {
			m_cubeVAOLoaded = true;
			m_shapeCube->updateVAO(m_cubeVAO);
			const GLuint data = m_shapeCube->getSize();
			m_indirectCube.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});
		m_quadVAOLoaded = false;
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_indirectQuad = StaticBuffer(sizeof(GLuint) * 4, 0);
		m_indirectQuad6Faces = StaticBuffer(sizeof(GLuint) * 4, 0);
		m_shapeQuad->addCallback(this, [&]() mutable {
			m_quadVAOLoaded = true;
			m_shapeQuad->updateVAO(m_quadVAO);
			const GLuint quadData[4] = { m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_indirectQuad.write(0, sizeof(GLuint) * 4, quadData); 
			const GLuint quad6Data[4] = { m_shapeQuad->getSize(), 6, 0, 0 }; 
			m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, quad6Data);
		});
		
		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_envmapFBO.m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Reflector Environment map FBO", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
		// Exit Early
		if (!m_cubeVAOLoaded || !m_shaderLighting || !m_shaderLighting->existsYet())
			return;

		// Clear Data
		reflectionIndicies.clear();
		m_oldest.clear();

		// Accumulate Reflector Data		
		for each (const auto & componentParam in components) {
			Reflector_Component * component = (Reflector_Component*)componentParam[0];
			reflectionIndicies.push_back(component->m_data->index);
			m_oldest.insert(component->m_updateTime, component);
		}

		// Update Draw Buffers
		const size_t & refSize = reflectionIndicies.size();
		m_visLights.write(0, sizeof(GLuint) *refSize, reflectionIndicies.data());
		m_indirectCube.write(sizeof(GLuint), sizeof(GLuint), &refSize); // update primCount (2nd param)

		// Render scene
		renderScene(deltaTime);
		// Render reflectors
		renderReflectors(deltaTime);
	}


	// Public Methods
	/** Registers a rfeflector component.
	@param	component	the light component to register. */
	void registerComponent(Reflector_Component & component) {
		component.m_data = m_reflectorBuffer.newElement();
		component.m_data->data->CubeSpot = m_envCount;
		component.m_cubeSpot = m_envCount;
		m_envCount += 6;
		m_envmapFBO.resize(m_envmapSize, m_envmapSize, m_envCount);
		// Default Values
		component.m_data->data->mMatrix = glm::mat4(1.0f);
		component.m_data->data->rotMatrix = glm::mat4(1.0f);
		component.m_data->data->BoxCamPos = glm::vec3(0.0f);
		component.m_data->data->BoxScale = glm::vec3(0.0f);
		component.m_outOfDate = true;

		for (int x = 0; x < 6; ++x) {
			component.m_Cameradata[x] = m_engine->getGraphicsModule().m_cameraBuffer.newElement();
			component.m_Cameradata[x]->data->pMatrix = glm::mat4(1.0f);
			component.m_Cameradata[x]->data->pMatrix_Inverse = glm::inverse(glm::mat4(1.0f));
			component.m_Cameradata[x]->data->vMatrix = glm::mat4(1.0f);
			component.m_Cameradata[x]->data->vMatrix_Inverse = glm::inverse(glm::mat4(1.0f));
			component.m_Cameradata[x]->data->EyePosition = glm::vec3(0.0f);
			component.m_Cameradata[x]->data->Dimensions = glm::vec2(512.0f);
			component.m_Cameradata[x]->data->NearPlane = 0.01f;
			component.m_Cameradata[x]->data->FarPlane = 1.0f;
			component.m_Cameradata[x]->data->FOV = 90.0f;
			component.m_Cameradata[x]->data->Gamma = 1.0f;
		}
	}
	bool & outOfDate() { return m_outOfDate; }


	// Public Attributes
	VectorBuffer<Reflection_Buffer> m_reflectorBuffer;


protected:
	// Protected Methods
	/** Render all the geometry for each reflector */
	void renderScene(const float & deltaTime) {
		if (!m_shaderCopy || !m_shaderCopy->existsYet() || !m_shaderConvolute || !m_shaderConvolute->existsYet() || !m_quadVAOLoaded)
			return;
		auto & graphics = m_engine->getGraphicsModule();
		bool didAnything = false, update = m_outOfDate;
		m_outOfDate = false;
		GLuint oldCameraID = 0;
		for each (auto * reflector in PQtoVector()) {		
			if (update || reflector->m_outOfDate) {
				if (!didAnything) {
					auto copySize = m_renderSize;
					m_engine->setPreference(PreferenceState::C_WINDOW_WIDTH, m_envmapSize);
					m_engine->setPreference(PreferenceState::C_WINDOW_HEIGHT, m_envmapSize);
					glViewport(0, 0, m_envmapSize, m_envmapSize);
					m_renderSize = copySize;
					oldCameraID = graphics.getActiveCamera();
					didAnything = true;
				}
				reflector->m_outOfDate = false;
				for (int x = 0; x < 6; ++x) {
					graphics.setActiveCamera(reflector->m_Cameradata[x]->index);
					graphics.renderFrame(deltaTime);

					// Copy lighting frame into cube-face
					m_lightingFBO->bindForReading();
					m_envmapFBO.bindForWriting();
					m_shaderCopy->bind();
					m_shaderCopy->Set_Uniform(0, x + reflector->m_cubeSpot);
					m_indirectQuad.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
					glBindVertexArray(m_quadVAO);
					glDrawArraysIndirect(GL_TRIANGLES, 0);
				}
				// Once cubemap is generated, convolute it
				m_shaderConvolute->bind();
				m_shaderConvolute->Set_Uniform(0, reflector->m_cubeSpot);
				m_envmapFBO.bindForReading();
				m_indirectQuad6Faces.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				for (float r = 1; r < 6; ++r) {
					// Ensure we are writing to MIP level r
					const float write_size = max(1.0f, (floor(m_envmapSize / pow(2.0f, r))));
					glViewport(0, 0, write_size, write_size);
					m_shaderConvolute->Set_Uniform(1, r / 5.0f);
					glNamedFramebufferTexture(m_envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_envmapFBO.m_textureID, r);

					// Ensure we are reading from MIP level r - 1
					glTextureParameterf(m_envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, r - 1.0f);
					glTextureParameterf(m_envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, r - 1.0f);

					// Convolute the 6 faces for this roughness level (RENDERS 6 TIMES)
					glDrawArraysIndirect(GL_TRIANGLES, 0);
				}

				// Reset texture, so it can be used for other component reflections
				glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, 0);
				glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, 5);
				glNamedFramebufferTexture(m_envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_envmapFBO.m_textureID, 0);
				reflector->m_updateTime = glfwGetTime();
			}
		}			
		if (didAnything) {
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			Asset_Shader::Release();
			glViewport(0, 0, m_renderSize.x, m_renderSize.y);
			graphics.setActiveCamera(oldCameraID);
			m_engine->setPreference(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
			m_engine->setPreference(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		}
	}
	/** Render all the lights */
	void renderReflectors(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		// Draw only into depth-stencil buffer
		m_shaderLighting->bind();										// Shader (spot)
		m_reflectionFBO->bindForWriting();								// Ensure writing to lighting FBO
		m_geometryFBO->bindForReading();								// Read from Geometry FBO
		glBindTextureUnit(4, m_envmapFBO.m_textureID);					// Reflection map (environment texture)
		m_brdfMap->bind(5);												// Environment BRDF
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_reflectorBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);	// Reflection buffer
		m_indirectCube.bindBuffer(GL_DRAW_INDIRECT_BUFFER);				// Draw call buffer
		glBindVertexArray(m_cubeVAO);									// Quad VAO
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		m_shaderLighting->Set_Uniform(1, true);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Now draw into color buffers
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glCullFace(GL_FRONT);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		m_shaderLighting->Set_Uniform(1, false);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glCullFace(GL_BACK);
		glDepthMask(GL_TRUE);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_STENCIL_TEST);
	}


private:
	// Private methods
	const std::vector<Reflector_Component*> PQtoVector() const {
		PriorityList<float, Reflector_Component*, std::greater<float>> m_closest(2);
		std::vector<Reflector_Component*> outList;
		outList.reserve(2);

		for each (const auto &element in m_oldest.toList()) {
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
	Engine * m_engine;
	Shared_Asset_Shader m_shaderLighting, m_shaderCopy, m_shaderConvolute;
	Shared_Asset_Primitive m_shapeCube, m_shapeQuad;
	Shared_Asset_Texture m_brdfMap;
	GLuint m_cubeVAO, m_quadVAO;
	bool m_cubeVAOLoaded, m_quadVAOLoaded;
	StaticBuffer m_indirectCube, m_indirectQuad, m_indirectQuad6Faces;
	glm::ivec2	m_renderSize;
	GLuint m_envmapSize;
	GLuint m_envCount;
	std::vector<GLuint> reflectionIndicies;
	DynamicBuffer m_visLights;
	PriorityList<float, Reflector_Component*, std::less<float>> m_oldest;
	FBO_Base * m_geometryFBO, *m_lightingFBO, *m_reflectionFBO;
	FBO_EnvMap m_envmapFBO;
	bool m_outOfDate = true;
};

#endif // REFLECTOR_S_H