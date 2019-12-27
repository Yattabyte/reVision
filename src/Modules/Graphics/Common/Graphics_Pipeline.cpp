#include "Modules/Graphics/Common/Graphics_Pipeline.h"

/* Rendering Techniques Used */
#include "Modules/Graphics/Logical/Transform_System.h"
#include "Modules/Graphics/Logical/CameraPerspective_System.h"
#include "Modules/Graphics/Logical/ReflectorPerspective_System.h"
#include "Modules/Graphics/Logical/ShadowPerspective_System.h"
#include "Modules/Graphics/Logical/FrustumCull_System.h"
#include "Modules/Graphics/Logical/SkeletalAnimation_System.h"


Graphics_Pipeline::Graphics_Pipeline(Engine& engine, Camera& clientCamera) :
	m_engine(engine),
	m_propView(engine, m_sceneCameras),
	m_shadowing(engine, m_sceneCameras),
	m_directLighting(engine, m_shadowing.getShadowData(), clientCamera, m_sceneCameras),
	m_indirectLighting(engine, m_shadowing.getShadowData(), clientCamera, m_sceneCameras),
	m_reflectorLighting(engine, m_sceneCameras),
	m_skybox(engine),
	m_ssao(engine),
	m_ssr(engine),
	m_joinReflections(engine),
	m_bloom(engine),
	m_hdr(engine),
	m_fxaa(engine),
	m_transHierachy(std::make_shared<Transform_System>(engine))
{
	// Create Systems
	m_worldSystems.addSystem(m_transHierachy);
	m_worldSystems.makeSystem<FrustumCull_System>(m_sceneCameras);
	m_worldSystems.makeSystem<Skeletal_Animation_System>();
	m_cameraSystems.makeSystem<CameraPerspective_System>(m_sceneCameras);
	m_cameraSystems.makeSystem<ShadowPerspective_System>(m_sceneCameras);
	m_cameraSystems.makeSystem<ReflectorPerspective_System>(m_sceneCameras);
}

std::vector<std::pair<int, int>> Graphics_Pipeline::begin(const float& deltaTime, ecsWorld& world, std::vector<Camera>& cameras)
{
	// Add input cameras to shared list
	m_sceneCameras.clear();
	m_sceneCameras.reserve(cameras.size());
	std::vector<std::pair<int, int>> perspectives;
	int count(0);
	for (auto& camera : cameras) {
		camera.updateFrustum();
		m_sceneCameras.push_back(&camera);
		perspectives.emplace_back( count, count );
		count++;
	}

	// Add cameras from world to shared list
	world.updateSystems(m_cameraSystems, deltaTime);

	// Update world systems
	std::dynamic_pointer_cast<Transform_System>(m_transHierachy)->m_world = &world;
	world.updateSystems(m_worldSystems, deltaTime);

	// Update rendering techniques
	for (auto& tech : m_allTechniques)
		tech->updateCache(deltaTime, world);

	// Write camera data to camera GPU buffer
	m_cameraBuffer.beginWriting();
	m_cameraBuffer.resize(m_sceneCameras.size());
	const auto sceneCameraCount = m_sceneCameras.size();
	for (size_t x = 0ULL; x < sceneCameraCount; ++x)
		m_cameraBuffer[x] = *m_sceneCameras[x]->get();
	m_cameraBuffer.endWriting();

	// Apply pre-rendering passes
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for (auto& tech : m_allTechniques)
		tech->updatePass(deltaTime);

	return perspectives;
}

void Graphics_Pipeline::end(const float& deltaTime)
{
	m_cameraBuffer.endReading();
	for (auto& tech : m_allTechniques)
		tech->clearCache(deltaTime);
}

void Graphics_Pipeline::render(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives, const unsigned int& categories)
{
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for (auto& tech : m_allTechniques)
		if ((categories & static_cast<unsigned int>(tech->getCategory())) != 0u)
			tech->renderTechnique(deltaTime, viewport, perspectives);
}

void Graphics_Pipeline::cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives)
{
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for (auto& tech : m_geometryTechniques)
		tech->cullShadows(deltaTime, perspectives);
}

void Graphics_Pipeline::renderShadows(const float& deltaTime)
{
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for (auto& tech : m_geometryTechniques)
		tech->renderShadows(deltaTime);
}