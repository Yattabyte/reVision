#include "Modules\Graphics\Graphics_M.h"
#include "Modules\Graphics\Common\RH_Volume.h"
#include "Modules\World\World_M.h"
#include "Engine.h"
#include <memory>
#include <random>

/* System Types Used */
#include "ECS\Systems\PlayerMovement_S.h"
#include "ECS\Systems\PropRendering_S.h"
#include "ECS\Systems\PropBSphere_S.h"
#include "ECS\Systems\SkeletonAnimation_S.h"
#include "ECS\Systems\LightDirectional_S.h"
#include "ECS\Systems\LightSpot_S.h"
#include "ECS\Systems\LightPoint_S.h"
#include "ECS\Systems\Reflector_S.h"

/* Post Processing Techniques Used */
#include "Modules\Graphics\Effects\PropRendering_FX.h"
#include "Modules\Graphics\Effects\LightDirectional_FX.h"
#include "Modules\Graphics\Effects\LightPoint_FX.h"
#include "Modules\Graphics\Effects\LightSpot_FX.h"
#include "Modules\Graphics\Effects\Reflector_FX.h"
#include "Modules\Graphics\Effects\Skybox.h"
#include "Modules\Graphics\Effects\SSAO.h"
#include "Modules\Graphics\Effects\Radiance_Hints.h"
#include "Modules\Graphics\Effects\Join_Reflections.h"
#include "Modules\Graphics\Effects\SSR.h"
#include "Modules\Graphics\Effects\Bloom.h"
#include "Modules\Graphics\Effects\HDR.h"
#include "Modules\Graphics\Effects\FXAA.h"
#include "Modules\Graphics\Effects\To_Screen.h"
#include "Modules\Graphics\Effects\Frametime_Counter.h"


Graphics_Module::Graphics_Module(Engine * engine) 
	: Engine_Module(engine), m_ecs(&m_engine->getECS()) {
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
	m_shaderCull = Asset_Shader::Create(m_engine, "Core\\Props\\culling");
	m_shaderGeometry = Asset_Shader::Create(m_engine, "Core\\Props\\geometry");

	// Error Reporting
	GLenum Status = glCheckNamedFramebufferStatus(m_geometryFBO.m_fboID, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Geometry FBO", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	Status = glCheckNamedFramebufferStatus(m_lightingFBO.m_fboID, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Lighting FBO", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	Status = glCheckNamedFramebufferStatus(m_reflectionFBO.m_fboID, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Reflection FBO", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
}

void Graphics_Module::initialize()
{
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
	addSystem(new PlayerMovement_System(m_engine));
	addSystem(new PropBSphere_System());
	addSystem(new SkeletonAnimation_System());
	addSystem(new PropRendering_System(m_engine));
	addSystem(new LightDirectional_System(m_engine));
	addSystem(new LightPoint_System(m_engine));
	addSystem(new LightSpot_System(m_engine));
	addSystem(new Reflector_System(m_engine));

	// Rendering Pipeline
	addEffect(new PropRendering_Effect(m_engine, &m_geometryFBO, &getSystem<PropRendering_System>()->m_renderState, m_shaderCull, m_shaderGeometry));
	addEffect(new LightDirectional_Effect(m_engine, &m_geometryFBO, &m_lightingFBO, &m_bounceFBO, &getEffect<PropRendering_Effect>()->m_propBuffer, &getEffect<PropRendering_Effect>()->m_skeletonBuffer, &getSystem<LightDirectional_System>()->m_renderState, m_volumeRH));
	addEffect(new LightPoint_Effect(m_engine, &m_geometryFBO, &m_lightingFBO, &getEffect<PropRendering_Effect>()->m_propBuffer, &getEffect<PropRendering_Effect>()->m_skeletonBuffer, &getSystem<LightPoint_System>()->m_renderState));
	addEffect(new LightSpot_Effect(m_engine, &m_geometryFBO, &m_lightingFBO, &getEffect<PropRendering_Effect>()->m_propBuffer, &getEffect<PropRendering_Effect>()->m_skeletonBuffer, &getSystem<LightSpot_System>()->m_renderState));
	addEffect(new Reflector_Effect(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO, &getSystem<Reflector_System>()->m_renderState));
	addEffect(new Skybox(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));
	addEffect(new Radiance_Hints(m_engine, &m_geometryFBO, &m_bounceFBO, m_volumeRH));
	addEffect(new SSAO(m_engine, &m_geometryFBO, &m_visualFX));
	addEffect(new Join_Reflections(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));
	addEffect(new SSR(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));
	addEffect(new Bloom(m_engine, &m_lightingFBO, &m_visualFX));
	addEffect(new HDR(m_engine, &m_lightingFBO));
	addEffect(new FXAA(m_engine));
	addEffect(new To_Screen(m_engine)); 
	addEffect(new Frametime_Counter(m_engine));

	auto & world = m_engine->getWorldModule();
	world.addLevelListener(&getSystem<LightSpot_System>()->m_renderState.m_outOfDate);
	world.addLevelListener(&getSystem<LightPoint_System>()->m_renderState.m_outOfDate);
	world.addLevelListener(&getSystem<Reflector_System>()->m_renderState.m_outOfDate);
}

void Graphics_Module::renderFrame(const float & deltaTime)
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
	m_cameraIndexBuffer.bindBufferBase(GL_UNIFORM_BUFFER, 1);	
	m_volumeRH->updateVolume(*m_cameraBuffer.getElement(getActiveCamera()));
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
