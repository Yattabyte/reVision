#pragma once
#ifndef LIGHTINGDIRECTIONAL_S_H
#define LIGHTINGDIRECTIONAL_S_H 

#include "ECS\Systems\ecsSystem.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "ECS\Components\LightDirectional_C.h"
#include "ECS\Resources\FBO_Shadow_Directional.h"
#include "ECS\Systems\PropShadowing.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Utilities\PriorityList.h"
#include "Engine.h"
#include "GLFW\glfw3.h"
#include <random>


/** A system that performs lighting operations for directional lights. */
class LightingDirectional_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~LightingDirectional_System() {
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_engine->removePrefCallback(PreferenceState::C_SHADOW_QUALITY, this);
		m_engine->removePrefCallback(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, this);
		m_engine->removePrefCallback(PreferenceState::C_RH_BOUNCE_SIZE, this);
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
		if (m_shader_Lighting.get()) m_shader_Lighting->removeCallback(this);
	}
	LightingDirectional_System(
		Engine * engine, 
		FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * bounceFBO,
		GL_Vector * propBuffer, GL_Vector * skeletonBuffer
	) : BaseECSSystem(), m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_bounceFBO(bounceFBO) {
		// Declare component types used
		addComponentType(LightDirectional_Component::ID);
		addComponentType(LightDirectionalShadow_Component::ID, FLAG_OPTIONAL);

		// Asset Loading
		m_shader_Lighting = Asset_Shader::Create(m_engine, "Core\\Directional\\Light");
		m_shader_Shadow = Asset_Shader::Create(m_engine, "Core\\Directional\\Shadow");
		m_shader_Culling = Asset_Shader::Create(m_engine, "Core\\Directional\\Culling");
		m_shader_Bounce = Asset_Shader::Create(m_engine, "Core\\Directional\\Bounce");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {
			m_renderSize = glm::ivec2(f, m_renderSize.y);
		});
		m_renderSize.y = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {
			m_renderSize = glm::ivec2(m_renderSize.x, f);
		});	
		m_updateQuality = m_engine->addPrefCallback<unsigned int>(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {m_updateQuality = (unsigned int)f; });
		m_shadowSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, this, [&](const float &f) { m_shadowSize = glm::ivec2(std::max(1, (int)f)); });
		m_bounceSize = m_engine->addPrefCallback<GLuint>(PreferenceState::C_RH_BOUNCE_SIZE, this, [&](const float &f) { m_bounceSize = (GLuint)f; });
		m_drawDistance = m_engine->addPrefCallback<float>(PreferenceState::C_DRAW_DISTANCE, this, [&](const float &f) { m_drawDistance = f; });
		m_shadowSize = glm::ivec2(std::max(1, m_shadowSize.x));
		m_shadowFBO.resize(m_shadowSize, 4);

		// Asset-Finished Callbacks
		m_indirectShape = StaticBuffer(sizeof(GLuint) * 4); 
		m_indirectBounce = StaticBuffer(sizeof(GLuint) * 4);
		m_shapeQuad->addCallback(this, [&]() mutable {
			// count, primCount, first, reserved
			const GLuint data[4] = { (GLuint)m_shapeQuad->getSize(), 0, 0, 0 };
			m_indirectShape.write(0, sizeof(GLuint) * 4, &data); 
			m_indirectBounce.write(0, sizeof(GLuint) * 4, &data);
		});
		m_shader_Lighting->addCallback(this, [&](void) {m_shader_Lighting->setUniform(0, 1.0f / m_shadowSize.x); });

		// Noise Texture
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		glm::vec3 data[32 * 32 * 32];
		for (int x = 0, total = (32 * 32 * 32); x < total; ++x)
			data[x] = glm::vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator));
		glCreateTextures(GL_TEXTURE_3D, 1, &m_textureNoise32);
		glTextureImage3DEXT(m_textureNoise32, GL_TEXTURE_3D, 0, GL_RGB16F, 32, 32, 32, 0, GL_RGB, GL_FLOAT, &data);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureNoise32, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Geometry rendering pipeline
		m_geometrySystems.addSystem(new PropShadowing_System(engine, 4, PropShadowing_System::RenderAll, m_shader_Culling, m_shader_Shadow, propBuffer, skeletonBuffer));

		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Directional Shadowmap FBO", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Exit Early
		if (!m_shapeQuad->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet())
			return;

		// Clear Data
		lightIndices.clear();
		shadowIndices.clear();
		m_oldest.clear();
		calculateCascades();

		// Accumulate Light Data		
		auto & graphics = m_engine->getGraphicsModule();
		const auto cameraBuffer = graphics.m_cameraBuffer.getElement(graphics.getActiveCamera());
		const glm::mat4 CamInv = glm::inverse(cameraBuffer->data->vMatrix);
		const glm::mat4 CamP = cameraBuffer->data->pMatrix;
		for each (const auto & componentParam in components) {
			LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[0];
			LightDirectionalShadow_Component * shadowComponent = (LightDirectionalShadow_Component*)componentParam[1];
			lightIndices.push_back(lightComponent->m_data->index);
			if (shadowComponent) {
				shadowComponent->m_data->wait();
				shadowIndices.push_back(shadowComponent->m_data->index);
				m_oldest.insert(shadowComponent->m_updateTime, std::make_pair(lightComponent, shadowComponent));

				for (int i = 0; i < NUM_CASCADES; i++) {
					const glm::vec3 & middle = m_middle[i];			
					const glm::vec3 & aabb = m_aabb[i];
					const glm::vec3 volumeUnitSize = (aabb - -aabb) / (float)m_shadowSize.x;
					const glm::vec3 frustumpos = glm::vec3(shadowComponent->m_mMatrix * CamInv * glm::vec4(middle, 1.0f));
					const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
					const glm::vec3 newMin = -aabb + clampedPos;
					const glm::vec3 newMax = aabb + clampedPos;
					const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
					const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
					const glm::mat4 pvMatrix = pMatrix * shadowComponent->m_mMatrix;					
					shadowComponent->m_data->data->lightVP[i] = pvMatrix;
					shadowComponent->m_data->data->inverseVP[i] = inverse(pvMatrix);
					const glm::vec4 v1 = glm::vec4(0, 0, m_cascadeEnd[i + 1], 1.0f);
					const glm::vec4 v2 = CamP * v1;
					shadowComponent->m_data->data->CascadeEndClipSpace[i] = v2.z;
				}
				shadowComponent->m_data->lock();
			}
			else
				shadowIndices.push_back(-1);
		}

		// Update Draw Buffers
		const size_t & lightSize = lightIndices.size();
		const GLuint bounceInstanceCount = GLuint(shadowIndices.size() * m_bounceSize);
		m_visLights.write(0, sizeof(GLuint) *lightSize, lightIndices.data());
		m_visShadows.write(0, sizeof(GLuint) *shadowIndices.size(), shadowIndices.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &lightSize); // update primCount (2nd param)
		m_indirectBounce.write(sizeof(GLuint), sizeof(GLuint), &bounceInstanceCount);
		m_shader_Bounce->setUniform(0, (GLint)shadowIndices.size());

		// Bind buffers common for rendering and shadowing
		m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8); // Light buffer
		m_shadowBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9); // Shadow buffer

		// Render important shadows
		renderShadows(deltaTime);
		// Render lights
		renderLights(deltaTime);
		// Render indirect lights
		renderBounce(deltaTime);
	}
		

	// Public Attributes
	VectorBuffer<LightDirectional_Buffer> m_lightBuffer;
	VectorBuffer<LightDirectionalShadow_Buffer> m_shadowBuffer;
	FBO_Shadow_Directional m_shadowFBO;

	
