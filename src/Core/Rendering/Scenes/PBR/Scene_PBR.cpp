#include "Rendering\Scenes\PBR\Scene_PBR.h"
#include "Assets\Asset_Shader.h"
#include "Managers\Config_Manager.h"
#include "Managers\Geometry_Manager.h"

static Shared_Asset_Shader shader;
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
Scene_PBR::Scene_PBR()
{
	CFG::addPreferenceCallback(CFG_ENUM::C_WINDOW_WIDTH, WidthChangeCallback);
	CFG::addPreferenceCallback(CFG_ENUM::C_WINDOW_HEIGHT, HeightChangeCallback);
	screen_width = CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH);
	screen_height = CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);
	Asset_Manager::load_asset(shader, "testShader");
}

void Scene_PBR::RenderFrame()
{
	auto &geometryMutex = Geometry_Manager::GetDataLock();
	const auto &geometryMap = Geometry_Manager::GetAllGeometry();

	glViewport(0, 0, screen_width, screen_height);

	m_gbuffer.Clear();

	m_gbuffer.BindForWriting();
	shader->Bind();
	shared_lock<shared_mutex> readLock(geometryMutex);
	for each (auto vec in geometryMap)
		for each (auto *obj in vec.second)
			obj->geometryPass();
	readLock.release();
	shader->Release();

	m_gbuffer.BindForReading();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gbuffer.m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);	
}
