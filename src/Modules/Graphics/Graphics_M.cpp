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
		(*m_cameraBuffer)->Dimensions = m_renderSize;
		updateCamera();
	});
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		(*m_cameraBuffer)->Dimensions = m_renderSize;
		updateCamera();
	});
	float farPlane = 1000.0f;
	preferences.getOrSetValue(PreferenceState::C_DRAW_DISTANCE, farPlane);
	preferences.addCallback(PreferenceState::C_DRAW_DISTANCE, m_aliveIndicator, [&](const float &f) {
		(*m_cameraBuffer)->FarPlane = f;
		updateCamera();
	});
	float fov = 90.0f;
	preferences.getOrSetValue(PreferenceState::C_FOV, fov);
	preferences.addCallback(PreferenceState::C_FOV, m_aliveIndicator, [&](const float &f) {
		(*m_cameraBuffer)->FOV = f;
		updateCamera();
	});

	// Camera Setup
	m_cameraBuffer = std::make_shared<CameraBuffer>();
	(*m_cameraBuffer)->pMatrix = glm::mat4(1.0f);
	(*m_cameraBuffer)->vMatrix = glm::mat4(1.0f);
	(*m_cameraBuffer)->EyePosition = glm::vec3(0.0f);
	(*m_cameraBuffer)->Dimensions = m_renderSize;
	(*m_cameraBuffer)->FarPlane = farPlane;
	(*m_cameraBuffer)->FOV = fov;
	updateCamera();

	// Initialization
	m_graphicsFBOS = std::make_shared<Graphics_Framebuffers>(engine);
	m_graphicsFBOS->createFBO("GEOMETRY", { { GL_RGB16F, GL_RGB, GL_FLOAT }, { GL_RGB16F, GL_RGB, GL_FLOAT }, { GL_RGBA16F, GL_RGBA, GL_FLOAT }, { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 } });
	m_graphicsFBOS->createFBO("LIGHTING", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	m_graphicsFBOS->createFBO("REFLECTION", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	glNamedFramebufferTexture(m_graphicsFBOS->m_fbos["LIGHTING"].first, GL_DEPTH_STENCIL_ATTACHMENT, std::get<0>(m_graphicsFBOS->m_fbos["GEOMETRY"].second.back()), 0);	
	glNamedFramebufferTexture(m_graphicsFBOS->m_fbos["REFLECTION"].first, GL_DEPTH_STENCIL_ATTACHMENT, std::get<0>(m_graphicsFBOS->m_fbos["GEOMETRY"].second.back()), 0);		
	m_visualFX.initialize(m_engine);
	m_volumeRH = std::shared_ptr<RH_Volume>(new RH_Volume(m_engine, m_cameraBuffer));

	// Rendering Effects & systems
	auto * transformSync = new TransformSync_Gfx_System(m_engine);
	auto * skeleAnimation = new SkeletonAnimation_System(m_engine);
	auto * propView = new Prop_View(m_engine, m_cameraBuffer, m_graphicsFBOS);
	auto * directionalLighting = new Directional_Lighting(m_engine, m_cameraBuffer, m_graphicsFBOS, m_volumeRH, propView);
	auto * pointLighting = new Point_Lighting(m_engine, m_cameraBuffer, m_graphicsFBOS, propView);
	auto * spotLighting = new Spot_Lighting(m_engine, m_cameraBuffer, m_graphicsFBOS, propView);
	auto * reflectorLighting = new Reflector_Lighting(m_engine, m_cameraBuffer, m_graphicsFBOS);
	auto * skybox = new Skybox(m_engine, m_graphicsFBOS);
	auto * radianceHints = new Radiance_Hints(m_engine, m_cameraBuffer, m_graphicsFBOS, m_volumeRH);
	auto * ssao = new SSAO(m_engine, m_graphicsFBOS, &m_visualFX);
	auto * ssr = new SSR(m_engine, m_graphicsFBOS);
	auto * joinReflections = new Join_Reflections(m_engine, m_graphicsFBOS);

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
	m_graphicsFBOS->clear();
	m_volumeRH->clear();

	// Wait on triple-buffered camera lock, then update camera
	m_cameraBuffer->waitFrame(m_engine->getCurrentFrame());
	m_cameraBuffer->bind(2, m_engine->getCurrentFrame());
	m_cameraBuffer->pushChanges(m_engine->getCurrentFrame());
	m_volumeRH->updateVolume();

	// Update Components
	m_engine->getModule_World().updateSystems(m_ecsSystems, deltaTime);

	// Render Techniques
	for each (auto *tech in m_gfxTechs)
		tech->applyEffect(deltaTime);

	// Set lock for 3 frames from now
	m_cameraBuffer->lockFrame(m_engine->getCurrentFrame());
}

void Graphics_Module::updateCamera()
{
	// Update Perspective Matrix
	const float ar = std::max(1.0f, (*m_cameraBuffer)->Dimensions.x) / std::max(1.0f, (*m_cameraBuffer)->Dimensions.y);
	const float horizontalRad = glm::radians((*m_cameraBuffer)->FOV);
	const float verticalRad = 2.0f * atanf(tanf(horizontalRad / 2.0f) / ar);
	(*m_cameraBuffer)->pMatrix = glm::perspective(verticalRad, ar, CameraBuffer::BufferStructure::ConstNearPlane, (*m_cameraBuffer)->FarPlane);
}

std::shared_ptr<CameraBuffer> Graphics_Module::getCameraBuffer() const
{
	return m_cameraBuffer;
}
