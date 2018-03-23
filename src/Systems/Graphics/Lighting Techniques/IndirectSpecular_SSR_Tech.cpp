#include "Systems\Graphics\Lighting Techniques\IndirectSpecular_SSR_Tech.h"
#include "Systems\GraphiCS\Frame Buffers\Geometry_Buffer.h"
#include "Systems\GraphiCS\Frame Buffers\Lighting_Buffer.h"
#include "Systems\GraphiCS\Frame Buffers\Reflection_Buffer.h"
#include "Systems\Graphics\VisualFX.h"
#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Managers\Message_Manager.h"
#include "Utilities\EnginePackage.h"
#include "glm\gtc\matrix_transform.hpp"
#include <minmax.h>


IndirectSpecular_SSR_Tech::~IndirectSpecular_SSR_Tech()
{
	glDeleteBuffers(1, &m_ssrUBO);
	glDeleteTextures(1, &m_texture);
	glDeleteFramebuffers(1, &m_fbo);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

IndirectSpecular_SSR_Tech::IndirectSpecular_SSR_Tech(EnginePackage * enginePackage, Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, Reflection_Buffer * refBuffer, VisualFX * visualFX)
{
	m_enginePackage = enginePackage;
	m_gBuffer = gBuffer;
	m_lBuffer = lBuffer;
	m_refBuffer = refBuffer;
	m_visualFX = visualFX;
	m_fbo = 0;
	m_texture = 0;
	m_ssrUBO = 0;

	Asset_Loader::load_asset(m_shaderCopy, "fx\\copyTexture");
	Asset_Loader::load_asset(m_shaderBlur, "fx\\gaussianBlur_MIP");
	Asset_Loader::load_asset(m_shaderSSR, "Lighting\\ssr");
	Asset_Loader::load_asset(m_brdfMap, "brdfLUT.png");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	Asset_Loader::load_asset(m_shaderCubemap, "Reflection\\reflectionCubemap");
	Asset_Loader::load_asset(m_shaderCubeProj, "Reflection\\cubeProjection");
	Asset_Loader::load_asset(TEST_SHADER, "test");
	Asset_Loader::load_asset(m_shapeCube, "box");
	m_cubeVAO = Asset_Primitive::Generate_VAO();
	m_shapeCube->addCallback(this, [&]() { m_shapeCube->updateVAO(m_cubeVAO); });
	m_shaderCubeProj->addCallback(this, [&]() {
		mat4 views[6];
		views[0] = (glm::rotate(mat4(1.0f), glm::radians(90.0f), vec3(0, 1, 0)));
		views[1] = (glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(0, 1, 0)));
		views[2] = (glm::rotate(mat4(1.0f), glm::radians(90.0f), vec3(1, 0, 0)));
		views[3] = (glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(1, 0, 0)));
		views[4] = (glm::rotate(mat4(1.0f), glm::radians(0.0f), vec3(0, 1, 0)));
		views[5] = (glm::rotate(mat4(1.0f), glm::radians(180.0f), vec3(0, 1, 0)));
		mat4 proj = (perspective(2.0f * atanf(tanf(radians(90.0f) / 2.0f)), 1.0f, 0.01f, m_enginePackage->getPreference(PreferenceState::C_DRAW_DISTANCE)));

		m_shaderCubeProj->bind();
		m_shaderCubeProj->Set_Uniform_Array(2, views, 6);
		m_shaderCubeProj->Set_Uniform(8, proj);
		Asset_Shader::Release();
	});

	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });
	m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });
	
	m_cube_fbo = 0;
	m_cube_tex = 0;
	glCreateFramebuffers(1, &m_cube_fbo);
	glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_cube_tex);
	glTextureStorage3D(m_cube_tex, 1, GL_RGB32F, 512, 512, 6 * 6 /*6 sides and 6 cubemaps*/);
	glTextureParameteri(m_cube_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_cube_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_cube_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_cube_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_cube_tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_cube_fbo, GL_COLOR_ATTACHMENT0, m_cube_tex, 0);
	glNamedFramebufferDrawBuffer(m_cube_fbo, GL_COLOR_ATTACHMENT0);
		
	glCreateFramebuffers(1, &m_fbo);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_LOD, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LOD, 5);
	glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, 5);
	for (int x = 0; x < 6; ++x) {
		const ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
		glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, x, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
	GLenum Status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
		std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
		MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Lighting Buffer", errorString);

		// Delete before returning
		glDeleteTextures(1, &m_texture);
		glDeleteFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}
	
	glCreateBuffers(1, &m_ssrUBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_ssrUBO);
	glNamedBufferStorage(m_ssrUBO, sizeof(SSR_Buffer), &m_ssrBuffer, GL_CLIENT_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	GLuint drawData[4] = { 36, 0, 0, 0 }; // count, primCount, first, reserved
	m_buffer = GL_MappedBuffer(sizeof(GLuint) * 4, &drawData);
}

