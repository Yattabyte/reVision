#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Logical/ViewingPerspective_System.h"
#include "Modules/Graphics/Logical/FrustumCull_System.h"
#include "Modules/Graphics/Logical/CameraFollower_System.h"
#include "Modules/Graphics/Logical/SkeletalAnimation_System.h"
#include "Modules/World/World_M.h"
#include "Engine.h"
#include <memory>
#include <random>


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
	auto sceneViewports = std::make_shared<std::vector<Viewport*>>();
	m_systems.addSystem(new ViewingPerspective_System(sceneViewports));
	m_systems.addSystem(new FrustumCull_System(sceneViewports));
	m_systems.addSystem(&m_cameraFollower);
	m_systems.addSystem(new Skeletal_Animation(engine));
	m_pipeline = std::make_unique<Graphics_Pipeline>(m_engine, sceneViewports, m_systems);

	// Add map support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.addComponentType("Renderable_Component", [engine](const ParamList & parameters) {
		auto * component = new Renderable_Component();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("CameraFollower_Component", [engine](const ParamList & parameters) {
		auto * component = new CameraFollower_Component();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Viewport_Component", [&, engine](const ParamList & parameters) {
		auto * component = new Viewport_Component();
		component->m_camera = std::make_shared<Viewport>(engine, glm::ivec2(0), m_renderSize, cameraData);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("BoundingSphere_Component", [engine](const ParamList & parameters) {
		auto * component = new BoundingSphere_Component();
		return std::make_pair(component->ID, component);
	});
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
	world.addComponentType("LightColor_Component", [](const ParamList & parameters) {
		auto * component = new LightColor_Component();
		component->m_color = CastAny(parameters, 0, glm::vec3(1.0f));
		component->m_intensity = CastAny(parameters, 1, 1.0f);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightRadius_Component", [](const ParamList & parameters) {
		auto * component = new LightRadius_Component();
		component->m_radius = CastAny(parameters, 0, 1.0f);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightCutoff_Component", [](const ParamList & parameters) {
		auto * component = new LightCutoff_Component();
		component->m_cutoff = CastAny(parameters, 0, 45.0f);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightDirectional_Component", [](const ParamList & parameters) {
		auto * component = new LightDirectional_Component();
		component->m_hasShadow = CastAny(parameters, 0, false);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightPoint_Component", [](const ParamList & parameters) {
		auto * component = new LightPoint_Component();
		component->m_hasShadow = CastAny(parameters, 0, false);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightSpot_Component", [](const ParamList & parameters) {
		auto * component = new LightSpot_Component();	
		component->m_hasShadow = CastAny(parameters, 0, false);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Reflector_Component", [](const ParamList & parameters) {
		auto * component = new Reflector_Component();
		return std::make_pair(component->ID, component);
	});
}

void Graphics_Module::deinitialize()
{
	m_engine->getManager_Messages().statement("Closing Module: Graphics...");

	// Update indicator
	m_aliveIndicator = false;

	// Remove support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.removeComponentType("Renderable_Component");
	world.removeComponentType("CameraFollower_Component");
	world.removeComponentType("Viewport_Component");
	world.removeComponentType("BoundingSphere_Component");
	world.removeComponentType("Prop_Component");
	world.removeComponentType("Skeleton_Component");
	world.removeComponentType("LightColor_Component");
	world.removeComponentType("LightRadius_Component");
	world.removeComponentType("LightCutoff_Component");
	world.removeComponentType("LightDirectional_Component");
	world.removeComponentType("LightPoint_Component");
	world.removeComponentType("LightSpot_Component");
	world.removeComponentType("Reflector_Component");
}

void Graphics_Module::frameTick(const float & deltaTime)
{
	// Prepare rendering pipeline for a new frame, wait for buffers to free
	m_pipeline->beginFrame(deltaTime);
	m_viewport->m_cameraBuffer->beginWriting();
	m_viewport->m_cameraBuffer->pushChanges();

	// All ECS Systems updated once per frame, updating components pertaining to all viewing perspectives
	m_engine->getModule_World().updateSystems(m_systems, deltaTime);

	// Update pipeline techniques ONCE per frame, not per render call!
	m_pipeline->update(deltaTime);

	// Render the scene from the user's perspective
	render(deltaTime, m_viewport);

	// Consolidate and prepare for the next frame, swap to next set of buffers
	m_pipeline->endFrame(deltaTime);
	m_viewport->m_cameraBuffer->endWriting();
}

void Graphics_Module::render(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const unsigned int & allowedCategories)
{
	// Prepare viewport for rendering
	viewport->clear();
	viewport->bind();
	m_cameraFollower.setViewport(viewport);

	// Update v

	// Render
	m_pipeline->render(deltaTime, viewport, allowedCategories);
}
