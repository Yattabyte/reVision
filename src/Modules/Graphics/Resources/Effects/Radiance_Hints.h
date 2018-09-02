#pragma once
#ifndef RADIANCE_HINTS_H
#define RADIANCE_HINTS_H

#include "Modules\Graphics\Resources\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Texture.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"


/** A post-processing technique for approximating indirect diffuse lighting (irradiant light, global illumination, etc) */
class Radiance_Hints : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Radiance_Hints() {
		glDeleteFramebuffers(1, &m_fboID);
		glDeleteTextures(1, &m_textureID);
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
		m_engine->removePrefCallback(PreferenceState::C_DRAW_DISTANCE, this);
		m_engine->removePrefCallback(PreferenceState::C_RH_BOUNCE_SIZE, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	}
	/** Constructor. */
	Radiance_Hints(Engine * engine, FBO_Base * geometryFBO, FBO_Base * bounceFBO)
	: m_engine(engine), m_geometryFBO(geometryFBO), m_bounceFBO(bounceFBO) {
		// Asset Loading
		m_shaderRecon = Asset_Shader::Create(m_engine, "Effects\\RH Reconstruction");
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Preference Callbacks
		m_drawDistance = m_engine->addPrefCallback<float>(PreferenceState::C_DRAW_DISTANCE, this, [&](const float &f) { m_drawDistance = f; });
		m_bounceSize = m_engine->addPrefCallback<GLuint>(PreferenceState::C_RH_BOUNCE_SIZE, this, [&](const float &f) { 
			m_bounceSize = (GLuint)f;
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), m_bounceSize, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
		});
		m_renderSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(glm::vec2(f, m_renderSize.y)); });
		m_renderSize.y = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(glm::vec2(m_renderSize.x, f)); });

		// Asset-Finished callbacks
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4);
		m_shapeQuad->addCallback(this, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), m_bounceSize, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
		});

		// GL Loading
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
		glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);

		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Radiance Hints Framebuffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		if (!glIsTexture(m_textureID))
			m_engine->reportError(MessageManager::TEXTURE_INCOMPLETE, "Radiance Hints Texture");

	}


	// Interface Implementations.
	virtual void applyEffect(const float & deltaTime) override {
		if (!m_shapeQuad->existsYet() || !m_shaderRecon->existsYet())
			return;

		auto & graphics = m_engine->getGraphicsModule();
		const auto cameraBuffer = graphics.m_cameraBuffer.getElement(graphics.getActiveCamera());
		const glm::mat4 CamInv = glm::inverse(cameraBuffer->data->vMatrix);
		const glm::vec2 &size = cameraBuffer->data->Dimensions;
		const float ar = size.x / size.y;
		const float tanHalfHFOV = glm::radians(cameraBuffer->data->FOV) / 2.0f;
		const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / ar);
		const float near_plane = -0.1f;
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
			// near face65
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
		glm::vec3 aabb(glm::length(frustumCorners[7] - middle));	

		const glm::vec3 volumeUnitSize = (aabb - -aabb) / float(m_bounceSize);
		const glm::vec3 frustumpos = (CamInv * glm::vec4(middle, 1.0f));
		const glm::vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
		const glm::vec3 newMin = -aabb + clampedPos;
		const glm::vec3 newMax = aabb + clampedPos;
		m_shaderRecon->setUniform(1, newMax);
		m_shaderRecon->setUniform(2, newMin);
		m_shaderRecon->setUniform(3, (float)m_bounceSize);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		m_shaderRecon->bind();
		m_geometryFBO->bindForReading(0);
		m_bounceFBO->bindForReading(4);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);		
		glDisable(GL_BLEND);

		// Bind for reading by next effect	
		glBindTextureUnit(4, m_textureID);
	}


private:
	// Private Methods
	/** Resize the frame buffer.
	@param	size	the new size of the frame buffer */
	void resize(const glm::vec2 & size) {
		m_renderSize = size;
		glTextureImage2DEXT(m_textureID, GL_TEXTURE_2D, 0, GL_RGB16F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_textureID, 0);
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	FBO_Base * m_geometryFBO = nullptr, * m_bounceFBO = nullptr;
	Shared_Asset_Shader m_shaderRecon;
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	float m_drawDistance = 100.0f;
	GLuint m_bounceSize = 16u;
	GLuint m_fboID = 0, m_textureID = 0;
};

#endif // RADIANCE_HINTS_H