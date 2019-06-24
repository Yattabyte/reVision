#pragma once
#ifndef DIRECTIONAL_TECHNIQUE_H
#define DIRECTIONAL_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Directional/FBO_Shadow_Directional.h"
#include "Modules/Graphics/Geometry/Prop/Prop_Shadow.h"
#include "Modules/World/ECS/components.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "Engine.h"
#include <random>
#include <vector>

#define NUM_CASCADES 4


/** A core lighting technique responsible for all directional lights. */
class Directional_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Directional_Technique() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Directional_Technique(Engine * engine, Prop_Technique * propView)
		: m_engine(engine), Graphics_Technique(PRIMARY_LIGHTING) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightDirectional_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(CameraFollower_Component::ID, FLAG_OPTIONAL);

		// Asset Loading
		m_shader_Lighting = Shared_Shader(m_engine, "Core\\Directional\\Light");
		m_shader_Shadow = Shared_Shader(m_engine, "Core\\Directional\\Shadow");
		m_shader_Culling = Shared_Shader(m_engine, "Core\\Directional\\Culling");
		m_shader_Bounce = Shared_Shader(m_engine, "Core\\Directional\\Bounce");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_maxShadowsCasters);
		preferences.addCallback(PreferenceState::C_SHADOW_MAX_PER_FRAME, m_aliveIndicator, [&](const float &f) { m_maxShadowsCasters = (unsigned int)f; });
		preferences.getOrSetValue(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, m_shadowSize.x);
		preferences.addCallback(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, m_aliveIndicator, [&](const float &f) {
			m_shadowSize = glm::ivec2(std::max(1, (int)f));
			m_shadowFBO.resize(m_shadowSize, (m_shadowCount + m_unusedShadows.size()) * 4);
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
			m_engine->getManager_Messages().error("Directional_Technique Shadowmap Framebuffer has encountered an error.");

		GLuint indData[] = { 0,0,0,0 };
		m_indirectShape.write(0, sizeof(GLuint) * 4, &indData);

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addNotifyOnComponentType(LightDirectional_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			((LightDirectional_Component*)c)->m_lightIndex = m_lightBuffer.newElement();
		});

		// World-Changed Callback
		world.addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementations
	inline virtual void beginFrame(const float & deltaTime) override {
		m_lightBuffer.beginWriting();
		m_visLights.beginWriting();
	}
	inline virtual void endFrame(const float & deltaTime) override {
		m_lightBuffer.endWriting();
		m_visLights.endWriting();
	}
	/***/
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		std::vector<GLint> lightIndices;
		m_shadowsToUpdate.clear();
		m_visibleShadows = 0;
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightDirectional_Component * lightComponent = (LightDirectional_Component*)componentParam[1];	
			LightColor_Component * colorComponent = (LightColor_Component*)componentParam[2];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[3];
			CameraFollower_Component * camFollowComponent = (CameraFollower_Component*)componentParam[4];
			const auto & index = lightComponent->m_lightIndex;

			// If a directional light has a cam follower, it's using cascaded shadow maps, so ignore visibility
			lightIndices.push_back((GLuint)*index);

			// Sync Camera Attributes
			if (camFollowComponent) {
				const auto size = glm::vec2(camFollowComponent->m_viewport->m_dimensions);
				const float ar = size.x / size.y;
				const float tanHalfHFOV = glm::radians(camFollowComponent->m_viewport->getFOV()) / 2.0f;
				const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
				const float near_plane = -CameraBuffer::BufferStructure::ConstNearPlane;
				const float far_plane = -camFollowComponent->m_viewport->getDrawDistance();
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
				const glm::mat4 CamInv = glm::inverse(camFollowComponent->m_viewport->getViewMatrix());
				const glm::mat4 CamP = camFollowComponent->m_viewport->getPerspectiveMatrix();
				const glm::mat4 sunModelMatrix = glm::inverse(glm::mat4_cast(transformComponent->m_transform.m_orientation) * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
				if (lightComponent->m_hasShadow) {
					m_visibleShadows++;
					m_shadowsToUpdate.push_back(lightComponent);
					for (int i = 0; i < NUM_CASCADES; i++) {
						const glm::vec3 volumeUnitSize = (aabb[i] - -aabb[i]) / (float)m_shadowSize.x;
						const glm::vec3 frustumpos = glm::vec3(sunModelMatrix * CamInv * glm::vec4(middle[i], 1.0f));
						const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
						const glm::vec3 newMin = -aabb[i] + clampedPos;
						const glm::vec3 newMax = aabb[i] + clampedPos;
						const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
						const glm::mat4 pMatrix = glm::ortho(l, r, b, t, n, f);
						const glm::mat4 pvMatrix = pMatrix * sunModelMatrix;
						const glm::vec4 v1 = glm::vec4(0, 0, cascadeEnd[i + 1], 1.0f);
						const glm::vec4 v2 = CamP * v1;
						m_lightBuffer[index].lightVP[i] = pvMatrix;
						m_lightBuffer[index].inverseVP[i] = inverse(pvMatrix);
						m_lightBuffer[index].CascadeEndClipSpace[i] = v2.z;
					}
				}
			}
			else
				lightComponent->m_shadowSpot = -1;

			// Sync Color Attributes
			if (colorComponent) {
				m_lightBuffer[index].LightColor = colorComponent->m_color;
				m_lightBuffer[index].LightIntensity = colorComponent->m_intensity;
			}

			// Sync Transform Attributes
			if (transformComponent) {
				const auto & orientation = transformComponent->m_transform.m_orientation;
				const auto matRot = glm::mat4_cast(orientation);
				const glm::mat4 sunTransform = matRot;
				m_lightBuffer[index].LightDirection = glm::vec3(glm::normalize(sunTransform * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));

				if (lightComponent->m_hasShadow)
					m_lightBuffer[index].lightV = glm::inverse(sunTransform * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
			}

			// Shadowmap logic
			if (lightComponent->m_hasShadow && lightComponent->m_shadowSpot == -1) {
				// Assign shadowmap spot
				int shadowSpot;
				if (m_unusedShadows.size()) {
					// Reclaim a disused shadow map index
					shadowSpot = m_unusedShadows.back();
					m_unusedShadows.pop_back();
				}
				else
					shadowSpot = (int)(m_shadowCount) * 4;
				lightComponent->m_shadowSpot = shadowSpot;
				m_shadowCount++;
				m_shadowFBO.resize(m_shadowSize, (m_shadowCount + m_unusedShadows.size()) * 4);
			}
			else if (!lightComponent->m_hasShadow && lightComponent->m_shadowSpot >= 0) {
				// Save the old shadow index
				m_unusedShadows.push_back(lightComponent->m_shadowSpot);
				lightComponent->m_shadowSpot = -1;
				m_shadowCount--;
			}

			// Sync Buffer Attributes
			m_lightBuffer[index].Shadow_Spot = lightComponent->m_shadowSpot;
		}

		// Update Draw Buffers
		m_visLightCount = lightIndices.size();
		const GLuint bounceInstanceCount = GLuint(m_visibleShadows * m_bounceSize);
		m_visLights.write(0, sizeof(GLuint) * m_visLightCount, lightIndices.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_visLightCount); // update primCount (2nd param)
		m_indirectBounce.write(sizeof(GLuint), sizeof(GLuint), &bounceInstanceCount);
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) override {
		// Exit Early
		if (!m_enabled || !m_shapeQuad->existsYet() || !m_shader_Lighting->existsYet() || !m_shader_Shadow->existsYet() || !m_shader_Culling->existsYet() || !m_shader_Bounce->existsYet())
			return;

		// Render important shadows
		renderShadows(deltaTime, viewport);

		// Render lights
		renderLights(deltaTime, viewport);

		// Render indirect lights
		renderBounce(deltaTime, viewport);
	}


private:
	// Private Methods
	/** Render all the geometry from each light.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderShadows(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		if (m_visLightCount) {
			glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
			m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_shader_Shadow->bind();
			m_shadowFBO.bindForWriting();
			for (auto * light : m_shadowsToUpdate) {
				const float clearDepth(1.0f);
				const glm::vec3 clear(0.0f);
				m_shadowFBO.clear(light->m_shadowSpot);
				// Render geometry components
				m_propShadowSystem->setData(viewport->get3DPosition(), (int)*light->m_lightIndex);
				m_propShadowSystem->renderShadows(deltaTime);
				light->m_updateTime = m_engine->getTime();
			}
			m_shadowsToUpdate.clear();
			glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));
		}
	}
	/** Render all the lights.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderLights(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		if (m_visLightCount) {
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);

			m_shader_Lighting->bind();									// Shader (directional)
			viewport->m_gfxFBOS->bindForWriting("LIGHTING");			// Ensure writing to lighting FBO
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);			// Read from Geometry FBO
			glBindTextureUnit(4, m_shadowFBO.m_textureIDS[2]);			// Shadow map (depth texture)
			m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	// SSBO visible light indices
			m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
			glBindVertexArray(m_shapeQuad->m_vaoID);					// Quad VAO
			glDrawArraysIndirect(GL_TRIANGLES, 0);						// Now draw
		}

	}
	/** Render light bounces. 
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from.*/
	inline void renderBounce(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		if (m_visLightCount) {
			m_shader_Bounce->setUniform(0, m_visibleShadows);
			m_shader_Bounce->setUniform(1, viewport->m_rhVolume->m_max);
			m_shader_Bounce->setUniform(2, viewport->m_rhVolume->m_min);
			m_shader_Bounce->setUniform(4, viewport->m_rhVolume->m_resolution);
			m_shader_Bounce->setUniform(6, viewport->m_rhVolume->m_unitSize);

			// Prepare rendering state
			glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
			glBindVertexArray(m_shapeQuad->m_vaoID);

			glViewport(0, 0, (GLsizei)viewport->m_rhVolume->m_resolution, (GLsizei)viewport->m_rhVolume->m_resolution);
			m_shader_Bounce->bind();
			viewport->m_rhVolume->writePrimary();
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0); // depth -> 3		
			glBindTextureUnit(0, m_shadowFBO.m_textureIDS[0]);
			glBindTextureUnit(1, m_shadowFBO.m_textureIDS[1]);
			glBindTextureUnit(2, m_shadowFBO.m_textureIDS[2]);
			glBindTextureUnit(4, m_textureNoise32);
			m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
			m_lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			m_indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
			glViewport(0, 0, GLsizei(viewport->m_dimensions.x), GLsizei(viewport->m_dimensions.y));

			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ZERO);
			glDisable(GL_BLEND);
		}
	}
	/** Clear out the lights and shadows queued up for rendering. */
	inline void clear() {
		const size_t lightSize = 0;
		const GLuint bounceInstanceCount = GLuint(0);
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &lightSize); // update primCount (2nd param)
		m_indirectBounce.write(sizeof(GLuint), sizeof(GLuint), &bounceInstanceCount);
		m_visibleShadows = 0;
		m_shadowsToUpdate.clear();
		m_lightBuffer.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shader_Lighting, m_shader_Shadow, m_shader_Culling, m_shader_Bounce;
	Shared_Primitive m_shapeQuad;
	GLuint m_textureNoise32 = 0;
	glm::ivec2 m_shadowSize = glm::ivec2(1024);
	GLuint m_bounceSize = 16u, m_maxShadowsCasters = 1u;
	StaticBuffer m_indirectShape = StaticBuffer(sizeof(GLuint) * 4), m_indirectBounce = StaticBuffer(sizeof(GLuint) * 4);
	Prop_Shadow * m_propShadowSystem = nullptr;
	std::vector<LightDirectional_Component*> m_shadowsToUpdate;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);

	// Visibility
	size_t m_visLightCount = 0ull;
	GLint m_visibleShadows = 0;
	DynamicBuffer m_visLights;

	// Core Lighting Data
	/** OpenGL buffer for directional lights. */
	struct Directional_Buffer {
		glm::mat4 lightV = glm::mat4(1.0f);
		glm::mat4 lightVP[NUM_CASCADES];
		glm::mat4 inverseVP[NUM_CASCADES];
		float CascadeEndClipSpace[NUM_CASCADES];
		glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
		glm::vec3 LightDirection = glm::vec3(0,-1,0); float padding2;
		float LightIntensity = 1.0f;
		int Shadow_Spot = -1; glm::vec2 padding3;
	};
	size_t m_shadowCount = 0ull;
	std::vector<int> m_unusedShadows;
	GL_ArrayBuffer<Directional_Buffer> m_lightBuffer;
	FBO_Shadow_Directional m_shadowFBO;
};

#endif // DIRECTIONAL_TECHNIQUE_H