#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflections.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Reflection_FBO.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Utilities\EnginePackage.h"
#include "glm\gtc\matrix_transform.hpp"
#include <minmax.h>


Reflections::~Reflections()
{
	glDeleteTextures(1, &m_texture);
	glDeleteFramebuffers(1, &m_fbo);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Reflections::Reflections(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Reflection_FBO * reflectionFBO)
{
	m_enginePackage = enginePackage;
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;
	m_reflectionFBO = reflectionFBO;
	m_reflectionUBO = &m_enginePackage->getSubSystem<System_Graphics>("Graphics")->m_reflectionUBO;
	m_fbo = 0;
	m_texture = 0;
	m_cube_fbo = 0;
	m_cube_tex = 0;

	Asset_Loader::load_asset(m_shaderCopy, "fx\\copyTexture");
	Asset_Loader::load_asset(m_shaderBlur, "fx\\gaussianBlur_MIP");
	Asset_Loader::load_asset(m_shaderSSR, "Lighting\\Indirect Lighting\\Reflections (specular)\\SSR");
	Asset_Loader::load_asset(m_shaderFinal, "Lighting\\Indirect Lighting\\Reflections (specular)\\reflections PBR");
	Asset_Loader::load_asset(m_shaderCubemap, "Lighting\\Indirect Lighting\\Reflections (specular)\\cubemap IBL");
	Asset_Loader::load_asset(m_shaderCubeProj, "Lighting\\Indirect Lighting\\Reflections (specular)\\screen to cubemap");
	Asset_Loader::load_asset(m_shaderParallax, "Lighting\\Indirect Lighting\\Reflections (specular)\\parallax IBL");
	Asset_Loader::load_asset(m_brdfMap, "brdfLUT.png");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	Asset_Loader::load_asset(m_shapeCube, "box");
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
	m_quadVAOLoaded = false;
	m_cubeVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_cubeVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_quadVAOLoaded = true; });
	m_shapeCube->addCallback(this, [&]() { m_shapeCube->updateVAO(m_cubeVAO); m_cubeVAOLoaded = true; });
	m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });

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
	}

	GLuint cubeData[4] = { 36, 0, 0, 0 }; // count, primCount, first, reserved
	m_cubeIndirectBuffer = MappedBuffer( sizeof(GLuint) * 4, cubeData);
	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = MappedBuffer(sizeof(GLuint) * 4, quadData);
	SSR_Buffer buffer;
	m_ssrBuffer = MappedBuffer(sizeof(SSR_Buffer), &buffer);
	m_visRefUBO = MappedBuffer(sizeof(GLuint) * 500, 0);
}

void Reflections::resize(const vec2 & size)
{
	m_renderSize = size;
	for (int x = 0; x < 6; ++x) {
		const ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
		glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, x, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
}

void Reflections::updateData(const Visibility_Token & vis_token)
{
	const size_t r_size = vis_token.specificSize("Reflector");
	if (r_size) {
		vector<GLuint> refArray(vis_token.specificSize("Reflector"));
		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Reflector_Component>("Reflector"))
			refArray[count++] = component->getBufferIndex();
		m_visRefUBO.write(0, sizeof(GLuint)*refArray.size(), refArray.data());
	}
}

void Reflections::applyLighting(const Visibility_Token & vis_token)
{
	if (m_quadVAOLoaded && m_cubeVAOLoaded) {
		blurLight();
		buildEnvMap();
		reflectLight(vis_token);
	}
}

void Reflections::blurLight()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Copy lighting texture to one with a MIP chain
	m_shaderCopy->bind();
	m_lightingFBO->bindForReading();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(m_quadVAO);
	m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

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
			glDrawArraysIndirect(GL_TRIANGLES, 0);
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
}

void Reflections::buildEnvMap()
{
	// Copy viewport to cubemap
	const int quad_size = m_shapeQuad->getSize();
	m_shaderCubeProj->bind();
	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_cube_fbo);
	glDisable(GL_BLEND);
	glBindTextureUnit(0, m_texture);

	glBindVertexArray(m_quadVAO);
	m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	glEnable(GL_BLEND);
}

void Reflections::reflectLight(const Visibility_Token & vis_token)
{
	// Bind starting state
	m_reflectionFBO->bindForWriting();
	m_geometryFBO->bindForReading();
	glBindTextureUnit(3, m_cube_tex); // Persistent cubemap
	m_brdfMap->bind(4); // BRDF LUT
	glBindTextureUnit(5, m_texture); // Blurred light MIP-chain
	m_reflectionUBO->bindBuffer();
	m_ssrBuffer.bindBufferBase(GL_UNIFORM_BUFFER, 6);
	const size_t primCount = vis_token.specificSize("Reflector");
	m_cubeIndirectBuffer.write(sizeof(GLuint), sizeof(GLuint), &primCount);

	// Apply persistent cubemap
	m_shaderCubemap->bind();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glBindVertexArray(m_quadVAO);
	m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// Stencil out parallax reflectors
	m_shaderParallax->bind();
	m_shaderParallax->Set_Uniform(0, true);
	m_visRefUBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);	
	glClear(GL_STENCIL_BUFFER_BIT);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glStencilFunc(GL_ALWAYS, 0, 0); // Always pass stencil test
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glBindVertexArray(m_cubeVAO);
	m_cubeIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);
	
	// Fill in stenciled region
	m_shaderParallax->Set_Uniform(0, false);
	glDisable(GL_DEPTH_TEST);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // Pass test if stencil value IS NOT 0
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// Apply SSR
	m_shaderSSR->bind();
	glEnable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	glBindVertexArray(m_quadVAO);
	m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// Read reflection colors and correct them based on surface properties
	m_shaderFinal->bind();
	m_lightingFBO->bindForWriting(); // Write back to lighting buffer
	m_reflectionFBO->bindForReading(3); // Read from final reflections
	glBlendFunc(GL_ONE, GL_ONE);
	m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// Revert State
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}
