#include "Modules\Graphics\Graphics_M.h"
#include "Modules\World\World_M.h"
#include "Engine.h"
#include <random>

/* System Types Used */
#include "ECS\Systems\PlayerMovement_S.h"
#include "ECS\Systems\PropRendering_S.h"
#include "ECS\Systems\PropBSphere_S.h"
#include "ECS\Systems\SkeletonAnimation_S.h"
#include "ECS\Systems\LightingDirectional_S.h"
#include "ECS\Systems\LightingSpot_S.h"
#include "ECS\Systems\LightingPoint_S.h"
#include "ECS\Systems\Reflector_S.h"

/* Post Processing Techniques Used */
#include "Modules\Graphics\Resources\Effects\Skybox.h"
#include "Modules\Graphics\Resources\Effects\SSAO.h"
#include "Modules\Graphics\Resources\Effects\Radiance_Hints.h"
#include "Modules\Graphics\Resources\Effects\Join_Reflections.h"
#include "Modules\Graphics\Resources\Effects\SSR.h"
#include "Modules\Graphics\Resources\Effects\Bloom.h"
#include "Modules\Graphics\Resources\Effects\HDR.h"
#include "Modules\Graphics\Resources\Effects\FXAA.h"
#include "Modules\Graphics\Resources\Effects\To_Screen.h"
#include "Modules\Graphics\Resources\Effects\Frametime_Counter.h"


Graphics_Module::~Graphics_Module()
{
	m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);	
	m_engine->removePrefCallback(PreferenceState::C_GAMMA, this);
	m_engine->removePrefCallback(PreferenceState::C_DRAW_DISTANCE, this);
}

Graphics_Module::Graphics_Module(Engine * engine) 
	: Engine_Module(engine), m_ecs(&m_engine->getECS()) {
	// GL settings
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Preferences loading
	m_renderSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {
		m_renderSize = glm::ivec2(f, m_renderSize.y);
		m_geometryFBO.resize(m_renderSize.x, m_renderSize.y);
		m_lightingFBO.resize(m_renderSize.x, m_renderSize.y);
		m_reflectionFBO.resize(m_renderSize.x, m_renderSize.y);
		m_defaultCamera->data->Dimensions = m_renderSize;
		updateCamera(m_defaultCamera->data);
	});
	m_renderSize.y = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {
		m_renderSize = glm::ivec2(m_renderSize.x, f);
		m_geometryFBO.resize(m_renderSize.x, m_renderSize.y);
		m_lightingFBO.resize(m_renderSize.x, m_renderSize.y);
		m_reflectionFBO.resize(m_renderSize.x, m_renderSize.y);
		m_defaultCamera->data->Dimensions = m_renderSize;
		updateCamera(m_defaultCamera->data);
	});
	const GLuint m_bounceSize = m_engine->addPrefCallback<GLuint>(PreferenceState::C_RH_BOUNCE_SIZE, this, [&](const float &f) { m_bounceFBO.resize((GLuint)f); });
	m_bounceFBO.resize(m_bounceSize);
	const float farPlane = m_engine->addPrefCallback<float>(PreferenceState::C_DRAW_DISTANCE, this, [&](const float &f) {
		m_defaultCamera->data->FarPlane = f;
		updateCamera(m_defaultCamera->data);
	});
	const float gamma = m_engine->addPrefCallback<float>(PreferenceState::C_GAMMA, this, [&](const float &f) {
		m_defaultCamera->data->Gamma = f;
		m_cameraBuffer.getElement(m_activeCamera)->data->Gamma = f;
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
	m_defaultCamera->data->NearPlane = 0.01f;
	m_defaultCamera->data->FarPlane = farPlane;
	m_defaultCamera->data->FOV = 110.0f;
	m_defaultCamera->data->Gamma = gamma;
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

	// Rendering Pipeline Generation
	// Logical Systems
	addSystem(new PlayerMovement_System(m_engine));
	addSystem(new PropBSphere_System());
	addSystem(new SkeletonAnimation_System());
	// Geometry Rendering
	addSystem(new PropRendering_System(m_engine, &m_geometryFBO, m_shaderCull, m_shaderGeometry));
	// Light Rendering
	addSystem(new LightingDirectional_System(m_engine, &m_geometryFBO, &m_lightingFBO, &m_bounceFBO, &getSystem<PropRendering_System>()->m_propBuffer, &getSystem<PropRendering_System>()->m_skeletonBuffer));
	addSystem(new LightingSpot_System(m_engine, &m_geometryFBO, &m_lightingFBO, &getSystem<PropRendering_System>()->m_propBuffer, &getSystem<PropRendering_System>()->m_skeletonBuffer));
	addSystem(new LightingPoint_System(m_engine, &m_geometryFBO, &m_lightingFBO, &getSystem<PropRendering_System>()->m_propBuffer, &getSystem<PropRendering_System>()->m_skeletonBuffer));
	addSystem(new Reflector_System(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));
	// Initiate specialized effects techniques
	m_fxTechs.push_back(new Skybox(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));
	m_fxTechs.push_back(new Radiance_Hints(m_engine, &m_geometryFBO, &m_bounceFBO));
	m_fxTechs.push_back(new SSAO(m_engine, &m_geometryFBO, &m_visualFX));
	m_fxTechs.push_back(new Join_Reflections(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));
	m_fxTechs.push_back(new SSR(m_engine, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));
	m_fxTechs.push_back(new Bloom(m_engine, &m_lightingFBO, &m_visualFX));
	m_fxTechs.push_back(new HDR(m_engine, &m_lightingFBO));
	m_fxTechs.push_back(new FXAA(m_engine));
	m_fxTechs.push_back(new To_Screen(m_engine));
	m_fxTechs.push_back(new Frametime_Counter(m_engine));

	auto & world = m_engine->getWorldModule();
	world.addLevelListener(&((getSystem<LightingSpot_System>())->outOfDate()));
	world.addLevelListener(&((getSystem<LightingPoint_System>())->outOfDate()));
	world.addLevelListener(&((getSystem<Reflector_System>())->outOfDate()));
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
	m_cameraBuffer.getElement(getActiveCamera())->wait();
	m_ecs->updateSystems(m_renderingSystems, deltaTime);

	// Post Processing
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	m_lightingFBO.bindForReading();
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
	camera->pMatrix = glm::perspective(verticalRad, ar, camera->NearPlane, camera->FarPlane);
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
