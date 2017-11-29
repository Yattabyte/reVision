#include "Systems\Graphics\Graphics_PBR.h"
#include "Rendering\Camera.h"
#include "Systems\Config_Manager.h"
#include "Systems\Shadowmap_Manager.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Entities\Components\Geometry_Component.h"
#include "Entities\Components\Lighting_Component.h"

static Shared_Asset_Shader geometry_shader, geometry_shadow_shader, lighting_shader;
static Shared_Asset_Primitive shape_quad;

System_Graphics_PBR::~System_Graphics_PBR()
{
}

System_Graphics_PBR::System_Graphics_PBR(Camera *engineCamera) : 
	m_engineCamera(engineCamera), m_gbuffer(), m_lbuffer(m_gbuffer.m_depth_stencil)
{
	screen_width = CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH);
	screen_height = CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);
	Asset_Manager::load_asset(geometry_shader, "Geometry\\geometry");
	Asset_Manager::load_asset(geometry_shadow_shader, "Geometry\\geometry_shadow");
	Asset_Manager::load_asset(lighting_shader, "Lighting\\lighting");
	Asset_Manager::load_asset(shape_quad, "quad");
}

void System_Graphics_PBR::Update(const float & deltaTime)
{
	glViewport(0, 0, screen_width, screen_height);

	shared_lock<shared_mutex> read_guard(m_engineCamera->getDataMutex());
	Visibility_Token &vis_token = m_engineCamera->GetVisibilityToken();
	
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

	/*geometry_shadow_shader->Bind();
	if (vis_token.visible_lights.size())
	for each (const auto *light in vis_token.visible_lights.at(Light_Directional::GetLightType()))
	light->shadowPass(vis_token);
	geometry_shadow_shader->Release();*/

	//Shadowmap_Manager::BindForWriting(SHADOW_REGULAR);

	glViewport(0, 0, screen_width, screen_height);

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
		geometry_shader->Bind();

		for each (auto component in *((vector<Geometry_Component*>*)(&vis_token.at("Anim_Model"))))
			component->Draw();

		geometry_shader->Release();
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
		lighting_shader->Bind();
		shape_quad->Bind();
		const int quad_size = shape_quad->GetSize();
		for each (auto component in *((vector<Lighting_Component*>*)(&vis_token.at("Light_Directional"))))
			component->directPass(quad_size);
		shape_quad->Unbind();
		lighting_shader->Release();
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

	glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
