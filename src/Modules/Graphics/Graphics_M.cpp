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
		m_quadIndirectBuffer = StaticTripleBuffer(sizeof(GLuint) * 4, quadData, GL_CLIENT_STORAGE_BIT);
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
		m_viewport->resize(m_renderSize, 1);
		(*m_clientCamera)->Dimensions = m_renderSize;
	});
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		m_viewport->resize(m_renderSize, 1);
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
	m_viewport = std::make_shared<Viewport>(glm::ivec2(0), m_renderSize);
	m_clientCamera = std::make_shared<Camera>();
	m_clientCamera->setEnabled(true);
	m_clientCamera->get()->Dimensions = glm::vec2(m_renderSize);
	m_clientCamera->get()->FarPlane = farPlane;
	m_clientCamera->get()->FOV = fov;
	genPerspectiveMatrix();
	m_rhVolume = std::make_shared<RH_Volume>(engine);

	// Rendering Effects & systems
	m_sceneCameras = std::make_shared<std::vector<Camera*>>();
	m_cameraBuffer = std::make_shared<GL_ArrayBuffer<Camera::GPUData>>();
	auto sharedCameraCounter = std::make_shared<int>(0);
	m_systems.addSystem(new CameraPerspective_System(m_sceneCameras));
	m_systems.addSystem(new CameraArrayPerspective_System(m_sceneCameras));
	m_systems.addSystem(new FrustumCull_System(m_sceneCameras));
	m_systems.addSystem(new Skeletal_Animation(engine));
	m_pipeline = std::make_unique<Graphics_Pipeline>(m_engine, m_clientCamera, m_sceneCameras, m_rhVolume, m_systems);

	// Add map support for the following list of component types
	auto & world = m_engine->getModule_World();
	world.addComponentType("Renderable_Component", [engine](const ParamList & parameters) {
		auto * component = new Renderable_Component();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Camera_Component", [&, engine](const ParamList & parameters) {
		auto * component = new Camera_Component();
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
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightPoint_Component", [](const ParamList & parameters) {
		auto * component = new LightPoint_Component();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightSpot_Component", [](const ParamList & parameters) {
		auto * component = new LightSpot_Component();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Shadow_Component", [](const ParamList & parameters) {
		auto * component = new Shadow_Component();
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
	world.removeComponentType("Shadow_Component");
	world.removeComponentType("Reflector_Component");
}

void Graphics_Module::frameTick(const float & deltaTime)
{
	// Prepare rendering pipeline for a new frame, wait for buffers to free
	m_clientCamera->updateFrustum();
	m_quadIndirectBuffer.beginWriting();
	m_cameraBuffer->beginWriting();
	m_sceneCameras->clear();
	m_sceneCameras->push_back(m_clientCamera.get());
	m_rhVolume->clear();
	m_rhVolume->updateVolume(m_clientCamera.get());

	// All ECS Systems updated once per frame, updating components pertaining to all viewing perspectives
	m_engine->getModule_World().updateSystems(m_systems, deltaTime);
	m_cameraBuffer->resize(m_sceneCameras->size());
	for (size_t x = 0ull; x < m_sceneCameras->size(); ++x)
		(*m_cameraBuffer)[x] = *(*m_sceneCameras)[x]->get();

	// Update pipeline techniques ONCE per frame, not per render call!
	m_pipeline->update(deltaTime);

	// Render the scene from the user's perspective to the screen
	size_t visibilityIndex = 0;
	bool found = false;
	for (size_t x = 0; x < m_sceneCameras->size(); ++x)
		if (m_sceneCameras->at(x) == m_clientCamera.get()) {
			visibilityIndex = x;
			found = true;
			break;
		}
	if (found) {
		renderScene(deltaTime, m_viewport, { {visibilityIndex, 0} });
		copyToScreen();
	}

	// Consolidate and prepare for the next frame, swap to next set of buffers
	m_pipeline->prepareForNextFrame(deltaTime);
	m_cameraBuffer->endWriting();
	m_quadIndirectBuffer.endWriting();
}

void Graphics_Module::renderScene(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives,  const unsigned int & allowedCategories)
{
	// Prepare viewport and camera for rendering
	viewport->bind();
	viewport->clear();
	m_cameraBuffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);

	// Render
	m_pipeline->render(deltaTime, viewport, perspectives, allowedCategories);
}

void Graphics_Module::cullShadows(const float & deltaTime, const std::vector<std::pair<int, int>>& perspectives)
{
	// Apply frustum culling or other techniques
	m_cameraBuffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	m_pipeline->cullShadows(deltaTime, perspectives);
}

void Graphics_Module::renderShadows(const float & deltaTime)
{
	// Render remaining shadows
	m_cameraBuffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	m_pipeline->renderShadows(deltaTime);
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