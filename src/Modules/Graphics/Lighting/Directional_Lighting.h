#pragma once
#ifndef DIRECTIONAL_LIGHTING_H
#define DIRECTIONAL_LIGHTING_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/FBO_Shadow_Directional.h"
#include "Modules/Graphics/Geometry/Prop_Shadow.h"
#include "Modules/World/ECS/TransformComponent.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "Engine.h"
#include <random>
#include <vector>

#define NUM_CASCADES 4


/***/
class Directional_Lighting : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Directional_Lighting() {
		// Update indicator
		m_aliveIndicator = false;
		auto & world = m_engine->getModule_World();
		world.removeNotifyOnComponentType("LightDirectional_Component", m_notifyLight);
		world.removeNotifyOnComponentType("LightDirectionalShadow_Component", m_notifyShadow);
	}
	/** Constructor. */
	inline Directional_Lighting(Engine * engine, Prop_View * propView)
		: m_engine(engine) {
		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Directional\\Light");
		m_shader_Shadow = Shared_Shader(m_engine, "Core\\Directional\\Shadow");
		m_shader_Culling = Shared_Shader(m_engine, "Core\\Directional\\Culling");
		m_shader_Bounce = Shared_Shader(m_engine, "Core\\Directional\\Bounce");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_QUALITY, m_updateQuality);
		preferences.addCallback(PreferenceState::C_SHADOW_QUALITY, m_aliveIndicator, [&](const float &f) { m_updateQuality = (unsigned int)f; });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, m_shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, m_aliveIndicator, [&](const float &f) {
			m_shadowSize = glm::ivec2(std::max(1, (int)f));
			m_shadowFBO.resize(m_shadowSize, 4);
			if (m_shader_Lighting && m_shader_Lighting->existsYet())
				m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / m_shadowSize.x); });
		});
		preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
		preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_bounceSize = (GLuint)f; });
		m_shadowSize = glm::ivec2(std::max(1, m_shadowSize.x));
		m_shadowFBO.resize(m_shadowSize, 4);

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			// count, primCount, first, reserved
			const GLuint data[4] = { (GLuint)m_shapeQuad->getSize(), 0, 0, 0 };
			m_indirectShape.write(0, sizeof(GLuint) * 4, &data);
			m_indirectBounce.write(0, sizeof(GLuint) * 4, &data);
		});
		m_shader_Lighting->addCallback(m_aliveIndicator, [&](void) { m_shader_Lighting->setUniform(0, 1.0f / m_shadowSize.x); });

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
		m_propShadowSystem = new Prop_Shadow(engine, 4, Prop_Shadow::RenderAll, m_shader_Culling, m_shader_Shadow, propView);

		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_shadowFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine->getManager_Messages().error("Directional_Lighting Shadowmap Framebuffer has encountered an error.");

		// Declare component types used
		addComponentType(LightDirectional_Component::ID);
		addComponentType(LightDirectionalShadow_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		GLuint indData[] = { 0,0,0,0 };
		m_indirectShape.write(0, sizeof(GLuint) * 4, &indData);

		// Error Reporting
		if (!isValid())
			engine->getManager_Messages().error("Invalid ECS System: Directional_Lighting");

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		m_notifyLight = world.addNotifyOnComponentType("LightDirectional_Component", [&](BaseECSComponent * c) {
			auto * component = (LightDirectional_Component*)c;
			component->m_lightIndex = m_lightBuffer.newElement();
		});
		m_notifyShadow = world.addNotifyOnComponentType("LightDirectionalShadow_Component", [&](BaseECSComponent * c) {
			auto * component = (LightDirectionalShadow_Component*)c;
			auto shadowSpot = (int)(m_shadowBuffer.getLength() * 4);
			component->m_shadowIndex = m_shadowBuffer.newElement();
			m_shadowBuffer[*component->m_shadowIndex].Shadow_Spot = shadowSpot;
			component->m_shadowSpot = shadowSpot;
			m_shadowFBO.resize(m_shadowFBO.m_size, shadowSpot + 4);
			// Default Values
			m_shadowBuffer[*component->m_shadowIndex].lightV = glm::mat4(1.0f);
			for (int x = 0; x < NUM_CASCADES; ++x) {
				m_shadowBuffer[*component->m_shadowIndex].lightVP[x] = glm::mat4(1.0f);
				m_shadowBuffer[*component->m_shadowIndex].inverseVP[x] = glm::inverse(glm::mat4(1.0f));
			}
		});
	}


	// Public Interface Implementations
	inline virtual void applyTechnique(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet() || !m_shader_Bounce->existsYet())
			return;

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
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate Light Data
		const glm::vec2 &size = (*m_cameraBuffer)->Dimensions;
		const float ar = size.x / size.y;
		const float tanHalfHFOV = glm::radians((*m_cameraBuffer)->FOV) / 2.0f;
		const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
		const float near_plane = -CameraBuffer::BufferStructure::ConstNearPlane;
		const float far_plane = -(*m_cameraBuffer)->FarPlane;
		float cascadeEnd[NUM_CASCADES + 1];
		glm::vec3 middle[NUM_CASCADES], aabb[NUM_CASCADES];
		constexpr float lambda = 0.75f;
		cascadeEnd[0] = near_plane;
		for (int x = 1; x < NUM_CASCADES + 1; ++x) {
			const float xDivM = float(x) / float(NUM_CASCADES);
			const float cLog = near_plane * powf((far_plane / near_plane), xDivM);
			const float cUni = near_plane + (far_plane - near_plane) * xDivM;
			cascadeEnd[x] = (lambda * cLog) + (1.0f - lambda) * cUni;
		}
		for (int i = 0; i < NUM_CASCADES; i++) {
			float points[4] = {
				cascadeEnd[i] * tanHalfHFOV,
				cascadeEnd[i + 1] * tanHalfHFOV,
				cascadeEnd[i] * tanHalfVFOV,
				cascadeEnd[i + 1] * tanHalfVFOV
			};
			float maxCoord = std::max(abs(cascadeEnd[i]), abs(cascadeEnd[i + 1]));
			for (int x = 0; x < 4; ++x)
				maxCoord = std::max(maxCoord, abs(points[x]));
			// Find the middle of current view frustum chunk
			middle[i] = glm::vec3(0, 0, ((cascadeEnd[i + 1] - cascadeEnd[i]) / 2.0f) + cascadeEnd[i]);

			// Measure distance from middle to the furthest point of frustum slice
			// Use to make a bounding sphere, but then convert into a bounding box
			aabb[i] = glm::vec3(glm::distance(glm::vec3(maxCoord), middle[i]));
		}
		const glm::mat4 CamInv = glm::inverse((*m_cameraBuffer)->vMatrix);
		const glm::mat4 CamP = (*m_cameraBuffer)->pMatrix;
		std::vector<GLint> lightIndices, shadowIndices;
		m_shadowsToUpdate.clear();
		for each (const auto & componentParam in components) {
			LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[0];
			LightDirectionalShadow_Component * shadowComponent = (LightDirectionalShadow_Component*)componentParam[1];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
			const auto & index = *lightComponent->m_lightIndex;

			// Sync Transform Attributes
			if (transformComponent) {
				const auto & orientation = transformComponent->m_transform.m_orientation;
				const auto matRot = glm::mat4_cast(orientation);
				const glm::mat4 sunTransform = matRot;
				lightComponent->m_direction = glm::vec3(glm::normalize(sunTransform * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
				
				if (shadowComponent) {
					const glm::mat4 sunTransform = matRot;
					const glm::mat4 sunModelMatrix = glm::inverse(sunTransform * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
					shadowComponent->m_orientation = orientation;					
					shadowComponent->m_mMatrix = sunModelMatrix;
					m_shadowBuffer[*shadowComponent->m_shadowIndex].lightV = sunModelMatrix;
				}
			}

			// Update Buffer Attributes
			m_lightBuffer[index].LightColor = lightComponent->m_color;
			m_lightBuffer[index].LightDirection = lightComponent->m_direction;
			m_lightBuffer[index].LightIntensity = lightComponent->m_intensity;
			lightIndices.push_back(index);
			if (shadowComponent) {
				shadowIndices.push_back(*shadowComponent->m_shadowIndex);
				m_shadowsToUpdate.push_back(std::make_pair(lightComponent, shadowComponent));
				for (int i = 0; i < NUM_CASCADES; i++) {
					const glm::vec3 volumeUnitSize = (aabb[i] - -aabb[i]) / (float)m_shadowSize.x;
					const glm::vec3 frustumpos = glm::vec3(shadowComponent->m_mMatrix * CamInv * glm::vec4(middle[i], 1.0f));
					const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
					const glm::vec3 newMin = -aabb[i] + clampedPos;
					const glm::vec3 newMax = aabb[i] + clampedPos;
					const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
					const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
					const glm::mat4 pvMatrix = pMatrix * shadowComponent->m_mMatrix;
					const glm::vec4 v1 = glm::vec4(0, 0, cascadeEnd[i + 1], 1.0f);
					const glm::vec4 v2 = CamP * v1;
					m_shadowBuffer[*shadowComponent->m_shadowIndex].lightVP[i] = pvMatrix;
					m_shadowBuffer[*shadowComponent->m_shadowIndex].inverseVP[i] = inverse(pvMatrix);
					m_shadowBuffer[*shadowComponent->m_shadowIndex].CascadeEndClipSpace[i] = v2.z;
				}
			}
			else
				shadowIndices.push_back(-1);
		}

		// Update Draw Buffers
		const size_t & lightSize = lightIndices.size(), &shadowSize = shadowIndices.size();
		const GLuint bounceInstanceCount = GLuint(shadowSize * m_bounceSize);
		m_visLights.write(0, sizeof(GLuint) * lightSize, lightIndices.data());
		m_visShadows.write(0, sizeof(GLuint) * shadowSize, shadowIndices.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &lightSize); // update primCount (2nd param)
		m_indirectBounce.write(sizeof(GLuint), sizeof(GLuint), &bounceInstanceCount);
		m_shadowCount = (GLint)shadowSize;
	}


private:
	// Private Methods
	/** Render all the geometry from each light. */
	inline void renderShadows(const float & deltaTime) {
		auto & world = m_engine->getModule_World();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		m_shader_Shadow->bind();
		m_shadowFBO.bindForWriting();

		for (const auto[light, shadow] : m_shadowsToUpdate) {
			const float clearDepth(1.0f);
			const glm::vec3 clear(0.0f);
			m_shadowFBO.clear(shadow->m_shadowSpot);
			// Update geometry components
			m_propShadowSystem->setData((*m_cameraBuffer)->EyePosition, *light->m_lightIndex, *shadow->m_shadowIndex);
			world.updateSystem(m_propShadowSystem, deltaTime);
			// Render geometry components
			m_propShadowSystem->applyTechnique(deltaTime);
			shadow->m_updateTime = m_engine->getTime();
		}

		glViewport(0, 0, (*m_cameraBuffer)->Dimensions.x, (*m_cameraBuffer)->Dimensions.y);
	}
	/** Render all the lights. */
	inline void renderLights(const float & deltaTime) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		m_shader_Lighting->bind();									// Shader (directional)
		m_gfxFBOS->bindForWriting("LIGHTING");						// Ensure writing to lighting FBO
		m_gfxFBOS->bindForReading("GEOMETRY", 0);					// Read from Geometry FBO
		glBindTextureUnit(4, m_shadowFBO.m_textureIDS[2]);			// Shadow map (depth texture)
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
		m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible shadow indices
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
		glBindVertexArray(m_shapeQuad->m_vaoID);					// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);						// Now draw

	}
	/** Render light bounces. */
	inline void renderBounce(const float & deltaTime) {
		m_shader_Bounce->setUniform(0, m_shadowCount);
		m_shader_Bounce->setUniform(1, m_volumeRH->m_max);
		m_shader_Bounce->setUniform(2, m_volumeRH->m_min);
		m_shader_Bounce->setUniform(4, m_volumeRH->m_resolution);
		m_shader_Bounce->setUniform(6, m_volumeRH->m_unitSize);

		// Prepare rendering state
		glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
		glBindVertexArray(m_shapeQuad->m_vaoID);

		glViewport(0, 0, (GLsizei)m_volumeRH->m_resolution, (GLsizei)m_volumeRH->m_resolution);
		m_shader_Bounce->bind();
		m_volumeRH->writePrimary();
		m_gfxFBOS->bindForReading("GEOMETRY", 0); // depth -> 3		
		glBindTextureUnit(0, m_shadowFBO.m_textureIDS[0]);
		glBindTextureUnit(1, m_shadowFBO.m_textureIDS[1]);
		glBindTextureUnit(2, m_shadowFBO.m_textureIDS[2]);
		glBindTextureUnit(4, m_textureNoise32);
		m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_visShadows.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);		// SSBO visible shadow indices
		m_indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glViewport(0, 0, (*m_cameraBuffer)->Dimensions.x, (*m_cameraBuffer)->Dimensions.y);

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shader_Lighting, m_shader_Shadow, m_shader_Culling, m_shader_Bounce;
	Shared_Primitive m_shapeQuad;
	GLuint m_textureNoise32 = 0;
	GLint m_shadowCount = 0;
	glm::ivec2 m_shadowSize = glm::ivec2(1024);
	GLuint m_bounceSize = 16u, m_updateQuality = 1u;
	StaticBuffer m_indirectShape = StaticBuffer(sizeof(GLuint) * 4), m_indirectBounce = StaticBuffer(sizeof(GLuint) * 4);
	DynamicBuffer m_visLights, m_visShadows;
	Prop_Shadow * m_propShadowSystem = nullptr;
	std::vector<std::pair<LightDirectional_Component*, LightDirectionalShadow_Component*>> m_shadowsToUpdate;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	int m_notifyLight = -1, m_notifyShadow = -1;

	// Core Lighting Data
	/** OpenGL buffer for directional lights. */
	struct Directional_Buffer {
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightDirection; float padding2;
		float LightIntensity; glm::vec3 padding3;
	};
	/** OpenGL buffer for directional light shadows. */
	struct Directional_Shadow_Buffer {
		glm::mat4 lightV = glm::mat4(1.0f);
		glm::mat4 lightVP[NUM_CASCADES];
		glm::mat4 inverseVP[NUM_CASCADES];
		float CascadeEndClipSpace[NUM_CASCADES];
		int Shadow_Spot = 0; glm::vec3 padding1;
	};
	GL_ArrayBuffer<Directional_Buffer> m_lightBuffer;
	GL_ArrayBuffer<Directional_Shadow_Buffer> m_shadowBuffer;
	FBO_Shadow_Directional m_shadowFBO;
};

#endif // DIRECTIONAL_LIGHTING_H