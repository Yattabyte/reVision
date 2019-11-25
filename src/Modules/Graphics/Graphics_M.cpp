#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Engine.h"
#include <memory>
#include <random>
#include <typeinfo>


void Graphics_Module::initialize() noexcept
{
	Engine_Module::initialize();
	m_engine.getManager_Messages().statement("Loading Module: Graphics...");

	// Asset Loading
	m_shader = Shared_Shader(m_engine, "Effects\\Copy Texture");
	m_shapeQuad = Shared_Auto_Model(m_engine, "quad");

	// Asset-Finished Callbacks
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
		m_indirectQuad = IndirectDraw<1>((GLuint)m_shapeQuad->getSize(), 1, 0, GL_CLIENT_STORAGE_BIT);
		});

	// GL settings
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_DEPTH_CLAMP);

	// Preferences
	auto& preferences = m_engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) {
		m_renderSize = glm::ivec2(f, m_renderSize.y);
		m_viewport->resize(m_renderSize, 1);
		(*m_clientCamera)->Dimensions = m_renderSize;
		});
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		m_viewport->resize(m_renderSize, 1);
		(*m_clientCamera)->Dimensions = m_renderSize;
		});
	float farPlane = 1000.0F;
	preferences.getOrSetValue(PreferenceState::Preference::C_DRAW_DISTANCE, farPlane);
	preferences.addCallback(PreferenceState::Preference::C_DRAW_DISTANCE, m_aliveIndicator, [&](const float& f) {
		if ((*m_clientCamera)->FarPlane != f) {
			(*m_clientCamera)->FarPlane = f;
			genPerspectiveMatrix();
		}
		});
	float fov = 90.0F;
	preferences.getOrSetValue(PreferenceState::Preference::C_FOV, fov);
	preferences.addCallback(PreferenceState::Preference::C_FOV, m_aliveIndicator, [&](const float& f) {
		if ((*m_clientCamera)->FOV != f) {
			(*m_clientCamera)->FOV = f;
			genPerspectiveMatrix();
		}
		});

	// Camera Setup
	m_viewport = std::make_shared<Viewport>(glm::ivec2(0), m_renderSize, m_engine);
	m_clientCamera = std::make_shared<Camera>();
	m_clientCamera->setEnabled(true);
	m_clientCamera->get()->Dimensions = glm::vec2(m_renderSize);
	m_clientCamera->get()->FarPlane = farPlane;
	m_clientCamera->get()->FOV = fov;
	genPerspectiveMatrix();

	// Rendering Effects & systems
	m_pipeline = std::make_unique<Graphics_Pipeline>(m_engine, m_clientCamera);
}

void Graphics_Module::deinitialize() noexcept
{
	m_engine.getManager_Messages().statement("Unloading Module: Graphics...");

	// Update indicator
	*m_aliveIndicator = false;
	m_pipeline.reset();
	m_viewport.reset();
	m_clientCamera.reset();
}

void Graphics_Module::renderWorld(ecsWorld& world, const float& deltaTime, const GLuint& fboID) noexcept
{
	renderWorld(world, deltaTime, m_viewport, { m_clientCamera });
	copyToScreen(fboID);
}

void Graphics_Module::renderWorld(ecsWorld& world, const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::shared_ptr<Camera>>& cameras) noexcept
{
	if (!cameras.empty()) {
		// Prepare rendering pipeline for a new frame, wait for buffers to free
		const auto perspectives = m_pipeline->begin(deltaTime, world, cameras);
		viewport->bind();
		viewport->clear();
		viewport->m_gfxFBOS->m_rhVolume.updateVolume(cameras[0].get());

		m_pipeline->render(deltaTime, viewport, perspectives);

		m_pipeline->end(deltaTime);
	}
}

void Graphics_Module::genPerspectiveMatrix() noexcept
{
	// Update Perspective Matrix
	const float ar = std::max(1.0F, (*m_clientCamera)->Dimensions.x) / std::max(1.0F, (*m_clientCamera)->Dimensions.y);
	const float horizontalRad = glm::radians((*m_clientCamera)->FOV);
	const float verticalRad = 2.0F * atanf(tanf(horizontalRad / 2.0F) / ar);
	(*m_clientCamera)->pMatrix = glm::perspective(verticalRad, ar, Camera::ConstNearPlane, (*m_clientCamera)->FarPlane);
	(*m_clientCamera)->pMatrixInverse = glm::inverse((*m_clientCamera)->pMatrix);
	(*m_clientCamera)->pvMatrix = (*m_clientCamera)->pMatrix * (*m_clientCamera)->vMatrix;
}

void Graphics_Module::copyToScreen(const GLuint& fboID) noexcept
{
	if (m_shapeQuad->existsYet() && m_shader->existsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
		m_shader->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_indirectQuad.drawCall();
		Shader::Release();
	}
}