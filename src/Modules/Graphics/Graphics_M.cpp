#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Engine.h"
#include <memory>
#include <random>
#include <typeinfo>


void Graphics_Module::initialize(Engine* engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Graphics...");

	// Asset Loading
	m_shader = Shared_Shader(engine, "Effects\\Copy Texture");
	m_shapeQuad = Shared_Auto_Model(engine, "quad");

	// Asset-Finished Callbacks
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
		m_indirectQuad = IndirectDraw((GLuint)m_shapeQuad->getSize(), 1, 0, GL_CLIENT_STORAGE_BIT);
		});

	// GL settings
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Preferences
	auto& preferences = m_engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) {
		m_renderSize = glm::ivec2(f, m_renderSize.y);
		m_viewport->resize(m_renderSize, 1);
		(*m_clientCamera)->Dimensions = m_renderSize;
		});
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		m_viewport->resize(m_renderSize, 1);
		(*m_clientCamera)->Dimensions = m_renderSize;
		});
	float farPlane = 1000.0f;
	preferences.getOrSetValue(PreferenceState::C_DRAW_DISTANCE, farPlane);
	preferences.addCallback(PreferenceState::C_DRAW_DISTANCE, m_aliveIndicator, [&](const float& f) {
		if ((*m_clientCamera)->FarPlane != f) {
			(*m_clientCamera)->FarPlane = f;
			genPerspectiveMatrix();
		}
		});
	float fov = 90.0f;
	preferences.getOrSetValue(PreferenceState::C_FOV, fov);
	preferences.addCallback(PreferenceState::C_FOV, m_aliveIndicator, [&](const float& f) {
		if ((*m_clientCamera)->FOV != f) {
			(*m_clientCamera)->FOV = f;
			genPerspectiveMatrix();
		}
		});

	// Camera Setup
	m_viewport = std::make_shared<Viewport>(glm::ivec2(0), m_renderSize);
	m_clientCamera = std::make_shared<Camera>();
	m_clientCamera->setEnabled(true);
	m_clientCamera->get()->Dimensions = glm::vec2(m_renderSize);
	m_clientCamera->get()->FarPlane = farPlane;
	m_clientCamera->get()->FOV = fov;
	genPerspectiveMatrix();
	m_rhVolume = std::make_shared<RH_Volume>(engine);

	// Rendering Effects & systems
	m_pipeline = std::make_unique<Graphics_Pipeline>(engine, m_clientCamera);
}

void Graphics_Module::deinitialize()
{
	m_engine->getManager_Messages().statement("Unloading Module: Graphics...");

	// Update indicator
	*m_aliveIndicator = false;
	m_pipeline.reset();
	m_viewport.reset();
	m_clientCamera.reset();
	m_rhVolume.reset();
}

void Graphics_Module::renderWorld(ecsWorld& world, const float& deltaTime, const GLuint& fboID)
{
	m_indirectQuad.beginWriting();
	renderWorld(world, deltaTime, m_viewport, m_rhVolume, { m_clientCamera });
	copyToScreen(fboID);
	m_indirectQuad.endWriting();
}

void Graphics_Module::renderWorld(ecsWorld& world, const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::shared_ptr<Camera>>& cameras)
{
	if (cameras.size()) {
		// Prepare rendering pipeline for a new frame, wait for buffers to free
		const auto perspectives = m_pipeline->begin(deltaTime, world, cameras);
		viewport->bind();
		viewport->clear();
		rhVolume->clear();
		rhVolume->updateVolume(cameras[0].get());

		m_pipeline->render(deltaTime, viewport, rhVolume, perspectives);

		m_pipeline->end(deltaTime);
	}
}

void Graphics_Module::genPerspectiveMatrix()
{
	// Update Perspective Matrix
	const float ar = std::max(1.0f, (*m_clientCamera)->Dimensions.x) / std::max(1.0f, (*m_clientCamera)->Dimensions.y);
	const float horizontalRad = glm::radians((*m_clientCamera)->FOV);
	const float verticalRad = 2.0f * atanf(tanf(horizontalRad / 2.0f) / ar);
	(*m_clientCamera)->pMatrix = glm::perspective(verticalRad, ar, Camera::ConstNearPlane, (*m_clientCamera)->FarPlane);
	(*m_clientCamera)->pMatrixInverse = glm::inverse((*m_clientCamera)->pMatrix);
	(*m_clientCamera)->pvMatrix = (*m_clientCamera)->pMatrix * (*m_clientCamera)->vMatrix;
}

void Graphics_Module::copyToScreen(const GLuint& fboID)
{
	if (m_shapeQuad->existsYet() && m_shader->existsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
		m_shader->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_indirectQuad.drawCall();
		Shader::Release();
	}
}