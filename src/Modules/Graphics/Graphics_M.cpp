#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/World/World_M.h"
#include "Engine.h"
#include <memory>
#include <random>

/* Component Types Used */
#include "Modules/Graphics/Components/Prop_C.h"
#include "Modules/Graphics/Components/Skeleton_C.h"
#include "Modules/Graphics/Components/LightDirectional_C.h"
#include "Modules/Graphics/Components/LightPoint_C.h"
#include "Modules/Graphics/Components/LightSpot_C.h"
#include "Modules/Graphics/Components/Reflector_C.h"
#include "Modules/Graphics/Components/Skeleton_C.h"
#include "Modules/Graphics/Components/Camera_C.h"

/* System Types Used */
#include "Modules/Graphics/Systems/TransformSync_S.h"
#include "Modules/Graphics/Systems/SkeletonAnimation_S.h"
#include "Modules/Graphics/Systems/PropRendering_S.h"
#include "Modules/Graphics/Systems/LightDirectional_S.h"
#include "Modules/Graphics/Systems/LightSpot_S.h"
#include "Modules/Graphics/Systems/LightPoint_S.h"
#include "Modules/Graphics/Systems/Reflector_S.h"

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
}

void Graphics_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_ecs = &m_engine->getECS();
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
		m_defaultCamera->data->Dimensions = m_renderSize;
		updateCamera(m_defaultCamera->data);
	});
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		m_geometryFBO.resize(m_renderSize.x, m_renderSize.y);
		m_lightingFBO.resize(m_renderSize.x, m_renderSize.y);
		m_reflectionFBO.resize(m_renderSize.x, m_renderSize.y);
		m_defaultCamera->data->Dimensions = m_renderSize;
		updateCamera(m_defaultCamera->data);
	});
	GLuint m_bounceSize = 16;
	preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_bounceSize);
	preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { m_bounceFBO.resize((GLuint)f); });
	m_bounceFBO.resize(m_bounceSize);
	float farPlane = 1000.0f;
	preferences.getOrSetValue(PreferenceState::C_DRAW_DISTANCE, farPlane);
	preferences.addCallback(PreferenceState::C_DRAW_DISTANCE, m_aliveIndicator, [&](const float &f) {
		m_defaultCamera->data->FarPlane = f;
		updateCamera(m_defaultCamera->data);
	});

	// Camera Setup
	m_cameraIndexBuffer = StaticBuffer(sizeof(GLuint));
	m_defaultCamera = m_cameraBuffer.newElement();
	m_defaultCamera->data->pMatrix = glm::mat4(1.0f);
	m_defaultCamera->data->pMatrix_Inverse = glm::inverse(glm::mat4(1.0f));
	m_defaultCamera->data->vMatrix = glm::mat4(1.0f);
	m_defaultCamera->data->vMatrix_Inverse = glm::inverse(glm::mat4(1.0f));
	m_defaultCamera->data->EyePosition = glm::vec3(0.0f);
	m_defaultCamera->data->Dimensions = m_renderSize;
	m_defaultCamera->data->FarPlane = farPlane;
	m_defaultCamera->data->FOV = 110.0f;
	updateCamera(m_defaultCamera->data);
	setActiveCamera(m_defaultCamera->index);

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
	GLint size = sizeof(Camera_Buffer), offsetAlignment = 0;
	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &offsetAlignment);
	m_cameraBuffer.setOffsetAlignment(size % offsetAlignment);
	m_volumeRH = std::shared_ptr<RH_Volume>(new RH_Volume(m_engine));

	// Graphics-related Component Updating
	auto * propRendering = new PropRendering_System(m_engine);
	auto * lightDirectional = new LightDirectional_System(m_engine);
	auto * lightPoint = new LightPoint_System();
	auto * lightSpot = new LightSpot_System();
	auto * reflector = new Reflector_System();
	m_renderingSystems.addSystem(new TransformSync_Gfx_System());
	m_renderingSystems.addSystem(new SkeletonAnimation_System());
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
	

	auto & world = m_engine->getModule_World();
	world.addLevelListener(&lightPoint->m_renderState.m_outOfDate);
	world.addLevelListener(&lightSpot->m_renderState.m_outOfDate);
	world.addLevelListener(&reflector->m_renderState.m_outOfDate);
	   
	// Component Constructors
	m_engine->registerECSConstructor("Prop_Component", new Prop_Constructor(m_engine, &propRenderingEffect->m_propBuffer));
	m_engine->registerECSConstructor("Skeleton_Component", new Skeleton_Constructor(m_engine, &propRenderingEffect->m_skeletonBuffer));
	m_engine->registerECSConstructor("LightDirectional_Component", new LightDirectional_Constructor(&lightDirectionalEffect->m_lightBuffer));
	m_engine->registerECSConstructor("LightDirectionalShadow_Component", new LightDirectionalShadow_Constructor(&lightDirectionalEffect->m_shadowBuffer, &lightDirectionalEffect->m_shadowFBO));
	m_engine->registerECSConstructor("LightPoint_Component", new LightPoint_Constructor(&lightPointEffect->m_lightBuffer));
	m_engine->registerECSConstructor("LightPointShadow_Component", new LightPointShadow_Constructor(&lightPointEffect->m_shadowBuffer, &lightPointEffect->m_shadowFBO));
	m_engine->registerECSConstructor("LightSpot_Component", new LightSpot_Constructor(&lightSpotEffect->m_lightBuffer));
	m_engine->registerECSConstructor("LightSpotShadow_Component", new LightSpotShadow_Constructor(&lightSpotEffect->m_shadowBuffer, &lightSpotEffect->m_shadowFBO));
	m_engine->registerECSConstructor("Reflector_Component", new Reflector_Constructor(&m_cameraBuffer, &reflectorEffect->m_reflectorBuffer, &reflectorEffect->m_envmapFBO));
}

