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


Graphics_Pipeline::Graphics_Pipeline(Engine& engine, Camera& clientCamera) noexcept :
	m_engine(engine)
{
	// Create Systems
	m_transHierachy = std::make_shared<Transform_System>(engine);
	m_worldSystems.addSystem(m_transHierachy);
	m_worldSystems.makeSystem<FrustumCull_System>(m_sceneCameras);
	m_worldSystems.makeSystem<Skeletal_Animation_System>(engine);
	m_cameraSystems.makeSystem<CameraPerspective_System>(m_sceneCameras);
	m_cameraSystems.makeSystem<ShadowPerspective_System>(m_sceneCameras);
	m_cameraSystems.makeSystem<ReflectorPerspective_System>(m_sceneCameras);

	// Create Rendering Techniques
	auto propView = new Prop_Technique(engine, m_sceneCameras);
	auto shadowing = new Shadow_Technique(engine, m_sceneCameras);
	auto directLighting = new Direct_Technique(engine, shadowing->getShadowData(), clientCamera, m_sceneCameras);
	auto indirectLighting = new Indirect_Technique(engine, shadowing->getShadowData(), clientCamera, m_sceneCameras);
	auto reflectorLighting = new Reflector_Technique(engine, m_sceneCameras);
	auto skybox = new Skybox(engine);
	auto ssao = new SSAO(engine);
	auto ssr = new SSR(engine);
	auto joinReflections = new Join_Reflections(engine);
	auto bloom = new Bloom(engine);
	auto hdr = new HDR(engine);
	auto fxaa = new FXAA(engine);

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

std::vector<std::pair<int, int>> Graphics_Pipeline::begin(const float& deltaTime, ecsWorld& world, std::vector<Camera>& cameras) noexcept
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
	for (auto* tech : m_allTechniques)
		tech->updateCache(deltaTime, world);

	// Write camera data to camera GPU buffer
	m_cameraBuffer.beginWriting();
	m_cameraBuffer.resize(m_sceneCameras.size());
	for (size_t x = 0ULL; x < m_sceneCameras.size(); ++x)
		m_cameraBuffer[x] = *m_sceneCameras[x]->get();
	m_cameraBuffer.endWriting();

	// Apply pre-rendering passes
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for (auto* tech : m_allTechniques)
		tech->updatePass(deltaTime);

	return perspectives;
}

void Graphics_Pipeline::end(const float& deltaTime) noexcept
{
	m_cameraBuffer.endReading();
	for (auto* tech : m_allTechniques)
		tech->clearCache(deltaTime);
}

void Graphics_Pipeline::render(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives, const unsigned int& categories) noexcept
{
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for (auto* tech : m_allTechniques)
		if (((unsigned int)categories & (unsigned int)tech->getCategory()) != 0u)
			tech->renderTechnique(deltaTime, viewport, perspectives);
}

void Graphics_Pipeline::cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives) noexcept
{
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for (auto* tech : m_geometryTechniques)
		tech->cullShadows(deltaTime, perspectives);
}

void Graphics_Pipeline::renderShadows(const float& deltaTime) noexcept
{
	m_cameraBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	for (auto* tech : m_geometryTechniques)
		tech->renderShadows(deltaTime);
}