protected:
	// Protected Methods
	/** Render all the geometry from each light. */
	void renderShadows(const float & deltaTime) {
		ECS & ecs = m_engine->getECS();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		m_shader_Shadow->bind();
		m_shadowFBO.bindForWriting();

		for each (const auto & pair in PQtoVector()) {
			const float clearDepth(1.0f);
			const glm::vec3 clear(0.0f);
			glUniform1i(0, pair.first->m_data->index);
			glUniform1i(1, pair.second->m_data->index);
			pair.second->m_data->wait();
			m_shadowFBO.clear(pair.second->m_shadowSpot);
			ecs.updateSystems(m_geometrySystems, deltaTime);
			pair.second->m_updateTime = (float)glfwGetTime();
		}

		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}
	/** Render all the lights. */
	void renderLights(const float & deltaTime) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		m_shader_Lighting->bind();										// Shader (directional)
		m_lightingFBO->bindForWriting();								// Ensure writing to lighting FBO
		m_geometryFBO->bindForReading();								// Read from Geometry FBO
		glBindTextureUnit(4, m_shadowFBO.m_textureIDS[2]);				// Shadow map (depth texture)
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);		// SSBO visible shadow indices
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_shapeQuad->m_vaoID);						// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);							// Now draw

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
	}
	/** Render light bounces. */
	void renderBounce(const float & deltaTime) {
		auto & graphics = m_engine->getGraphicsModule();
		const auto cameraBuffer = graphics.m_cameraBuffer.getElement(graphics.getActiveCamera());
		const glm::mat4 CamInv = glm::inverse(cameraBuffer->data->vMatrix);
		const glm::vec2 &size = cameraBuffer->data->Dimensions;
		const float ar = size.x / size.y;
		const float tanHalfHFOV = glm::radians(cameraBuffer->data->FOV) / 2.0f;
		const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
		const float near_plane = -CAMERA_NEAR_PLANE;
		const float far_plane = -m_drawDistance;
		const float cascadeEnd[2] = {
			near_plane, (far_plane * 0.3f)
		};
		const float points[4] = {
			cascadeEnd[0] * tanHalfHFOV,
			cascadeEnd[1] * tanHalfHFOV,
			cascadeEnd[0] * tanHalfVFOV,
			cascadeEnd[1] * tanHalfVFOV
		};
		glm::vec3 frustumCorners[8] = {
			// near face
			glm::vec3(points[0], points[2], cascadeEnd[0]),
			glm::vec3(-points[0], points[2], cascadeEnd[0]),
			glm::vec3(points[0], -points[2], cascadeEnd[0]),
			glm::vec3(-points[0], -points[2], cascadeEnd[0]),
			// far face
			glm::vec3(points[1], points[3], cascadeEnd[1]),
			glm::vec3(-points[1], points[3], cascadeEnd[1]),
			glm::vec3(points[1], -points[3], cascadeEnd[1]),
			glm::vec3(-points[1], -points[3], cascadeEnd[1])
		};
		glm::vec3 middle(0, 0, ((cascadeEnd[1] - cascadeEnd[0]) / 2.0f) + cascadeEnd[0]);
		glm::vec3 aabb(glm::distance(frustumCorners[7], middle));
		const glm::vec3 volumeUnitSize = (aabb - -aabb) / float(m_bounceSize);
		const glm::vec3 frustumpos = (CamInv * glm::vec4(middle, 1.0f));
		const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
		const glm::vec3 newMin = -aabb + clampedPos;
		const glm::vec3 newMax = aabb + clampedPos;
		m_shader_Bounce->setUniform(1, newMax);
		m_shader_Bounce->setUniform(2, newMin);
		m_shader_Bounce->setUniform(4, (float)m_bounceSize);
		m_shader_Bounce->setUniform(6, volumeUnitSize.x);

		// Prepare rendering state
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
		glBindVertexArray(m_shapeQuad->m_vaoID);

		glViewport(0, 0, m_bounceSize, m_bounceSize);
		m_bounceFBO->bindForWriting();
		m_shader_Bounce->bind();		
		m_geometryFBO->bindForReading(0); // depth -> 3		
		glBindTextureUnit(0, m_shadowFBO.m_textureIDS[0]);
		glBindTextureUnit(1, m_shadowFBO.m_textureIDS[1]);
		glBindTextureUnit(2, m_shadowFBO.m_textureIDS[2]);
		glBindTextureUnit(4, m_textureNoise32);
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);		// SSBO visible shadow indices
		m_indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	}


