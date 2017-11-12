#include "Rendering\Scenes\PBR\Scene_PBR.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Managers\Config_Manager.h"
#include "Managers\Geometry_Manager.h"
#include "Managers\Lighting_Manager.h"

static Shared_Asset_Shader geometry_shader, lighting_shader;
static Shared_Asset_Primitive shape_quad;
static float screen_width = 1.0f, screen_height = 1.0f;

static void WidthChangeCallback(const float &width) {
	screen_width = width;
}
static void HeightChangeCallback(const float &height) {
	screen_height = height;
}

// Shutdown
Scene_PBR::~Scene_PBR()
{
	CFG::removePreferenceCallback(CFG_ENUM::C_WINDOW_WIDTH, WidthChangeCallback);
	CFG::removePreferenceCallback(CFG_ENUM::C_WINDOW_HEIGHT, HeightChangeCallback);
}

// Startup
Scene_PBR::Scene_PBR() : m_gbuffer(), m_lbuffer(m_gbuffer.m_depth_stencil)
{
	CFG::addPreferenceCallback(CFG_ENUM::C_WINDOW_WIDTH, WidthChangeCallback);
	CFG::addPreferenceCallback(CFG_ENUM::C_WINDOW_HEIGHT, HeightChangeCallback);
	screen_width = CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH);
	screen_height = CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);
	Asset_Manager::load_asset(geometry_shader, "Geometry\\geometry");
	Asset_Manager::load_asset(lighting_shader, "Lighting\\lighting");
	Asset_Manager::load_asset(shape_quad, "quad");
}

void Scene_PBR::RenderFrame()
{
	glViewport(0, 0, screen_width, screen_height);

	GeometryPass();
	LightingPass();
	FinalPass();
}

void Scene_PBR::GeometryPass()
{
	auto &geometryMutex = Geometry_Manager::GetDataLock();
	const auto &geometryMap = Geometry_Manager::GetAllGeometry();

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	m_gbuffer.Clear();
	m_gbuffer.BindForWriting();
	geometry_shader->Bind();
	shared_lock<shared_mutex> readLock(geometryMutex);
	for each (auto vec in geometryMap)
		for each (auto *obj in vec.second)
			obj->geometryPass();
	readLock.release();
	geometry_shader->Release();
}

void Scene_PBR::LightingPass()
{
	auto &lightingMutex = Lighting_Manager::GetDataLock();
	const auto &lightingMap = Lighting_Manager::GetAllLights();

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
	shared_lock<shared_mutex> readLock(lightingMutex);
	for each (auto vec in lightingMap)
		for each (auto *obj in vec.second)
			obj->directPass(quad_size);
	readLock.release();
	shape_quad->Unbind();
	lighting_shader->Release();
}

void Scene_PBR::FinalPass()
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
