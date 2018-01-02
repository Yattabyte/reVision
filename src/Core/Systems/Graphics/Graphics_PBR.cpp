#include "Systems\Graphics\Graphics_PBR.h"
#include "Utilities\Engine_Package.h"
#include "Rendering\Camera.h"
#include "Entities\Components\Geometry_Component.h"
#include "Entities\Components\Lighting_Component.h"


class Primitive_Observer : Asset_Observer
{
public:
	Primitive_Observer(Shared_Asset_Primitive &asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao), m_asset(asset) {};
	virtual ~Primitive_Observer() { 
		m_asset->RemoveObserver(this); 
	};
	virtual void Notify_Finalized() {
		if (m_asset->ExistsYet()) // in case this gets used more than once by mistake
			m_asset->UpdateVAO(m_vao_id);
	}

	GLuint m_vao_id;
	Shared_Asset_Primitive m_asset;
};

System_Graphics_PBR::~System_Graphics_PBR()
{
	if (!m_Initialized)
		delete m_observer;
}


System_Graphics_PBR::System_Graphics_PBR() :
	m_visualFX(), m_gbuffer(), m_lbuffer(), m_hdrbuffer()
{
	m_quadVAO = 0;
}

void System_Graphics_PBR::Initialize(Engine_Package * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_visualFX.Initialize(m_enginePackage);
		m_gbuffer.Initialize(m_enginePackage);		
		m_lbuffer.Initialize(m_enginePackage, &m_visualFX, m_gbuffer.m_depth_stencil);
		m_hdrbuffer.Initialize(m_enginePackage);
		Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");
		Asset_Loader::load_asset(m_shaderGeometryShadow, "Geometry\\geometry_shadow");
		Asset_Loader::load_asset(m_shaderLighting, "Lighting\\lighting");
		Asset_Loader::load_asset(m_shaderHDR, "FX\\HDR"); 
		Asset_Loader::load_asset(m_shaderFXAA, "FX\\FXAA"); 
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		Asset_Loader::load_asset(m_shaderSky, "skybox");
		Asset_Loader::load_asset(m_textureSky, "sky\\");
		m_quadVAO = Asset_Primitive::GenerateVAO();
		m_observer = (void*)(new Primitive_Observer(m_shapeQuad, m_quadVAO));
		m_gbuffer.End();
		m_Initialized = true;
	}
}

void System_Graphics_PBR::Update(const float & deltaTime)
{
	glViewport(0, 0, m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT));

	shared_lock<shared_mutex> read_guard(m_enginePackage->m_Camera.getDataMutex());
	Visibility_Token &vis_token = m_enginePackage->m_Camera.GetVisibilityToken();
	
	if (vis_token.size() && 
		m_shapeQuad->ExistsYet() &&
		m_textureSky->ExistsYet() && 
		m_shaderSky->ExistsYet() &&
		m_shaderGeometry->ExistsYet() &&
		m_shaderLighting->ExistsYet()) 
	{
		RegenerationPass(vis_token);
		GeometryPass(vis_token);
		SkyPass();
		LightingPass(vis_token);
		HDRPass();
		FinalPass();
	}
}

void System_Graphics_PBR::RegenerationPass(const Visibility_Token & vis_token)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_BACK);

	//Shadowmap_Manager::BindForWriting(SHADOW_LARGE);

	/*m_shaderGeometryShadow->Bind();
	if (vis_token.visible_lights.size())
	for each (const auto *light in vis_token.visible_lights.at(Light_Directional::GetLightType()))
	light->shadowPass(vis_token);
	m_shaderGeometryShadow->Release();*/

	//Shadowmap_Manager::BindForWriting(SHADOW_REGULAR);

	glViewport(0, 0, m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT));

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

		for each (auto &component in *((vector<Geometry_Component*>*)(&vis_token.at("Anim_Model"))))
			component->Draw();

		m_shaderGeometry->Release();
	}
}

void System_Graphics_PBR::SkyPass()
{
	const size_t &quad_size = m_shapeQuad->GetSize();
	m_lbuffer.Clear();
	m_lbuffer.BindForWriting();

	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);

	m_textureSky->Bind(GL_TEXTURE0);
	m_shaderSky->Bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}

void System_Graphics_PBR::LightingPass(const Visibility_Token & vis_token)
{
	const size_t &quad_size = m_shapeQuad->GetSize();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_gbuffer.BindForReading();

	if (vis_token.find("Light_Directional") != vis_token.end()) {
		m_shaderLighting->Bind();
		glBindVertexArray(m_quadVAO);
		for each (auto &component in *((vector<Lighting_Component*>*)(&vis_token.at("Light_Directional"))))
			component->directPass(quad_size);
	}

	glBindVertexArray(0);
	Asset_Shader::Release();
	m_lbuffer.ApplyBloom();
}

void System_Graphics_PBR::HDRPass()
{
	const size_t &quad_size = m_shapeQuad->GetSize();
	m_lbuffer.BindForReading();
	m_hdrbuffer.Clear();
	m_hdrbuffer.BindForWriting();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	//glDisable(GL_CULL_FACE);

	m_shaderHDR->Bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();
}

void System_Graphics_PBR::FinalPass()
{
	const size_t &quad_size = m_shapeQuad->GetSize();
	m_hdrbuffer.BindForReading();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	m_shaderFXAA->Bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();
	m_gbuffer.End();	
}