#include "Systems\Graphics\Graphics_PBR.h"
#include "Utilities\Engine_Package.h"
#include "Rendering\Camera.h"
#include "Systems\Config_Manager.h"
#include "Systems\Shadowmap_Manager.h"
#include "Entities\Components\Geometry_Component.h"
#include "Entities\Components\Lighting_Component.h"



System_Graphics_PBR::~System_Graphics_PBR()
{
}

System_Graphics_PBR::System_Graphics_PBR(Engine_Package *package) :
	m_enginePackage(package), m_gbuffer(), m_lbuffer(m_gbuffer.m_depth_stencil)
{
	Asset_Manager::load_asset(m_shaderGeometry, "Geometry\\geometry");
	Asset_Manager::load_asset(m_shaderGeometryShadow, "Geometry\\geometry_shadow");
	Asset_Manager::load_asset(m_shaderLighting, "Lighting\\lighting");
	Asset_Manager::load_asset(m_shapeQuad, "quad");
}

void System_Graphics_PBR::Update(const float & deltaTime)
{
	glViewport(0, 0, m_enginePackage->window_width, m_enginePackage->window_height);

	shared_lock<shared_mutex> read_guard(m_enginePackage->m_Camera->getDataMutex());
	Visibility_Token &vis_token = m_enginePackage->m_Camera->GetVisibilityToken();
	
	if (vis_token.size()) {
		RegenerationPass(vis_token);
		GeometryPass(vis_token);
		LightingPass(vis_token);
		FinalPass(vis_token);
	}
}

void System_Graphics_PBR::RegenerationPass(const Visibility_Token & vis_token)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_BACK);

	Shadowmap_Manager::BindForWriting(SHADOW_LARGE);

	/*m_shaderGeometryShadow->Bind();
	if (vis_token.visible_lights.size())
	for each (const auto *light in vis_token.visible_lights.at(Light_Directional::GetLightType()))
	light->shadowPass(vis_token);
	m_shaderGeometryShadow->Release();*/

	//Shadowmap_Manager::BindForWriting(SHADOW_REGULAR);

	glViewport(0, 0, m_enginePackage->window_width, m_enginePackage->window_height);

	glDepthFunc(GL_LESS);
}

void System_Graphics_PBR::GeometryPass(const Visibility_Token & vis_token)
{
	if (vis_token.find("Anim_Model") != vis_token.end()) {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		m_gbuffer.Clear();
		m_gbuffer.BindForWriting();
		m_shaderGeometry->Bind();

		for each (auto component in *((vector<Geometry_Component*>*)(&vis_token.at("Anim_Model"))))
			component->Draw();

		m_shaderGeometry->Release();
	}
}

void System_Graphics_PBR::LightingPass(const Visibility_Token & vis_token)
{
	if (vis_token.find("Light_Directional") != vis_token.end()) {
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		/*glDisable(GL_STENCIL_TEST);*/
		m_lbuffer.Clear();
		
		m_gbuffer.BindForReading();
		m_lbuffer.BindForWriting();
		m_shaderLighting->Bind();
		m_shapeQuad->Bind();
		const int quad_size = m_shapeQuad->GetSize();
		for each (auto component in *((vector<Lighting_Component*>*)(&vis_token.at("Light_Directional"))))
			component->directPass(quad_size);
		m_shapeQuad->Unbind();
		m_shaderLighting->Release();
	}
}

void System_Graphics_PBR::FinalPass(const Visibility_Token & vis_token)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	//glDisable(GL_CULL_FACE);

	m_lbuffer.BindForReading();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_lbuffer.m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBlitFramebuffer(0, 0, m_enginePackage->window_width, m_enginePackage->window_height, 0, 0, m_enginePackage->window_width, m_enginePackage->window_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}