void Graphics_Module::frameTick(const float & deltaTime)
{
	// Update rendering pipeline
	static GLsync fence = nullptr;
	if (fence) {
		GLenum state = GL_UNSIGNALED;
		while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED)
			state = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	}

	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	m_geometryFBO.clear();
	m_lightingFBO.clear();
	m_reflectionFBO.clear();
	m_bounceFBO.clear();
	m_engine->getManager_Materials().bind();
	m_cameraIndexBuffer.bindBufferBase(GL_UNIFORM_BUFFER, 1);	
	m_volumeRH->updateVolume(*getActiveCameraBuffer());
	m_ecs->updateSystems(m_renderingSystems, deltaTime);

	// Rendering
	for each (auto *tech in m_fxTechs)
		if (tech->isEnabled())
			tech->applyEffect(deltaTime);

	if (fence)
		glDeleteSync(fence);
	fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void Graphics_Module::updateCamera(Camera_Buffer * camera)
{
	// Update Perspective Matrix
	float ar = std::max(1.0f, camera->Dimensions.x) / std::max(1.0f, camera->Dimensions.y);
	float horizontalRad = glm::radians(camera->FOV);
	float verticalRad = 2.0f * atanf(tanf(horizontalRad / 2.0f) / ar);
	camera->pMatrix = glm::perspective(verticalRad, ar, CAMERA_NEAR_PLANE, camera->FarPlane);
	camera->pMatrix_Inverse = glm::inverse(camera->pMatrix);
	camera->vMatrix_Inverse = glm::inverse(camera->vMatrix);
}

void Graphics_Module::setActiveCamera(const GLuint & newCameraID)
{
	m_activeCamera = newCameraID;
	m_cameraIndexBuffer.write(0, sizeof(GLuint), &newCameraID);
	const GLint offsetAlignment = m_cameraBuffer.getOffsetAlignment();
	m_cameraBuffer.bindBufferBaseRange(GL_SHADER_STORAGE_BUFFER, 2, (sizeof(Camera_Buffer) + offsetAlignment) * newCameraID, sizeof(Camera_Buffer) + offsetAlignment);
}

const GLuint Graphics_Module::getActiveCamera() const
{
	return m_activeCamera;
}

VB_Element<Camera_Buffer>* Graphics_Module::getActiveCameraBuffer()
{
	return m_cameraBuffer.getElement(getActiveCamera());
}