void IndirectSpecular_SSR_Tech::resize(const vec2 & size)
{
	m_renderSize = size;
	for (int x = 0; x < 6; ++x) {
		const ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
		glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, x, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
}

void IndirectSpecular_SSR_Tech::updateLighting(const Visibility_Token & vis_token)
{
	
}

void IndirectSpecular_SSR_Tech::applyLighting(const Visibility_Token & vis_token)
{
	blurLight();	 
	buildEnvMap();
	reflectLight(vis_token);
}

void IndirectSpecular_SSR_Tech::blurLight()
{
	const int quad_size = m_shapeQuad->getSize();
	glBindVertexArray(m_quadVAO);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Copy lighting texture to one with a MIP chain
	m_shaderCopy->bind();
	m_lBuffer->bindForReading();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);

	// Blur MIP chain, reading from 1 MIP level and writing into next
	m_shaderBlur->bind();
	glBindTextureUnit(0, m_texture);
	for (int horizontal = 0; horizontal < 2; ++horizontal) {
		Asset_Shader::Set_Uniform(0, horizontal);
		ivec2 read_size = m_renderSize;
		for (int x = 1; x < 6; ++x) {
			// Ensure we are reading from MIP level x - 1
			Asset_Shader::Set_Uniform(1, read_size);
			glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, x - 1);
			glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, x - 1);
			// Ensure we are writing to MIP level x
			ivec2 write_size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
			glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, x);

			glViewport(0, 0, max(1.0f, write_size.x), max(1.0f, write_size.y));
			glDrawArrays(GL_TRIANGLES, 0, quad_size);
			read_size = write_size;
		}
		// Blend second pass
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	// Restore to default
	glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, 5);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	Asset_Shader::Release();
	// Maintain state for next function call: reflectLight()
}

void IndirectSpecular_SSR_Tech::buildEnvMap() 
{
	// Copy viewport to cubemap
	const int quad_size = m_shapeQuad->getSize();
	m_shaderCubeProj->bind();
	glViewport(0, 0, 512, 512);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_cube_fbo);
	glBindVertexArray(m_quadVAO);
	glDisable(GL_BLEND);
	glBindTextureUnit(0, m_texture);

	glDrawArrays(GL_TRIANGLES, 0, quad_size);

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	glEnable(GL_BLEND);
	Asset_Shader::Release();
}

void IndirectSpecular_SSR_Tech::reflectLight(const Visibility_Token & vis_token)
{
	// Use Fallback Reflections
	const int quad_size = m_shapeQuad->getSize();
	m_refBuffer->bindForWriting();
	m_gBuffer->bindForReading();
	m_shaderCubemap->bind();
	glBindTextureUnit(3, m_cube_tex);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);


	// Reflector test
	if (m_shapeCube->existsYet() && TEST_SHADER->existsYet()) {
		TEST_SHADER->bind();
		glBindVertexArray(m_cubeVAO);		
		const size_t primCount = vis_token.specificSize("Reflector");
		m_buffer.write(sizeof(GLuint), sizeof(GLuint), &primCount);
		m_buffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, (GLvoid*)0);
	}



	// Apply SSR
	m_lBuffer->bindForWriting();
	glBindVertexArray(m_quadVAO);
	m_shaderSSR->bind();
	m_gBuffer->bindForReading(); // Gbuffer
	m_refBuffer->bindForReading(3); // Fallback Reflections
	m_brdfMap->bind(4); // BRDF LUT
	glBindTextureUnit(5, m_texture); // blurred light MIP-chain
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE); 
	glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_ssrUBO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);

	// Revert State
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
	glCullFace(GL_BACK);
	Asset_Shader::Release();
}
