#include "Systems\Graphics\Resources\Geometry Techniques\Model_Technique.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\World\ECS\Components\Anim_Model_Component.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\Camera.h"


Model_Technique::~Model_Technique()
{
	m_shapeCube->removeCallback(this);
}

Model_Technique::Model_Technique(Geometry_FBO * geometryFBO, VectorBuffer<Geometry_Dynamic_Struct> * geometrySSBO)
{
	// FBO's
	m_geometryFBO = geometryFBO;
	m_geometryDynamicSSBO = geometrySSBO;
	
	// Asset Loading
	Asset_Loader::load_asset(m_shaderCull, "Geometry\\culling");
	Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");

	// Cube Loading
	Asset_Loader::load_asset(m_shapeCube, "box");
	m_cubeVAOLoaded = false;
	m_cubeVAO = Asset_Primitive::Generate_VAO();
	m_shapeCube->addCallback(this, [&]() {
		m_shapeCube->updateVAO(m_cubeVAO);
		m_cubeVAOLoaded = true;
	});
}

void Model_Technique::updateData(const vector<Camera*> & viewers)
{
}

void Model_Technique::renderGeometry(Camera & camera)
{
	const size_t size = camera.getVisibilityToken().specificSize("Anim_Model");
	if (size && m_shaderGeometry->existsYet()) {
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		m_geometryFBO->bindForWriting();
		m_shaderGeometry->bind();
		m_geometryDynamicSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		glBindVertexArray(Asset_Manager::Get_Model_Manager()->getVAO());

		// Render only the objects that passed the previous depth test (modified indirect draw buffer)     
		const auto &visBuffers = camera.getVisibilityBuffers();		
		visBuffers.m_buffer_Index[CAM_GEOMETRY_DYNAMIC].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		visBuffers.m_buffer_Render[CAM_GEOMETRY_DYNAMIC].bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);
	}
}

void Model_Technique::occlusionCullBuffers(Camera & camera)
{
	const size_t size = camera.getVisibilityToken().specificSize("Anim_Model");
	if (m_shaderCull->existsYet() && m_cubeVAOLoaded && size) {
		// Set up state
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		m_geometryFBO->bindDepthWriting();
		m_geometryDynamicSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);

		// Render bounding boxes for all models using last frame's depth buffer
		glBindVertexArray(m_cubeVAO);
		m_shaderCull->bind();
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		// Draw bounding boxes for each camera
		// Main geometry UBO bound to '5', the buffer below indexes into that
		const auto &visBuffers = camera.getVisibilityBuffers();
		visBuffers.m_buffer_Index[CAM_GEOMETRY_DYNAMIC].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		visBuffers.m_buffer_Culling[CAM_GEOMETRY_DYNAMIC].bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		visBuffers.m_buffer_Render[CAM_GEOMETRY_DYNAMIC].bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

		// Undo state changes
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
}

void Model_Technique::writeCameraBuffers(Camera & camera, const unsigned int & instanceCount)
{
	const Visibility_Token vis_token = camera.getVisibilityToken();
	const size_t size = vis_token.specificSize("Anim_Model");
	if (size) {
		// Draw parameter order = { COUNT, INSTANCE_COUNT, FIRST, BASE_INSTANCE }
		vector<ivec4> cullingDrawData(size);
		vector<ivec4> renderingDrawData(size);
		vector<uint> visibleIndices(size);

		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Anim_Model_Component>("Anim_Model")) {
			const ivec2 drawInfo = component->getDrawInfo();
			visibleIndices[count] = component->getBufferIndex();
			// Check mesh complexity and if viewer not within BSphere
			if ((component->getMeshSize() >= 100) && !(component->containsPoint(camera.getPosition()))) { // Allow
				cullingDrawData[count] = ivec4(36, instanceCount, 0, 1);
				renderingDrawData[count++] = ivec4(drawInfo.y, 0, drawInfo.x, 1);
			}
			else { // Skip				
				cullingDrawData[count] = ivec4(36, 0, 0, 1);
				renderingDrawData[count++] = ivec4(drawInfo.y, instanceCount, drawInfo.x, 1);
			}
		}

		auto &visBuffers = camera.getVisibilityBuffers();
		visBuffers.m_buffer_Index[CAM_GEOMETRY_DYNAMIC].write(0, sizeof(uint) * size, visibleIndices.data());
		visBuffers.m_buffer_Culling[CAM_GEOMETRY_DYNAMIC].write(0, sizeof(ivec4) * size, cullingDrawData.data());
		visBuffers.m_buffer_Render[CAM_GEOMETRY_DYNAMIC].write(0, sizeof(ivec4) * size, renderingDrawData.data());
	}
}
