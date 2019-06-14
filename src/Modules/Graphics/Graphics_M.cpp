#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/World/World_M.h"
#include "Engine.h"
#include <memory>
#include <random>

/* Graphics Effects Used */
#include "Modules/Graphics/Geometry/Prop_Animation.h"
#include "Modules/Graphics/Geometry/Prop_View.h"
#include "Modules/Graphics/Lighting/Directional_Lighting.h"
#include "Modules/Graphics/Lighting/Point_Lighting.h"
#include "Modules/Graphics/Lighting/Spot_Lighting.h"
#include "Modules/Graphics/Lighting/Reflector_Lighting.h"

/* Other Effects Used */
#include "Modules/Graphics/Effects/Skybox.h"
#include "Modules/Graphics/Effects/Radiance_Hints.h"
#include "Modules/Graphics/Effects/SSAO.h"
#include "Modules/Graphics/Effects/SSR.h"
#include "Modules/Graphics/Effects/Join_Reflections.h"
#include "Modules/Graphics/Effects/Bloom.h"


Graphics_Module::~Graphics_Module()
{
	// Update indicator
	m_aliveIndicator = false;

	// Remove support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.removeComponentType("Prop_Component");
	world.removeComponentType("Skeleton_Component");
	world.removeComponentType("LightDirectional_Component");
	world.removeComponentType("LightDirectionalShadow_Component");
	world.removeComponentType("LightPoint_Component");
	world.removeComponentType("LightPointShadow_Component");
	world.removeComponentType("LightSpot_Component");
	world.removeComponentType("LightSpotShadow_Component");
	world.removeComponentType("Reflector_Component");
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
		m_graphicsFBOS->resize(m_renderSize);
	});
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		(*m_cameraBuffer)->Dimensions = m_renderSize;
		updateCamera();
		m_graphicsFBOS->resize(m_renderSize);
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
	m_graphicsFBOS = std::make_shared<Graphics_Framebuffers>(m_renderSize);
	m_graphicsFBOS->createFBO("GEOMETRY", { { GL_RGB16F, GL_RGB, GL_FLOAT }, { GL_RGB16F, GL_RGB, GL_FLOAT }, { GL_RGBA16F, GL_RGBA, GL_FLOAT }, { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 } });
	m_graphicsFBOS->createFBO("LIGHTING", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	m_graphicsFBOS->createFBO("REFLECTION", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	m_graphicsFBOS->createFBO("BOUNCE", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	m_graphicsFBOS->createFBO("SSAO", { { GL_RG8, GL_RED, GL_FLOAT }, { GL_RG8, GL_RED, GL_FLOAT }, { GL_RG8, GL_RED, GL_FLOAT } });
	m_graphicsFBOS->createFBO("SSR", { { GL_RGB8, GL_RGB, GL_FLOAT } });
	m_graphicsFBOS->createFBO("SSR_MIP", { { GL_RGB8, GL_RGB, GL_FLOAT } }, true);
	m_graphicsFBOS->createFBO("BLOOM", { { GL_RGB16F, GL_RGB, GL_FLOAT }, { GL_RGB16F, GL_RGB, GL_FLOAT } });
	glNamedFramebufferTexture(m_graphicsFBOS->getFboID("LIGHTING"), GL_DEPTH_STENCIL_ATTACHMENT, m_graphicsFBOS->getTexID("GEOMETRY", 3), 0);	
	glNamedFramebufferTexture(m_graphicsFBOS->getFboID("REFLECTION"), GL_DEPTH_STENCIL_ATTACHMENT, m_graphicsFBOS->getTexID("GEOMETRY", 3), 0);	
	m_visualFX.initialize(m_engine);
	m_volumeRH = std::make_shared<RH_Volume>(m_engine);

	// Rendering Effects & systems
	auto propView = new Prop_View(m_engine);
	m_pipeline = Graphics_Pipeline(m_engine, {
		new Prop_Animation(m_engine),
		propView,
		new Directional_Lighting(m_engine, propView),
		new Point_Lighting(m_engine, propView),
		new Spot_Lighting(m_engine, propView),
		new Reflector_Lighting(m_engine),
		new Skybox(m_engine),
		new Radiance_Hints(m_engine),
		new SSAO(m_engine, &m_visualFX),
		new SSR(m_engine),
		new Join_Reflections(m_engine),
		new Bloom(m_engine)
	});

	// Add support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.addComponentType("Prop_Component", [engine](const ParamList & parameters) {
		auto * component = new Prop_Component();
		component->m_model = Shared_Model(engine, CastAny(parameters[0], std::string("")));
		component->m_skin = CastAny(parameters[1], 0u);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Skeleton_Component", [engine](const ParamList & parameters) {
		auto * component = new Skeleton_Component();
		component->m_mesh = Shared_Mesh(engine, "\\Models\\" + CastAny(parameters[0], std::string("")));
		component->m_animation = CastAny(parameters[1], 0);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightDirectional_Component", [](const ParamList & parameters) {
		auto * component = new LightDirectional_Component();
		component->m_color = CastAny(parameters[0], glm::vec3(1.0f));
		component->m_intensity = CastAny(parameters[1], 1.0f);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightDirectionalShadow_Component", [](const ParamList & parameters) {
		auto * component = new LightDirectionalShadow_Component();		
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightPoint_Component", [](const ParamList & parameters) {
		auto * component = new LightPoint_Component();
		component->m_color = CastAny(parameters[0], glm::vec3(1.0f));
		component->m_intensity = CastAny(parameters[1], 1.0f);
		component->m_radius = CastAny(parameters[2], 1.0f);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightPointShadow_Component", [](const ParamList & parameters) {
		auto * component = new LightPointShadow_Component();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightSpot_Component", [](const ParamList & parameters) {
		auto * component = new LightSpot_Component();
		component->m_color = CastAny(parameters[0], glm::vec3(1.0f));
		component->m_intensity = CastAny(parameters[1], 1.0f);
		component->m_radius = CastAny(parameters[2], 1.0f);
		component->m_cutoff = CastAny(parameters[3], 45.0f);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightSpotShadow_Component", [](const ParamList & parameters) {
		auto * component = new LightSpotShadow_Component();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Reflector_Component", [](const ParamList & parameters) {
		auto * component = new Reflector_Component();
		return std::make_pair(component->ID, component);
	});
}

void Graphics_Module::frameTick(const float & deltaTime)
{
	// Clear Frame Buffers
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	m_graphicsFBOS->clear();
	m_volumeRH->clear();

	// Wait on triple-buffered camera lock, then update camera
	m_cameraBuffer->waitFrame(m_engine->getCurrentFrame());
	m_cameraBuffer->pushChanges(m_engine->getCurrentFrame());
	m_cameraBuffer->bind(2, m_engine->getCurrentFrame());
	m_volumeRH->updateVolume(m_cameraBuffer);

	// Apply Graphics Pipeline
	render(deltaTime, m_cameraBuffer, m_graphicsFBOS, m_volumeRH);

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

void Graphics_Module::render(const float & deltaTime, const std::shared_ptr<CameraBuffer> & cameraBuffer, const std::shared_ptr<Graphics_Framebuffers> & gfxFBOS, const std::shared_ptr<RH_Volume> & volumeRH)
{
	m_pipeline.render(deltaTime, cameraBuffer, gfxFBOS, volumeRH);
}
