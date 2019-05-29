#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Graphics/ECS/components.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/World/World_M.h"
#include "Engine.h"
#include <memory>
#include <random>

/* System Types Used */
#include "Modules/Graphics/ECS/TransformSync_S.h"
#include "Modules/Graphics/ECS/SkeletonAnimation_S.h"
#include "Modules/Graphics/ECS/PropRendering_S.h"
#include "Modules/Graphics/ECS/LightDirectional_S.h"
#include "Modules/Graphics/ECS/LightSpot_S.h"
#include "Modules/Graphics/ECS/LightPoint_S.h"
#include "Modules/Graphics/ECS/Reflector_S.h"

/* Post Processing Techniques Used */
#include "Modules/Graphics/Effects/PropRendering_FX.h"
#include "Modules/Graphics/Effects/LightDirectional_FX.h"
#include "Modules/Graphics/Effects/LightPoint_FX.h"
#include "Modules/Graphics/Effects/LightSpot_FX.h"
#include "Modules/Graphics/Effects/Reflector_FX.h"
#include "Modules/Graphics/Effects/Skybox.h"
#include "Modules/Graphics/Effects/SSAO.h"
#include "Modules/Graphics/Effects/Radiance_Hints.h"
#include "Modules/Graphics/Effects/Join_Reflections.h"
#include "Modules/Graphics/Effects/SSR.h"


