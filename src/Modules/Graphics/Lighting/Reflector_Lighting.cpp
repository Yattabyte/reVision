#include "Modules/Graphics/Lighting/Reflector_Lighting.h"
#include "Modules/World/ECS/TransformComponent.h"
#include "Engine.h"


Reflector_Lighting::~Reflector_Lighting() 
{
	// Update indicator
	m_aliveIndicator = false;
	auto & world = m_engine->getModule_World();
	world.removeNotifyOnComponentType("Reflector_Component", m_notifyReflector);
}

Reflector_Lighting::Reflector_Lighting(Engine * engine)
	: m_engine(engine)
{
	// Asset Loading
	m_shaderLighting = Shared_Shader(m_engine, "Core\\Reflector\\IBL_Parallax");
	m_shaderStencil = Shared_Shader(m_engine, "Core\\Reflector\\Stencil");
	m_shaderCopy = Shared_Shader(m_engine, "Core\\Reflector\\2D_To_Cubemap");
	m_shaderConvolute = Shared_Shader(m_engine, "Core\\Reflector\\Cube_Convolution");
	m_shapeCube = Shared_Primitive(m_engine, "cube");
	m_shapeQuad = Shared_Primitive(m_engine, "quad");

	// Preferences
	auto & preferences = m_engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_ENVMAP_SIZE, m_envmapSize);
	preferences.addCallback(PreferenceState::C_ENVMAP_SIZE, m_aliveIndicator, [&](const float &f) {
		m_envmapSize = std::max(1u, (unsigned int)f);
		m_envmapFBO.resize(m_envmapSize, m_envmapSize, 6);
		(*m_reflectorCamera)->Dimensions = glm::vec2(m_envmapSize);
		updateCamera();
	});

	// Camera Setup
	m_reflectorCamera = std::make_shared<CameraBuffer>();
	(*m_reflectorCamera)->pMatrix = glm::mat4(1.0f);
	(*m_reflectorCamera)->vMatrix = glm::mat4(1.0f);
	(*m_reflectorCamera)->EyePosition = glm::vec3(0.0f);
	(*m_reflectorCamera)->Dimensions = glm::vec2(m_envmapSize);
	(*m_reflectorCamera)->FarPlane = 100.0f;
	(*m_reflectorCamera)->FOV = 90.0f;
	updateCamera();

	// Environment Map
	m_envmapFBO.resize(m_envmapSize, m_envmapSize, 6);
	GLuint data[] = { 0,0,0,0 };
	m_indirectCube.write(0, sizeof(GLuint) * 4, &data);
	m_indirectQuad.write(0, sizeof(GLuint) * 4, &data);
	m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, &data);


	// Asset-Finished Callbacks		
	m_shapeCube->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint data = { (GLuint)m_shapeCube->getSize() };
		m_indirectCube.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
	});
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
		m_indirectQuad.write(0, sizeof(GLuint) * 4, quadData);
		const GLuint quad6Data[4] = { (GLuint)m_shapeQuad->getSize(), 6, 0, 0 };
		m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, quad6Data);
	});

	// Error Reporting
	if (glCheckNamedFramebufferStatus(m_envmapFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		m_engine->getManager_Messages().error("Reflector_Lighting Environment Map Framebuffer has encountered an error.");

	// Graphics Pipeline Initialization
	m_reflectorFBOS = std::make_shared<Graphics_Framebuffers>(engine);
	m_reflectorFBOS->createFBO("GEOMETRY", { { GL_RGB16F, GL_RGB, GL_FLOAT },{ GL_RGB16F, GL_RGB, GL_FLOAT },{ GL_RGBA16F, GL_RGBA, GL_FLOAT },{ GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 } });
	m_reflectorFBOS->createFBO("LIGHTING", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	m_reflectorFBOS->createFBO("REFLECTION", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	m_reflectorFBOS->createFBO("BOUNCE", { { GL_RGB16F, GL_RGB, GL_FLOAT } });
	glNamedFramebufferTexture(m_reflectorFBOS->m_fbos["LIGHTING"].first, GL_DEPTH_STENCIL_ATTACHMENT, std::get<0>(m_reflectorFBOS->m_fbos["GEOMETRY"].second.back()), 0);
	glNamedFramebufferTexture(m_reflectorFBOS->m_fbos["REFLECTION"].first, GL_DEPTH_STENCIL_ATTACHMENT, std::get<0>(m_reflectorFBOS->m_fbos["GEOMETRY"].second.back()), 0);
	m_reflectorVRH = std::shared_ptr<RH_Volume>(new RH_Volume(m_engine, m_reflectorCamera));

	// Declare component types used
	addComponentType(Reflector_Component::ID);
	addComponentType(Transform_Component::ID, FLAG_OPTIONAL);

	// Error Reporting
	if (!isValid())
		m_engine->getManager_Messages().error("Invalid ECS System: Reflector_Lighting");

	auto & world = m_engine->getModule_World();
	world.addLevelListener(&m_outOfDate);
	m_notifyReflector = world.addNotifyOnComponentType("Reflector_Component", [&](BaseECSComponent * c) {
		auto * component = (Reflector_Component*)c;
		auto envCount = (int)(m_reflectorBuffer.getCount() * 6);
		component->m_data = m_reflectorBuffer.newElement();
		component->m_data->data->CubeSpot = envCount;
		component->m_cubeSpot = envCount;
		m_envmapFBO.resize(m_envmapFBO.m_size.x, m_envmapFBO.m_size.y, envCount + 6);
		component->m_outOfDate = true;
		for (int x = 0; x < 6; ++x) {
			component->m_Cameradata[x].Dimensions = m_envmapFBO.m_size;
			component->m_Cameradata[x].FOV = 90.0f;
		}
	});
}

void Reflector_Lighting::applyTechnique(const float & deltaTime) 
{
	// Exit Early
	if (!m_enabled || !m_shapeCube->existsYet() || !m_shapeQuad->existsYet() || !m_shaderLighting->existsYet() || !m_shaderStencil->existsYet() || !m_shaderCopy->existsYet() || !m_shaderConvolute->existsYet())
		return;
	if (!m_renderingSelf) {
		m_renderingSelf = true;

		renderScene(deltaTime);
		renderReflectors(deltaTime);

		m_renderingSelf = false;
	}
}

void Reflector_Lighting::updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>>& components) 
{
	if (m_renderingSelf) return;
	// Accumulate Reflector Data		
	std::vector<GLint> reflectionIndicies;
	PriorityList<float, Reflector_Component*, std::less<float>> oldest;
	for each (const auto & componentParam in components) {
		Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[0];
		Transform_Component * transformComponent = (Transform_Component*)componentParam[1];

		// Sync Transform Attributes
		if (transformComponent) {
			const auto & position = transformComponent->m_transform.m_position;
			const auto & orientation = transformComponent->m_transform.m_orientation;
			const auto & scale = transformComponent->m_transform.m_scale;
			const auto & modelMatrix = transformComponent->m_transform.m_modelMatrix;
			const auto matRot = glm::mat4_cast(orientation);
			const float largest = pow(std::max(std::max(scale.x, scale.y), scale.z), 2.0f);
			reflectorComponent->m_transform = Transform(position, orientation, scale);
			reflectorComponent->m_data->data->mMatrix = modelMatrix;
			reflectorComponent->m_data->data->rotMatrix = glm::inverse(matRot);
			reflectorComponent->m_data->data->BoxCamPos = position;
			reflectorComponent->m_data->data->BoxScale = scale;
			const glm::mat4 vMatrices[6] = {
				glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
				glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
				glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
				glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
				glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
				glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
			};
			const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, largest);
			const glm::mat4 pMatrix_Inverse = glm::inverse(pMatrix);
			for (int x = 0; x < 6; ++x) {
				reflectorComponent->m_Cameradata[x].FarPlane = largest;
				reflectorComponent->m_Cameradata[x].EyePosition = position;
				reflectorComponent->m_Cameradata[x].pMatrix = pMatrix;
				reflectorComponent->m_Cameradata[x].vMatrix = vMatrices[x];
			}
		}

		// Update Buffer Attributes
		reflectionIndicies.push_back(reflectorComponent->m_data->index);
		oldest.insert(reflectorComponent->m_updateTime, reflectorComponent);
	}

	// Update Draw Buffers
	const size_t & refSize = reflectionIndicies.size();
	m_visLights.write(0, sizeof(GLuint) *refSize, reflectionIndicies.data());
	m_indirectCube.write(sizeof(GLuint), sizeof(GLuint), &refSize); // update primCount (2nd param)
	m_reflectorsToUpdate = PQtoVector(oldest);
}

void Reflector_Lighting::renderScene(const float & deltaTime)
{
	for each (auto reflector in std::vector<Reflector_Component*>(m_reflectorsToUpdate)) {
		if (reflector->m_outOfDate || m_outOfDate) {
			reflector->m_outOfDate = false;
			for (int x = 0; x < 6; ++x) {
				// Clear Frame Buffers
				glViewport(0, 0, m_envmapSize, m_envmapSize);
				m_reflectorFBOS->clear();
				m_reflectorVRH->clear();

				// Repurpose camera's tripple buffer
				m_reflectorCamera->waitFrame(0);
				m_reflectorCamera->bind(2, 0);
				*(m_reflectorCamera.get())->get() = reflector->m_Cameradata[x];
				m_reflectorCamera->pushChanges(0);
				m_reflectorCamera->lockFrame(0);
				m_reflectorVRH->updateVolume();

				// Apply Graphics Pipeline
				m_engine->getModule_Graphics().render(deltaTime, m_reflectorCamera, m_reflectorFBOS, m_reflectorVRH);

				// Copy lighting frame into cube-face
				m_reflectorFBOS->bindForReading("LIGHTING", 0);
				m_envmapFBO.bindForWriting();
				m_shaderCopy->bind();
				m_shaderCopy->setUniform(0, x + reflector->m_cubeSpot);
				m_indirectQuad.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				glBindVertexArray(m_shapeQuad->m_vaoID);
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}

			// Once cubemap is generated, convolute it
			m_shaderConvolute->bind();
			m_shaderConvolute->setUniform(0, reflector->m_cubeSpot);
			m_envmapFBO.bindForReading();
			m_indirectQuad6Faces.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			for (unsigned int r = 1; r < 6; ++r) {
				// Ensure we are writing to MIP level r
				const unsigned int write_size = (unsigned int)std::max(1.0f, (floor(m_envmapSize / pow(2.0f, (float)r))));
				glViewport(0, 0, write_size, write_size);
				m_shaderConvolute->setUniform(1, (float)r / 5.0f);
				glNamedFramebufferTexture(m_envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_envmapFBO.m_textureID, r);

				// Ensure we are reading from MIP level r - 1
				glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, r - 1);
				glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, r - 1);

				// Convolute the 6 faces for this roughness level (RENDERS 6 TIMES)
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}

			// Reset texture, so it can be used for other component reflections
			glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, 0);
			glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, 5);
			glNamedFramebufferTexture(m_envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_envmapFBO.m_textureID, 0);
			reflector->m_updateTime = m_engine->getTime();
			Shader::Release();
		}
	}

	m_outOfDate = false;
	glViewport(0, 0, (*m_cameraBuffer)->Dimensions.x, (*m_cameraBuffer)->Dimensions.y);
}

