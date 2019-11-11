#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Engine.h"

/* Rendering Techniques Used */
#include "Modules/Graphics/Geometry/Prop/Prop_Technique.h"
#include "Modules/Graphics/Lighting/Shadow/Shadow_Technique.h"
#include "Modules/Graphics/Lighting/Direct/Direct_Technique.h"
#include "Modules/Graphics/Lighting/Indirect/Indirect_Technique.h"
#include "Modules/Graphics/Lighting/Reflector/Reflector_Technique.h"
#include "Modules/Graphics/Effects/Skybox.h"
#include "Modules/Graphics/Effects/SSAO.h"
#include "Modules/Graphics/Effects/SSR.h"
#include "Modules/Graphics/Effects/Join_Reflections.h"
#include "Modules/Graphics/Effects/Bloom.h"
#include "Modules/Graphics/Effects/HDR.h"
#include "Modules/Graphics/Effects/FXAA.h"
#include "Modules/Graphics/Logical/Transform_System.h"
#include "Modules/Graphics/Logical/CameraPerspective_System.h"
#include "Modules/Graphics/Logical/ReflectorPerspective_System.h"
#include "Modules/Graphics/Logical/ShadowPerspective_System.h"
#include "Modules/Graphics/Logical/FrustumCull_System.h"
#include "Modules/Graphics/Logical/SkeletalAnimation_System.h"


Graphics_Pipeline::Graphics_Pipeline(Engine* engine, const std::shared_ptr<Camera>& clientCamera)
	: m_engine(engine)
{
	// Camera
	m_sceneCameras = std::make_shared<std::vector<Camera*>>();

	// Create Systems
	m_transHierachy = m_worldSystems.makeSystem<Transform_System>(m_engine);
	m_worldSystems.makeSystem<FrustumCull_System>(m_sceneCameras);
	m_worldSystems.makeSystem<Skeletal_Animation_System>(m_engine);
	m_cameraSystems.makeSystem<CameraPerspective_System>(m_sceneCameras);
	m_cameraSystems.makeSystem<ShadowPerspective_System>(m_sceneCameras);
	m_cameraSystems.makeSystem<ReflectorPerspective_System>(m_sceneCameras);

	// Create Rendering Techniques
	auto propView = new Prop_Technique(engine, m_sceneCameras);
	auto shadowing = new Shadow_Technique(engine, m_sceneCameras);
	auto directLighting = new Direct_Technique(engine, shadowing->getShadowData(), clientCamera, m_sceneCameras);
	auto indirectLighting = new Indirect_Technique(engine, shadowing->getShadowData(), clientCamera, m_sceneCameras);
	auto reflectorLighting = new Reflector_Technique(engine, m_sceneCameras);
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
		shadowing, directLighting, indirectLighting, skybox, reflectorLighting,
	};
	m_effectTechniques = {
		ssao, ssr, joinReflections, bloom, hdr, fxaa,
	};
	m_allTechniques = {
		propView,
		shadowing, directLighting, indirectLighting, skybox, reflectorLighting,
		ssao, ssr, joinReflections, bloom, hdr, fxaa,
	};
}

std::vector<std::pair<int, int>> Graphics_Pipeline::begin(const float& deltaTime, ecsWorld& world, const std::vector<std::shared_ptr<Camera>>& cameras)
{
	// Add input cameras to shared list
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

	// Add cameras from world to shared list
	world.updateSystems(m_cameraSystems, deltaTime);

	// Update world systems
	std::dynamic_pointer_cast<Transform_System>(m_transHierachy)->m_world = &world;
	world.updateSystems(m_worldSystems, deltaTime);

	// Update rendering techniques
	for each (auto * tech in m_allTechniques)
		tech->updateCache(deltaTime, world);

	// Write camera data to camera GPU buffer
	m_cameraBuffer.beginWriting();
	m_cameraBuffer.resize(m_sceneCameras->size());
	for (size_t x = 0ull; x < m_sceneCameras->size(); ++x)
		m_cameraBuffer[x] = *(*m_sceneCameras)[x]->get();
	m_cameraBuffer.endWriting();

	// Apply pre-rendering passes
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for each (auto * tech in m_allTechniques)
		tech->updatePass(deltaTime);

	return perspectives;
}

void Graphics_Pipeline::end(const float& deltaTime)
{
	m_cameraBuffer.endReading();
	for each (auto * tech in m_allTechniques)
		tech->clearCache(deltaTime);
}

void Graphics_Pipeline::render(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives, const unsigned int& allowedCategories)
{
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for each (auto * tech in m_allTechniques)
		if (allowedCategories & tech->getCategory())
			tech->renderTechnique(deltaTime, viewport, perspectives);
}

void Graphics_Pipeline::cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives)
{
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for each (auto * tech in m_geometryTechniques)
		tech->cullShadows(deltaTime, perspectives);
}

void Graphics_Pipeline::renderShadows(const float& deltaTime)
{
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for each (auto * tech in m_geometryTechniques)
		tech->renderShadows(deltaTime);
}