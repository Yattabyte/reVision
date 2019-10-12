#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Engine.h"

/* Rendering Techniques Used */
#include "Modules/Graphics/Geometry/Prop/Prop_Technique.h"
#include "Modules/Graphics/Lighting/Shadow/Shadow_Technique.h"
#include "Modules/Graphics/Lighting/Directional/Directional_Technique.h"
#include "Modules/Graphics/Lighting/Point/Point_Technique.h"
#include "Modules/Graphics/Lighting/Spot/Spot_Technique.h"
#include "Modules/Graphics/Lighting/Reflector/Reflector_Technique.h"
#include "Modules/Graphics/Effects/Skybox.h"
#include "Modules/Graphics/Effects/Radiance_Hints.h"
#include "Modules/Graphics/Effects/SSAO.h"
#include "Modules/Graphics/Effects/SSR.h"
#include "Modules/Graphics/Effects/Join_Reflections.h"
#include "Modules/Graphics/Effects/Bloom.h"
#include "Modules/Graphics/Effects/HDR.h"
#include "Modules/Graphics/Effects/FXAA.h"
#include "Modules/Graphics/Logical/Transform_System.h"
#include "Modules/Graphics/Logical/CameraPerspective_System.h"
#include "Modules/Graphics/Logical/CameraArrayPerspective_System.h"
#include "Modules/Graphics/Logical/FrustumCull_System.h"
#include "Modules/Graphics/Logical/SkeletalAnimation_System.h"


Graphics_Pipeline::Graphics_Pipeline(Engine* engine, const std::shared_ptr<Camera>& clientCamera)
	: m_engine(engine)
{
	// Camera
	m_sceneCameras = std::make_shared<std::vector<Camera*>>();
	m_cameraBuffer = std::make_shared<GL_ArrayBuffer<Camera::GPUData>>();

	// Create Systems
	m_transHierachy = m_worldSystems.makeSystem<Transform_System>(m_engine);
	m_worldSystems.makeSystem<FrustumCull_System>(m_sceneCameras);
	m_worldSystems.makeSystem<Skeletal_Animation_System>(m_engine);
	m_cameraSystems.makeSystem<CameraPerspective_System>(m_sceneCameras);
	m_cameraSystems.makeSystem<CameraArrayPerspective_System>(m_sceneCameras);

	// Report invalid ecs systems
	auto& msg = engine->getManager_Messages();
	for each (const auto & system in m_worldSystems)
		if (!system->isValid())
			msg.error("Invalid ECS System: " + std::string(typeid(*system).name()));

	// Create Rendering Techniques
	auto propView = new Prop_Technique(engine, m_sceneCameras, m_worldSystems);
	auto shadowing = new Shadow_Technique(engine, m_sceneCameras, m_worldSystems);
	auto directionalLighting = new Directional_Technique(engine, shadowing->getShadowData(), clientCamera, m_sceneCameras, m_worldSystems);
	auto pointLighting = new Point_Technique(engine, shadowing->getShadowData(), m_sceneCameras, m_worldSystems);
	auto spotLighting = new Spot_Technique(engine, shadowing->getShadowData(), m_sceneCameras, m_worldSystems);
	auto reflectorLighting = new Reflector_Technique(engine, m_sceneCameras, m_worldSystems);
	auto radianceHints = new Radiance_Hints(engine);
	auto skybox = new Skybox(m_engine);
	auto ssao = new SSAO(m_engine);
	auto ssr = new SSR(m_engine);
	auto joinReflections = new Join_Reflections(m_engine);
	auto bloom = new Bloom(m_engine);
	auto hdr = new HDR(m_engine);
	auto fxaa = new FXAA(m_engine);

	// Filter Techniques
	m_geometryTechniques = {
		propView
	};
	m_lightingTechniques = {
		shadowing, directionalLighting, pointLighting, spotLighting, skybox, reflectorLighting
	};
	m_effectTechniques = {
		radianceHints, ssao, ssr, joinReflections, bloom, hdr, fxaa
	};

	// Join All Techniques
	m_allTechniques.reserve(m_geometryTechniques.size() + m_lightingTechniques.size() + m_effectTechniques.size());
	for each (const auto & tech in m_geometryTechniques)
		m_allTechniques.push_back(tech);
	for each (const auto & tech in m_lightingTechniques)
		m_allTechniques.push_back(tech);
	for each (const auto & tech in m_effectTechniques)
		m_allTechniques.push_back(tech);
}

void Graphics_Pipeline::begin()
{
	m_cameraBuffer->beginWriting();
}

void Graphics_Pipeline::end(const float& deltaTime)
{
	m_cameraBuffer->endWriting();
	for each (auto * tech in m_allTechniques)
		tech->prepareForNextFrame(deltaTime);
}

std::vector<std::pair<int, int>> Graphics_Pipeline::update(const float& deltaTime, ecsWorld& world, const std::vector<std::shared_ptr<Camera>>& cameras)
{
	// Push cameras to the beginning of the camera list
	m_sceneCameras->clear();
	m_sceneCameras->reserve(cameras.size());
	std::vector<std::pair<int, int>> perspectives;
	int count(0);
	for each (auto & camera in cameras) {
		camera->updateFrustum();
		m_sceneCameras->push_back(camera.get());
		perspectives.push_back({ count, count });
		count++;
	}

	// Find remaining cameras in the world, add them to the list
	world.updateSystems(m_worldSystems, deltaTime);

	// Aggregate camera data into camera buffer
	m_cameraBuffer->resize(m_sceneCameras->size());
	for (size_t x = 0ull; x < m_sceneCameras->size(); ++x)
		(*m_cameraBuffer)[x] = *(*m_sceneCameras)[x]->get();

	// Update world systems
	std::dynamic_pointer_cast<Transform_System>(m_transHierachy)->m_world = &world;
	world.updateSystems(m_worldSystems, deltaTime);

	// Update rendering techniques
	for each (auto * tech in m_allTechniques)
		tech->updateTechnique(deltaTime);

	return perspectives;
}

void Graphics_Pipeline::render(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::pair<int, int>>& perspectives, const unsigned int& allowedCategories)
{
	m_cameraBuffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for each (auto * tech in m_allTechniques)
		if (allowedCategories & tech->getCategory())
			tech->renderTechnique(deltaTime, viewport, rhVolume, perspectives);
}

void Graphics_Pipeline::cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives)
{
	for each (auto * tech in m_geometryTechniques)
		tech->cullShadows(deltaTime, perspectives);
}

void Graphics_Pipeline::renderShadows(const float& deltaTime)
{
	for each (auto * tech in m_geometryTechniques)
		tech->renderShadows(deltaTime);
}