Graphics_Module::~Graphics_Module()
{
	// Update indicator
	m_aliveIndicator = false;
	auto & world = m_engine->getModule_World();
	world.removeComponentType("Transform_Component");
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
		m_geometryFBO.resize(m_renderSize.x, m_renderSize.y);
		m_lightingFBO.resize(m_renderSize.x, m_renderSize.y);
		m_reflectionFBO.resize(m_renderSize.x, m_renderSize.y);
		m_cameraBuffer->Dimensions = m_renderSize;
		updateCamera(m_cameraBuffer);
	});
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		m_geometryFBO.resize(m_renderSize.x, m_renderSize.y);
		m_lightingFBO.resize(m_renderSize.x, m_renderSize.y);
		m_reflectionFBO.resize(m_renderSize.x, m_renderSize.y);
		m_cameraBuffer->Dimensions = m_renderSize;
		updateCamera(m_cameraBuffer);
		
	});
	GLuint m_bounceSize = 16;
	preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
	preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_bounceFBO.resize((GLuint)f); });
	m_bounceFBO.resize(m_bounceSize);
	float farPlane = 1000.0f;
	preferences.getOrSetValue(PreferenceState::C_DRAW_DISTANCE, farPlane);
	preferences.addCallback(PreferenceState::C_DRAW_DISTANCE, m_aliveIndicator, [&](const float &f) {		
		m_cameraBuffer->FarPlane = f;
		updateCamera(m_cameraBuffer);
		
	});
	float fov = 90.0f;
	preferences.getOrSetValue(PreferenceState::C_FOV, fov);
	preferences.addCallback(PreferenceState::C_FOV, m_aliveIndicator, [&](const float &f) {
		m_cameraBuffer->FOV = f;
		updateCamera(m_cameraBuffer);		
	});

	// Camera Setup
	m_cameraBuffer->pMatrix = glm::mat4(1.0f);
	m_cameraBuffer->vMatrix = glm::mat4(1.0f);
	m_cameraBuffer->EyePosition = glm::vec3(0.0f);
	m_cameraBuffer->Dimensions = m_renderSize;
	m_cameraBuffer->FarPlane = farPlane;
	m_cameraBuffer->FOV = fov;
	updateCamera(m_cameraBuffer);		

	// Asset Loading
	m_shaderCull = Shared_Shader(m_engine, "Core\\Props\\culling");
	m_shaderGeometry = Shared_Shader(m_engine, "Core\\Props\\geometry");

	// Error Reporting
	auto & msgMgr = m_engine->getManager_Messages();
	if (glCheckNamedFramebufferStatus(m_geometryFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		msgMgr.error("Geometry Framebuffer has encountered an error.");
	if (glCheckNamedFramebufferStatus(m_lightingFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		msgMgr.error("Lighting Framebuffer has encountered an error.");
	if (glCheckNamedFramebufferStatus(m_reflectionFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		msgMgr.error("Reflection Framebuffer has encountered an error.");

	m_visualFX.initialize(m_engine);
	m_geometryFBO.resize(m_renderSize.x, m_renderSize.y);
	m_lightingFBO.resize(m_renderSize.x, m_renderSize.y);
	m_reflectionFBO.resize(m_renderSize.x, m_renderSize.y);
	m_lightingFBO.attachTexture(m_geometryFBO.m_textureIDS[3], GL_DEPTH_STENCIL_ATTACHMENT);
	m_reflectionFBO.attachTexture(m_geometryFBO.m_textureIDS[3], GL_DEPTH_STENCIL_ATTACHMENT);
	m_volumeRH = std::shared_ptr<RH_Volume>(new RH_Volume(m_engine));

	// Graphics-related Component Updating
	auto * propRendering = new PropRendering_System(m_engine);
	auto * lightDirectional = new LightDirectional_System(m_engine);
	auto * lightPoint = new LightPoint_System(m_engine);
	auto * lightSpot = new LightSpot_System(m_engine);
	auto * reflector = new Reflector_System(m_engine);
	m_renderingSystems.addSystem(new TransformSync_Gfx_System(m_engine));
	m_renderingSystems.addSystem(new SkeletonAnimation_System(m_engine));
	m_renderingSystems.addSystem(propRendering);
	m_renderingSystems.addSystem(lightDirectional);
	m_renderingSystems.addSystem(lightPoint);
	m_renderingSystems.addSystem(lightSpot);
	m_renderingSystems.addSystem(reflector);

	// Rendering Pipeline
	auto * propRenderingEffect = new PropRendering_Effect(m_engine, &m_geometryFBO, &propRendering->m_renderState, m_shaderCull, m_shaderGeometry);
	auto * lightDirectionalEffect = new LightDirectional_Effect(m_engine, &m_geometryFBO, &m_lightingFBO, &m_bounceFBO, &propRenderingEffect->m_propBuffer, &propRenderingEffect->m_skeletonBuffer, &lightDirectional->m_renderState, m_volumeRH);
	auto * lightPointEffect = new LightPoint_Effect(m_engine, &m_geometryFBO, &m_lightingFBO, &propRenderingEffect->m_propBuffer, &propRenderingEffect->m_skeletonBuffer, &lightPoint->m_renderState);
	auto * lightSpotEffect = new LightSpot_Effect(m_engine, &m_geometryFBO, &m_lightingFBO, &propRenderingEffect->m_propBuffer, &propRenderingEffect->m_skeletonBuffer, &lightSpot->m_renderState);
	auto * reflectorEffect = new Reflector_Effect(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO, &reflector->m_renderState);
	m_fxTechs.push_back(propRenderingEffect);
	m_fxTechs.push_back(lightDirectionalEffect);
	m_fxTechs.push_back(lightPointEffect);
	m_fxTechs.push_back(lightSpotEffect);
	m_fxTechs.push_back(reflectorEffect);
	m_fxTechs.push_back(new Skybox(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));
	m_fxTechs.push_back(new Radiance_Hints(m_engine, &m_geometryFBO, &m_bounceFBO, m_volumeRH));
	m_fxTechs.push_back(new SSAO(m_engine, &m_geometryFBO, &m_visualFX));
	m_fxTechs.push_back(new SSR(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));
	m_fxTechs.push_back(new Join_Reflections(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));	

	// World listeners, for when static build-once elements need rebuilt on world changes
	auto & world = m_engine->getModule_World();
	world.addLevelListener(&lightPoint->m_renderState.m_outOfDate);
	world.addLevelListener(&lightSpot->m_renderState.m_outOfDate);
	world.addLevelListener(&reflector->m_renderState.m_outOfDate);
	   
	// Component Constructors
	world.addComponentType("Transform_Component", [](const ParamList & parameters) { 
		const auto position = CastAny(parameters[0], glm::vec3(0.0f));
		const auto orientation = CastAny(parameters[1], glm::quat(1, 0, 0, 0));
		const auto scale = CastAny(parameters[2], glm::vec3(1.0f));

		auto * component = new Transform_Component();
		component->m_transform.m_position = position;
		component->m_transform.m_orientation = orientation;
		component->m_transform.m_scale = scale;
		component->m_transform.update();
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Prop_Component", [engine, propRenderingEffect](const ParamList & parameters) {
		auto directory = CastAny(parameters[0], std::string(""));
		auto material = CastAny(parameters[1], 0u);
		auto * component = new Prop_Component();
		component->m_data = propRenderingEffect->m_propBuffer.newElement();
		component->m_model = Shared_Model(engine, directory);
		component->m_data->data->materialID = material;
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Skeleton_Component", [engine, propRenderingEffect](const ParamList & parameters) {
		auto directory = CastAny(parameters[0], std::string(""));
		auto animation = CastAny(parameters[1], 0);
		auto * component = new Skeleton_Component();
		component->m_data = propRenderingEffect->m_skeletonBuffer.newElement();
		component->m_mesh = Shared_Mesh(engine, "\\Models\\" + directory);
		component->m_animation = animation;
		for (int x = 0; x < NUM_MAX_BONES; ++x)
			component->m_data->data->bones[x] = glm::mat4(1.0f);
		return std::make_pair(component->ID, component);
	});	
	world.addComponentType("LightDirectional_Component", [engine, lightDirectionalEffect](const ParamList & parameters) {
		auto color = CastAny(parameters[0], glm::vec3(1.0f));
		auto intensity = CastAny(parameters[1], 1.0f);
		auto * component = new LightDirectional_Component();
		component->m_data = lightDirectionalEffect->m_lightBuffer.newElement();
		component->m_data->data->LightColor = color;
		component->m_data->data->LightIntensity = intensity;
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightDirectionalShadow_Component", [engine, lightDirectionalEffect](const ParamList & parameters) {
		auto shadowSpot = (int)(lightDirectionalEffect->m_shadowBuffer.getCount() * 4);
		auto * component = new LightDirectionalShadow_Component();
		component->m_data = lightDirectionalEffect->m_shadowBuffer.newElement();
		component->m_data->data->Shadow_Spot = shadowSpot;
		component->m_updateTime = 0.0f;
		component->m_shadowSpot = shadowSpot;
		lightDirectionalEffect->m_shadowFBO.resize(lightDirectionalEffect->m_shadowFBO.m_size, shadowSpot + 4);
		// Default Values
		component->m_data->data->lightV = glm::mat4(1.0f);
		for (int x = 0; x < NUM_CASCADES; ++x) {
			component->m_data->data->lightVP[x] = glm::mat4(1.0f);
			component->m_data->data->inverseVP[x] = glm::inverse(glm::mat4(1.0f));
		}
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightPoint_Component", [engine, lightPointEffect](const ParamList & parameters) {
		auto color = CastAny(parameters[0], glm::vec3(1.0f));
		auto intensity = CastAny(parameters[1], 1.0f);
		auto radius = CastAny(parameters[2], 1.0f);
		auto * component = new LightPoint_Component();
		component->m_data = lightPointEffect->m_lightBuffer.newElement();
		component->m_data->data->LightColor = color;
		component->m_data->data->LightIntensity = intensity;
		component->m_data->data->LightRadius = radius;
		component->m_radius = radius;
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightPointShadow_Component", [engine, lightPointEffect](const ParamList & parameters) {
		auto shadowSpot = (int)(lightPointEffect->m_shadowBuffer.getCount() * 12);
		auto * component = new LightPointShadow_Component();
		component->m_radius = CastAny(parameters[0], 1.0f);
		component->m_data = lightPointEffect->m_shadowBuffer.newElement();
		component->m_data->data->Shadow_Spot = shadowSpot;
		component->m_updateTime = 0.0f;
		component->m_shadowSpot = shadowSpot;
		component->m_outOfDate = true;
		lightPointEffect->m_shadowFBO.resize(lightPointEffect->m_shadowFBO.m_size, shadowSpot + 12);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightSpot_Component", [engine, lightSpotEffect](const ParamList & parameters) {
		auto color = CastAny(parameters[0], glm::vec3(1.0f));
		auto intensity = CastAny(parameters[1], 1.0f);
		auto radius = CastAny(parameters[2], 1.0f);
		auto cutoff = CastAny(parameters[3], 45.0f);
		auto * component = new LightSpot_Component();
		component->m_data = lightSpotEffect->m_lightBuffer.newElement();
		component->m_data->data->LightColor = color;
		component->m_data->data->LightIntensity = intensity;
		component->m_data->data->LightRadius = radius;
		component->m_data->data->LightCutoff = cosf(glm::radians(cutoff));
		component->m_radius = radius;
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("LightSpotShadow_Component", [engine, lightSpotEffect](const ParamList & parameters) {
		auto shadowSpot = (int)(lightSpotEffect->m_shadowBuffer.getCount() * 2);
		auto * component = new LightSpotShadow_Component();
		component->m_radius = CastAny(parameters[0], 1.0f);
		component->m_cutoff = CastAny(parameters[1], 45.0f);
		component->m_data = lightSpotEffect->m_shadowBuffer.newElement();
		component->m_data->data->Shadow_Spot = shadowSpot;
		component->m_updateTime = 0.0f;
		component->m_shadowSpot = shadowSpot;
		component->m_outOfDate = true;
		lightSpotEffect->m_shadowFBO.resize(lightSpotEffect->m_shadowFBO.m_size, shadowSpot + 2);
		return std::make_pair(component->ID, component);
	});
	world.addComponentType("Reflector_Component", [engine, reflectorEffect](const ParamList & parameters) {
		auto envCount = (int)(reflectorEffect->m_reflectorBuffer.getCount() * 6);
		auto * component = new Reflector_Component();
		component->m_data = reflectorEffect->m_reflectorBuffer.newElement();
		component->m_data->data->CubeSpot = envCount;
		component->m_cubeSpot = envCount;
		reflectorEffect->m_envmapFBO.resize(reflectorEffect->m_envmapFBO.m_size.x, reflectorEffect->m_envmapFBO.m_size.y, envCount + 6);
		component->m_outOfDate = true;
		for (int x = 0; x < 6; ++x) {
			component->m_Cameradata[x].Dimensions = reflectorEffect->m_envmapFBO.m_size;
			component->m_Cameradata[x].FOV = 90.0f;
		}
		return std::make_pair(component->ID, component);
	});
}

void Graphics_Module::frameTick(const float & deltaTime)
{
	// Wait on tripple-buffered camera buffer lock
	m_cameraBuffer.waitFrame(m_engine->getCurrentFrame());

	// Bind the camera buffer, commit changes, clear buffers
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	m_cameraBuffer.bind(2, m_engine->getCurrentFrame());
	m_cameraBuffer.pushChanges(m_engine->getCurrentFrame());
	m_geometryFBO.clear();
	m_lightingFBO.clear();
	m_reflectionFBO.clear();
	m_bounceFBO.clear();
	m_engine->getManager_Materials().bind();
	m_volumeRH->updateVolume(m_cameraBuffer);
	m_engine->getModule_World().updateSystems(m_renderingSystems, deltaTime);

	// Rendering
	for each (auto *tech in m_fxTechs)
		if (tech->isEnabled())
			tech->applyEffect(deltaTime);

	// Set lock for 3 frames from now
	m_cameraBuffer.lockFrame(m_engine->getCurrentFrame());
}

void Graphics_Module::updateCamera(CameraBuffer & cameraBuffer)
{
	// Update Perspective Matrix
	const float ar = std::max(1.0f, cameraBuffer->Dimensions.x) / std::max(1.0f, cameraBuffer->Dimensions.y);
	const float horizontalRad = glm::radians(cameraBuffer->FOV);
	const float verticalRad = 2.0f * atanf(tanf(horizontalRad / 2.0f) / ar);
	cameraBuffer->pMatrix = glm::perspective(verticalRad, ar, CameraBuffer::BufferStructure::CAMERA_NEAR_PLANE, cameraBuffer->FarPlane);
}

CameraBuffer & Graphics_Module::getCameraBuffer()
{
	return m_cameraBuffer;
}
