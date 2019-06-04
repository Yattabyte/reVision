#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/World/World_M.h"
#include "Engine.h"
#include <memory>
#include <random>

/* Graphics Effects Used */
#include "Modules/Graphics/Geometry/Prop_View.h"
#include "Modules/Graphics/Lighting/Directional_Lighting.h"
#include "Modules/Graphics/Lighting/Point_Lighting.h"
#include "Modules/Graphics/Lighting/Spot_Lighting.h"
#include "Modules/Graphics/Lighting/Reflector_Lighting.h"

/* System Types Used */
#include "Modules/Graphics/ECS/TransformSync_S.h"
#include "Modules/Graphics/ECS/SkeletonAnimation_S.h"

/* Other Effects Used */
#include "Modules/Graphics/Effects/Skybox.h"
#include "Modules/Graphics/Effects/SSAO.h"
#include "Modules/Graphics/Effects/Radiance_Hints.h"
#include "Modules/Graphics/Effects/Join_Reflections.h"
#include "Modules/Graphics/Effects/SSR.h"


Graphics_Module::~Graphics_Module()
{
	// Update indicator
	m_aliveIndicator = false;
}

void Graphics_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Graphics...");

	// GL settings
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Preferences
	auto & preferences = m_engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_renderSize = glm::ivec2(f, m_renderSize.y);
		m_geometryFBO.resize(m_renderSize.x, m_renderSize.y);
		m_lightingFBO.resize(m_renderSize.x, m_renderSize.y);
		m_reflectionFBO.resize(m_renderSize.x, m_renderSize.y);
		m_cameraBuffer->Dimensions = m_renderSize;
		updateCamera(m_cameraBuffer);
	});
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		m_geometryFBO.resize(m_renderSize.x, m_renderSize.y);
		m_lightingFBO.resize(m_renderSize.x, m_renderSize.y);
		m_reflectionFBO.resize(m_renderSize.x, m_renderSize.y);
		m_cameraBuffer->Dimensions = m_renderSize;
		updateCamera(m_cameraBuffer);
		
	});
	GLuint m_bounceSize = 16;
	preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
	preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_bounceSize = (GLuint)f;  m_bounceFBO.resize((GLuint)f); });
	m_bounceFBO.resize(m_bounceSize);
	float farPlane = 1000.0f;
	preferences.getOrSetValue(PreferenceState::C_DRAW_DISTANCE, farPlane);
	preferences.addCallback(PreferenceState::C_DRAW_DISTANCE, m_aliveIndicator, [&](const float &f) {		
		m_cameraBuffer->FarPlane = f;
		updateCamera(m_cameraBuffer);		
	});
	float fov = 90.0f;
	preferences.getOrSetValue(PreferenceState::C_FOV, fov);
	preferences.addCallback(PreferenceState::C_FOV, m_aliveIndicator, [&](const float &f) {
		m_cameraBuffer->FOV = f;
		updateCamera(m_cameraBuffer);		
	});

	// Camera Setup
	m_cameraBuffer->pMatrix = glm::mat4(1.0f);
	m_cameraBuffer->vMatrix = glm::mat4(1.0f);
	m_cameraBuffer->EyePosition = glm::vec3(0.0f);
	m_cameraBuffer->Dimensions = m_renderSize;
	m_cameraBuffer->FarPlane = farPlane;
	m_cameraBuffer->FOV = fov;
	updateCamera(m_cameraBuffer);		

	// Error Reporting
	auto & msgMgr = m_engine->getManager_Messages();
	if (glCheckNamedFramebufferStatus(m_geometryFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		msgMgr.error("Geometry Framebuffer has encountered an error.");
	if (glCheckNamedFramebufferStatus(m_lightingFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		msgMgr.error("Lighting Framebuffer has encountered an error.");
	if (glCheckNamedFramebufferStatus(m_reflectionFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		msgMgr.error("Reflection Framebuffer has encountered an error.");

	// Initialization
	m_visualFX.initialize(m_engine);
	m_geometryFBO.resize(m_renderSize.x, m_renderSize.y);
	m_lightingFBO.resize(m_renderSize.x, m_renderSize.y);
	m_reflectionFBO.resize(m_renderSize.x, m_renderSize.y);
	m_lightingFBO.attachTexture(m_geometryFBO.m_textureIDS[3], GL_DEPTH_STENCIL_ATTACHMENT);
	m_reflectionFBO.attachTexture(m_geometryFBO.m_textureIDS[3], GL_DEPTH_STENCIL_ATTACHMENT);
	m_volumeRH = std::shared_ptr<RH_Volume>(new RH_Volume(m_engine));

	// Rendering Effects & systems
	auto * propView = new Prop_View(m_engine, &m_geometryFBO);
	auto * directionalLighting = new Directional_Lighting(m_engine, &m_geometryFBO, &m_lightingFBO, &m_bounceFBO, m_volumeRH, propView);
	auto * pointLighting = new Point_Lighting(m_engine, &m_geometryFBO, &m_lightingFBO, propView);
	auto * spotLighting = new Spot_Lighting(m_engine, &m_geometryFBO, &m_lightingFBO, propView);
	auto * reflectorLighting = new Reflector_Lighting(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO);
	auto * transformSync = new TransformSync_Gfx_System(m_engine);
	auto * skeleAnimation = new SkeletonAnimation_System(m_engine);
	auto * skybox = new Skybox(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO);
	auto * radianceHints = new Radiance_Hints(m_engine, &m_geometryFBO, &m_bounceFBO, m_volumeRH);
	auto * ssao = new SSAO(m_engine, &m_geometryFBO, &m_visualFX);
	auto * ssr = new SSR(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO);
	auto * joinReflections = new Join_Reflections(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO);

	// ECS Pipeline
	m_ecsSystems.addSystem(transformSync);
	m_ecsSystems.addSystem(skeleAnimation);
	m_ecsSystems.addSystem(propView);
	m_ecsSystems.addSystem(directionalLighting);
	m_ecsSystems.addSystem(pointLighting);
	m_ecsSystems.addSystem(spotLighting);
	m_ecsSystems.addSystem(reflectorLighting);

	// Rendering Pipeline
	m_gfxTechs.push_back(propView);
	m_gfxTechs.push_back(directionalLighting);
	m_gfxTechs.push_back(pointLighting);
	m_gfxTechs.push_back(spotLighting);
	m_gfxTechs.push_back(reflectorLighting);
	m_gfxTechs.push_back(skybox);	
	m_gfxTechs.push_back(radianceHints);
	m_gfxTechs.push_back(ssao);
	m_gfxTechs.push_back(ssr);
	m_gfxTechs.push_back(joinReflections);
}

void Graphics_Module::frameTick(const float & deltaTime)
{
	// Clear Frame Buffers
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	m_geometryFBO.clear();
	m_lightingFBO.clear();
	m_reflectionFBO.clear();
	m_bounceFBO.clear();

	// Wait on triple-buffered camera lock, then update camera
	m_cameraBuffer.waitFrame(m_engine->getCurrentFrame());
	m_cameraBuffer.bind(2, m_engine->getCurrentFrame());
	m_cameraBuffer.pushChanges(m_engine->getCurrentFrame());
	m_volumeRH->updateVolume(m_cameraBuffer);

	// Update Components
	m_engine->getModule_World().updateSystems(m_ecsSystems, deltaTime);

	// Render Techniques
	for each (auto *tech in m_gfxTechs)
		tech->applyEffect(deltaTime);

	// Set lock for 3 frames from now
	m_cameraBuffer.lockFrame(m_engine->getCurrentFrame());
}

void Graphics_Module::updateCamera(CameraBuffer & cameraBuffer) const
{
	// Update Perspective Matrix
	const float ar = std::max(1.0f, cameraBuffer->Dimensions.x) / std::max(1.0f, cameraBuffer->Dimensions.y);
	const float horizontalRad = glm::radians(cameraBuffer->FOV);
	const float verticalRad = 2.0f * atanf(tanf(horizontalRad / 2.0f) / ar);
	cameraBuffer->pMatrix = glm::perspective(verticalRad, ar, CameraBuffer::BufferStructure::ConstNearPlane, cameraBuffer->FarPlane);
}

CameraBuffer & Graphics_Module::getCameraBuffer()
{
	return m_cameraBuffer;
}
