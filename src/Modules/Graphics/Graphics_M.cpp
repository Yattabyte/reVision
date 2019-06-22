#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/World/World_M.h"
#include "Engine.h"
#include <memory>
#include <random>

Graphics_Module::~Graphics_Module()
{
	// Update indicator
	m_aliveIndicator = false;

	// Remove support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.removeComponentType("Prop_Component");
	world.removeComponentType("Skeleton_Component");
	world.removeComponentType("LightDirectional_Component");
	world.removeComponentType("LightPoint_Component");
	world.removeComponentType("LightSpot_Component");
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
		m_viewport->resize(m_renderSize);
	});
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		m_viewport->resize(m_renderSize);
	});
	float farPlane = 1000.0f;
	preferences.getOrSetValue(PreferenceState::C_DRAW_DISTANCE, farPlane);
	preferences.addCallback(PreferenceState::C_DRAW_DISTANCE, m_aliveIndicator, [&](const float &f) {
		m_viewport->setDrawDistance(f);
	});
	float fov = 90.0f;
	preferences.getOrSetValue(PreferenceState::C_FOV, fov);
	preferences.addCallback(PreferenceState::C_FOV, m_aliveIndicator, [&](const float &f) {
		m_viewport->setFOV(f);
	});

	// Camera Setup
	CameraBuffer::BufferStructure cameraData;
	cameraData.pMatrix = glm::mat4(1.0f);
	cameraData.vMatrix = glm::mat4(1.0f);
	cameraData.EyePosition = glm::vec3(0.0f);
	cameraData.Dimensions = glm::vec2(m_renderSize);
	cameraData.FarPlane = farPlane;
	cameraData.FOV = fov;
	m_viewport = std::make_shared<Viewport>(engine, glm::ivec2(0), m_renderSize, cameraData);

	// Rendering Effects & systems
	m_pipeline = std::make_unique<Graphics_Pipeline>(m_engine);

	// Add map support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.addComponentType("Prop_Component", [engine](const ParamList & parameters) {
		auto * component = new Prop_Component();
		component->m_model = Shared_Model(engine, CastAny(parameters, 0, std::string("")));
		component->m_skin = CastAny(parameters, 1, 0u);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Skeleton_Component", [engine](const ParamList & parameters) {
		auto * component = new Skeleton_Component();
		component->m_mesh = Shared_Mesh(engine, "\\Models\\" + CastAny(parameters, 0, std::string("")));
		component->m_animation = CastAny(parameters, 1, 0);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightDirectional_Component", [](const ParamList & parameters) {
		auto * component = new LightDirectional_Component();
		component->m_color = CastAny(parameters, 0, glm::vec3(1.0f));
		component->m_intensity = CastAny(parameters, 1, 1.0f);
		component->m_hasShadow = CastAny(parameters, 2, false);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightPoint_Component", [](const ParamList & parameters) {
		auto * component = new LightPoint_Component();
		component->m_color = CastAny(parameters, 0, glm::vec3(1.0f));
		component->m_intensity = CastAny(parameters, 1, 1.0f);
		component->m_radius = CastAny(parameters, 2, 1.0f);
		component->m_hasShadow = CastAny(parameters, 3, false);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightSpot_Component", [](const ParamList & parameters) {
		auto * component = new LightSpot_Component();
		component->m_color = CastAny(parameters, 0, glm::vec3(1.0f));
		component->m_intensity = CastAny(parameters, 1, 1.0f);
		component->m_radius = CastAny(parameters, 2, 1.0f);
		component->m_cutoff = CastAny(parameters, 3, 45.0f);
		component->m_hasShadow = CastAny(parameters, 4, false);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Reflector_Component", [](const ParamList & parameters) {
		auto * component = new Reflector_Component();
		return std::make_pair(component->ID, component);
	});
}

void Graphics_Module::frameTick(const float & deltaTime)
{
	// Prepare rendering pipeline for a new frame, wait for buffers to free
	m_pipeline->beginFrame(deltaTime);
	m_viewport->m_cameraBuffer->beginWriting();
	m_viewport->m_cameraBuffer->pushChanges();

	// Clear Frame Buffers
	m_viewport->clear();
	m_viewport->bind();	

	// Render graphics pipeline using current set of buffers
	render(deltaTime, m_viewport);

	// Consolidate and prepare for the next frame, swap to next set of buffers
	m_pipeline->endFrame(deltaTime);
	m_viewport->m_cameraBuffer->endWriting();
}

void Graphics_Module::render(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const unsigned int & allowedCategories)
{
	m_pipeline->render(deltaTime, viewport, allowedCategories);
}
