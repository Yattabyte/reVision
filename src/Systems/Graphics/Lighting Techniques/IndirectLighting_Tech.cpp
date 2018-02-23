#include "Systems\Graphics\Lighting Techniques\IndirectLighting_Tech.h"
#include "Systems\GraphiCS\Frame Buffers\Geometry_Buffer.h"
#include "Systems\GraphiCS\Frame Buffers\Lighting_Buffer.h"
#include "Systems\GraphiCS\Frame Buffers\Shadow_Buffer.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Utilities\EnginePackage.h"


struct Primitiveee_Observer : Asset_Observer {
	Primitiveee_Observer(Shared_Asset_Primitive & asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao) {};
	virtual void Notify_Finalized() {
		if (m_asset->existsYet())
			dynamic_pointer_cast<Asset_Primitive>(m_asset)->updateVAO(m_vao_id);
	}
	GLuint m_vao_id;
};

IndirectLighting_Tech::~IndirectLighting_Tech()
{
	delete m_QuadObserver;
}

IndirectLighting_Tech::IndirectLighting_Tech(EnginePackage * enginePackage, Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, Shadow_Buffer *sBuffer)
{
	m_gBuffer = gBuffer;
	m_lBuffer = lBuffer;
	m_sBuffer = sBuffer;
	m_enginePackage = enginePackage;

	Asset_Loader::load_asset(m_shaderDirectional_Bounce, "Lighting\\directional_bounce");
	Asset_Loader::load_asset(m_shaderPoint_Bounce, "Lighting\\point_bounce");
	Asset_Loader::load_asset(m_shaderSpot_Bounce, "Lighting\\spot_bounce");
	Asset_Loader::load_asset(m_shaderGISecondBounce, "Lighting\\gi_second_bounce");
	Asset_Loader::load_asset(m_shaderGIReconstruct, "Lighting\\gi_reconstruction");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_QuadObserver = (void*)(new Primitiveee_Observer(m_shapeQuad, m_quadVAO));
	m_giBuffer.initialize(16, m_enginePackage);

	GLuint VBO = 0;
	glGenVertexArrays(1, &m_bounceVAO);
	glBindVertexArray(m_bounceVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLint), GLint(0), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_INT, sizeof(GLint), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void IndirectLighting_Tech::updateLighting(const Visibility_Token & vis_token)
{
	// Prepare rendering state
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	m_giBuffer.updateData();
	m_giBuffer.bindNoise(GL_TEXTURE4);
	m_giBuffer.bindForWriting(0);

	// Perform primary light bounce
	glBindVertexArray(m_bounceVAO);
	// Bounce directional light
	m_sBuffer->BindForReading_GI(SHADOW_LARGE, GL_TEXTURE0);
	if (vis_token.find("Light_Directional")) {
		m_shaderDirectional_Bounce->bind();
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
			component->indirectPass(16);
	}
	// Bounce point lights
	m_sBuffer->BindForReading_GI(SHADOW_REGULAR, GL_TEXTURE0);
	if (vis_token.find("Light_Point")) {
		m_shaderPoint_Bounce->bind();
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
			component->indirectPass(16);
	}
	// Bounce spot lights
	if (vis_token.find("Light_Spot")) {
		m_shaderSpot_Bounce->bind();
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
			component->indirectPass(16);
	}

	// Perform secondary light bounce
	m_shaderGISecondBounce->bind();
	m_giBuffer.bindForReading(0, GL_TEXTURE5);
	m_giBuffer.bindForWriting(1);
	glDrawArraysInstanced(GL_POINTS, 0, 1, 32);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	Asset_Shader::Release();
}

void IndirectLighting_Tech::applyLighting(const Visibility_Token & vis_token)
{
	// Reconstruct GI from data
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_gBuffer->bindForReading();
	m_lBuffer->bindForWriting();

	m_shaderGIReconstruct->bind();
	const size_t &quad_size = m_shapeQuad->getSize();

	m_giBuffer.bindNoise(GL_TEXTURE4);
	glBindVertexArray(m_quadVAO);
	m_giBuffer.bindForReading(1, GL_TEXTURE5);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);


	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	Asset_Shader::Release();
}