private:
	// Private methods
	const std::vector<std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>> PQtoVector() const {
		PriorityList<float, std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>, std::greater<float>> m_closest(m_updateQuality / 2);
		std::vector<std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>> outList;
		outList.reserve(m_updateQuality);

		for each (const auto &element in m_oldest.toList()) {
			if (outList.size() < (m_updateQuality / 2))
				outList.push_back(element);
			else
				m_closest.insert(element.second->m_updateTime, element);
		}

		for each (const auto &element in m_closest.toList()) {
			if (outList.size() >= m_updateQuality)
				break;
			outList.push_back(element);
		}

		return outList;
	}
	void calculateCascades() {
		auto & graphics = m_engine->getGraphicsModule();
		const auto cameraBuffer = graphics.m_cameraBuffer.getElement(graphics.getActiveCamera());	
		const glm::vec2 &size = cameraBuffer->data->Dimensions;
		const float ar = size.x / size.y;
		const float tanHalfHFOV = glm::radians(cameraBuffer->data->FOV) / 2.0f;
		const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
		const float near_plane = -0.1f;
		const float far_plane = -m_drawDistance;
		m_cascadeEnd[0] = near_plane;
		for (int x = 1; x < NUM_CASCADES + 1; ++x) {
			const float cLog = near_plane * powf((far_plane / near_plane), (float(x) / float(NUM_CASCADES)));
			const float cUni = near_plane + ((far_plane - near_plane) * x / NUM_CASCADES);
			const float lambda = 0.75f;
			m_cascadeEnd[x] = (lambda*cLog) + ((1.0f - lambda)*cUni);
		}
		for (int i = 0; i < NUM_CASCADES; i++) {			
			float points[4] = {
				m_cascadeEnd[i] * tanHalfHFOV,
				m_cascadeEnd[i + 1] * tanHalfHFOV,
				m_cascadeEnd[i] * tanHalfVFOV,
				m_cascadeEnd[i + 1] * tanHalfVFOV
			};
			glm::vec3 frustumCorners[8] = {
				// near face
				glm::vec3(points[0], points[2], m_cascadeEnd[i]),
				glm::vec3(-points[0], points[2], m_cascadeEnd[i]),
				glm::vec3(points[0], -points[2], m_cascadeEnd[i]),
				glm::vec3(-points[0], -points[2], m_cascadeEnd[i]),
				// far face
				glm::vec3(points[1], points[3], m_cascadeEnd[i + 1]),
				glm::vec3(-points[1], points[3], m_cascadeEnd[i + 1]),
				glm::vec3(points[1], -points[3], m_cascadeEnd[i + 1]),
				glm::vec3(-points[1], -points[3], m_cascadeEnd[i + 1])
			};

			// Find the middle of current view frustum chunk
			m_middle[i] = glm::vec3(0, 0, ((m_cascadeEnd[i + 1] - m_cascadeEnd[i]) / 2.0f) + m_cascadeEnd[i]);

			// Measure distance from middle to the furthest point of frustum slice
			// Use to make a bounding sphere, but then convert into a bounding box
			m_aabb[i] = glm::vec3(glm::distance(frustumCorners[7], m_middle[i]));
		}	
	}


	// Private Attributes
	Engine * m_engine;
	Shared_Asset_Shader m_shader_Lighting, m_shader_Shadow, m_shader_Culling, m_shader_Bounce;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_updateQuality = 1u;
	glm::ivec2 m_renderSize = glm::ivec2(1920, 1080);
	glm::ivec2 m_shadowSize = glm::ivec2(1024);
	GLuint m_bounceSize = 16u;
	StaticBuffer m_indirectShape, m_indirectBounce;
	std::vector<GLint> lightIndices, shadowIndices;
	DynamicBuffer m_visLights, m_visShadows;
	PriorityList<float, std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>, std::less<float>> m_oldest;
	ECSSystemList m_geometrySystems;
	FBO_Base * m_geometryFBO, *m_lightingFBO, * m_bounceFBO;
	GLuint m_textureNoise32 = 0;
	float m_drawDistance = 100.0f;
	float m_cascadeEnd[NUM_CASCADES + 1];
	glm::vec3 m_middle[NUM_CASCADES], m_aabb[NUM_CASCADES];
};

#endif // LIGHTINGDIRECTIONAL_S_H