void Reflector_Lighting::renderReflectors(const float & deltaTime) 
{
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	// Draw only into depth-stencil buffer
	m_shaderStencil->bind();										// Shader (reflector)
	m_gfxFBOS->bindForWriting("REFLECTION");						// Ensure writing to reflection FBO
	m_gfxFBOS->bindForReading("GEOMETRY", 0);						// Read from Geometry FBO
	glBindTextureUnit(4, m_envmapFBO.m_textureID);					// Reflection map (environment texture)
	m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
	m_reflectorBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);	// Reflection buffer
	m_indirectCube.bindBuffer(GL_DRAW_INDIRECT_BUFFER);				// Draw call buffer
	glBindVertexArray(m_shapeCube->m_vaoID);						// Quad VAO
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 0, 0);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glFrontFace(GL_CW);
	glDrawArraysIndirect(GL_TRIANGLES, 0);
	glFrontFace(GL_CCW);

	// Now draw into color buffers
	m_shaderLighting->bind();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glCullFace(GL_FRONT);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_STENCIL_TEST);
}

void Reflector_Lighting::updateCamera()
{
	// Update Perspective Matrix
	const float ar = std::max(1.0f, (*m_reflectorCamera)->Dimensions.x) / std::max(1.0f, (*m_reflectorCamera)->Dimensions.y);
	const float horizontalRad = glm::radians((*m_reflectorCamera)->FOV);
	const float verticalRad = 2.0f * atanf(tanf(horizontalRad / 2.0f) / ar);
	(*m_reflectorCamera)->pMatrix = glm::perspective(verticalRad, ar, CameraBuffer::BufferStructure::ConstNearPlane, (*m_reflectorCamera)->FarPlane);
}

std::vector<Reflector_Component*> Reflector_Lighting::PQtoVector(PriorityList<float, Reflector_Component*, std::less<float>> oldest) 
{
	PriorityList<float, Reflector_Component*, std::greater<float>> m_closest(2);
	std::vector<Reflector_Component*> outList;
	outList.reserve(2);

	for each (const auto &element in oldest.toList()) {
		if (outList.size() < 2)
			outList.push_back(element);
		else
			m_closest.insert(element->m_updateTime, element);
	}

	for each (const auto &element in m_closest.toList()) {
		if (outList.size() >= 2)
			break;
		outList.push_back(element);
	}

	return outList;
}