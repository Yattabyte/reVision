#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Logical/CameraPerspective_System.h"
#include "Modules/Graphics/Logical/CameraArrayPerspective_System.h"
#include "Modules/Graphics/Logical/FrustumCull_System.h"
#include "Modules/Graphics/Logical/SkeletalAnimation_System.h"
#include "Modules/World/World_M.h"
#include "Engine.h"
#include <memory>
#include <random>
#include <typeinfo>


void Graphics_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Graphics...");

	// Asset Loading
	m_shader = Shared_Shader(m_engine, "Effects\\Copy Texture");
	m_shapeQuad = Shared_Primitive(m_engine, "quad");	
	
	// Asset-Finished Callbacks
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData, 0);
	});

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
		(*m_clientCamera)->Dimensions = m_renderSize;
	});
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		m_viewport->resize(m_renderSize);
		(*m_clientCamera)->Dimensions = m_renderSize;
	});
	float farPlane = 1000.0f;
	preferences.getOrSetValue(PreferenceState::C_DRAW_DISTANCE, farPlane);
	preferences.addCallback(PreferenceState::C_DRAW_DISTANCE, m_aliveIndicator, [&](const float &f) {
		if ((*m_clientCamera)->FarPlane != f) {
			(*m_clientCamera)->FarPlane = f;
			genPerspectiveMatrix();
		}
	});
	float fov = 90.0f;
	preferences.getOrSetValue(PreferenceState::C_FOV, fov);
	preferences.addCallback(PreferenceState::C_FOV, m_aliveIndicator, [&](const float &f) {
		if ((*m_clientCamera)->FOV != f) {
			(*m_clientCamera)->FOV = f;
			genPerspectiveMatrix();
		}
	});

	// Camera Setup
	m_viewport = std::make_shared<Viewport>(engine, glm::ivec2(0), m_renderSize);
	CameraBuffer::BufferStructure cameraData;
	cameraData.pMatrix = glm::mat4(1.0f);
	cameraData.vMatrix = glm::mat4(1.0f);
	cameraData.EyePosition = glm::vec3(0.0f);
	cameraData.Dimensions = glm::vec2(m_renderSize);
	cameraData.FarPlane = farPlane;
	cameraData.FOV = fov;
	m_clientCamera = std::make_shared<CameraBuffer>();
	m_clientCamera->replace(cameraData);
	genPerspectiveMatrix();

	// Rendering Effects & systems
	auto sceneViewports = std::make_shared<std::vector<std::shared_ptr<CameraBuffer>>>();
	m_systems.addSystem(new CameraPerspective_System(sceneViewports));
	m_systems.addSystem(new CameraArrayPerspective_System(sceneViewports));
	m_systems.addSystem(new FrustumCull_System(sceneViewports));
	m_systems.addSystem(new Skeletal_Animation(engine));
	m_pipeline = std::make_unique<Graphics_Pipeline>(m_engine, m_clientCamera, sceneViewports, m_systems);

	// Add map support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.addComponentType("Renderable_Component", [engine](const ParamList & parameters) {
		auto * component = new Renderable_Component();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Camera_Component", [&, engine](const ParamList & parameters) {
		auto * component = new Camera_Component();
		component->m_camera = std::make_shared<CameraBuffer>();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("CameraArray_Component", [&, engine](const ParamList & parameters) {
		auto * component = new CameraArray_Component();
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

	// Reprot invalid ecs systems
	auto & msg = engine->getManager_Messages();
	for each (const auto & system in m_systems)
		if (!system->isValid())
			msg.error("Invalid ECS System: " + std::string(typeid(*system).name()));
}

void Graphics_Module::deinitialize()
{
	m_engine->getManager_Messages().statement("Closing Module: Graphics...");

	// Update indicator
	m_aliveIndicator = false;

	// Remove support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.removeComponentType("Renderable_Component");
	world.removeComponentType("Camera_Component");
	world.removeComponentType("CameraArray_Component");
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
	m_clientCamera->beginWriting();
	m_clientCamera->pushChanges();

	// All ECS Systems updated once per frame, updating components pertaining to all viewing perspectives
	m_engine->getModule_World().updateSystems(m_systems, deltaTime);

	// Update pipeline techniques ONCE per frame, not per render call!
	m_pipeline->update(deltaTime);

	// Render the scene from the user's perspective to the screen
	renderScene(deltaTime, m_viewport, m_clientCamera);
	copyToScreen();

	// Consolidate and prepare for the next frame, swap to next set of buffers
	m_pipeline->prepareForNextFrame(deltaTime);
	m_clientCamera->endWriting();
}

void Graphics_Module::renderScene(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::shared_ptr<CameraBuffer> & camera, const unsigned int & allowedCategories)
{
	// Prepare viewport and camera for rendering
	viewport->clear();
	viewport->bind(camera);
	camera->bind(2);

	// Render
	m_pipeline->render(deltaTime, viewport, camera, allowedCategories);
}

void Graphics_Module::renderShadows(const float & deltaTime, const std::shared_ptr<CameraBuffer> & camera, const int & layer)
{
	// Prepare camera for rendering
	camera->bind(2);

	// Render
	m_pipeline->shadow(deltaTime, camera, layer);
}

void Graphics_Module::genPerspectiveMatrix() 
{
	// Update Perspective Matrix
	const float ar = std::max(1.0f, (*m_clientCamera)->Dimensions.x) / std::max(1.0f, (*m_clientCamera)->Dimensions.y);
	const float horizontalRad = glm::radians((*m_clientCamera)->FOV);
	const float verticalRad = 2.0f * atanf(tanf(horizontalRad / 2.0f) / ar);
	(*m_clientCamera)->pMatrix = glm::perspective(verticalRad, ar, CameraBuffer::ConstNearPlane, (*m_clientCamera)->FarPlane);
}

void Graphics_Module::copyToScreen()
{
	if (m_shapeQuad->existsYet() && m_shader->existsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		m_shader->bind();
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}
