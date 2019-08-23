#pragma once
#ifndef WIREFRAME_SYSTEM_H
#define WIREFRAME_SYSTEM_H 

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Assets/Mesh.h"
#include "Assets/Shader.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Engine.h"


/***/
class Wireframe_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~Wireframe_System() {
		// Update indicator
		*m_aliveIndicator = false;

		if (m_vboID != 0 && m_vaoID != 0) {
			glDeleteBuffers(1, &m_vboID);
			glDeleteVertexArrays(1, &m_vaoID);
		}
	}
	/***/
	inline Wireframe_System(Engine * engine) 
		: m_engine(engine) {
		// Declare component types used
		addComponentType(Transform_Component::ID);
		addComponentType(BoundingBox_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightSpot_Component::ID, FLAG_OPTIONAL);
		//addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);

		m_indirectGeometry = StaticBuffer(sizeof(glm::ivec4) * 3, 0, GL_DYNAMIC_STORAGE_BIT);

		// Assets
		m_shader = Shared_Shader(engine, "Editor//wireframe");
		m_sphere = Shared_Mesh(engine, "\\Models\\sphere.obj");
		m_cone = Shared_Mesh(engine, "\\Models\\cone.obj");
		m_cube = Shared_Mesh(engine, "\\Models\\cube.obj");

		// Asset-Finished Callbacks
		m_sphere->addCallback(m_aliveIndicator, [&]() mutable {
			prepareGeometry();
		});
		m_cone->addCallback(m_aliveIndicator, [&]() mutable {
			prepareGeometry();
		});
		m_cube->addCallback(m_aliveIndicator, [&]() mutable {
			prepareGeometry();
		});
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector< std::vector<BaseECSComponent*> >& components) override {
		if (m_shader->existsYet() && m_sphere->existsYet() && m_cone->existsYet() && m_cube->existsYet()) {
			const auto pMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->pMatrix;
			const auto vMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->vMatrix;
			std::vector<glm::mat4> sphereData, coneData, cubeData;
			glm::ivec4 drawData[3];
			for each (const auto & componentParam in components) {
				auto* trans = (Transform_Component*)componentParam[0];
				auto* bbox = (BoundingBox_Component*)componentParam[1];
				auto* bsphere = (BoundingSphere_Component*)componentParam[2];
				auto* spot = (LightSpot_Component*)componentParam[3];

				const auto transform = trans->m_worldTransform;
				if (bbox)
					cubeData.push_back(pMatrix * vMatrix * (transform * Transform(bbox->m_positionOffset, glm::quat(1.0f, 0, 0, 0), bbox->m_extent)).m_modelMatrix);													
				if (bsphere)
					if (spot)
						coneData.push_back(pMatrix * vMatrix * (transform * Transform(glm::vec3(0), glm::quat(1.0f, 0, 0, 0), glm::vec3(bsphere->m_radius))).m_modelMatrix);
					else
						sphereData.push_back(pMatrix * vMatrix * (transform * Transform(glm::vec3(0), glm::quat(1.0f, 0, 0, 0), glm::vec3(bsphere->m_radius))).m_modelMatrix);
			}
			// Spheres
			const auto sphereCount = (GLuint)sphereData.size();
			drawData[0] = { m_sphereSize, sphereCount, m_sphereOffset, 0 };
			// Cones
			const auto coneCount = (GLuint)coneData.size();
			drawData[1] = { m_coneSize, coneCount, m_coneOffset, sphereCount };
			// Cubes
			const auto cubeCount = (GLuint)cubeData.size();
			drawData[2]  = { m_cubeSize, cubeCount, m_cubeOffset, sphereCount + coneCount };
			
			// Write data
			m_ssbo.beginWriting();
			m_ssbo.write(0, sizeof(glm::mat4) * sphereCount, sphereData.data());
			m_ssbo.write(sizeof(glm::mat4) * sphereCount, sizeof(glm::mat4) * coneCount, coneData.data());
			m_ssbo.write(sizeof(glm::mat4) * (sphereCount + coneCount), sizeof(glm::mat4) * cubeCount, cubeData.data());
			m_indirectGeometry.write(0, sizeof(glm::ivec4) * 3, drawData);

			m_shader->bind();
			glBindVertexArray(m_vaoID);
			m_ssbo.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
			m_indirectGeometry.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glFrontFace(GL_CCW);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, (GLsizei)3, 0);

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			m_shader->Release();
			m_ssbo.endWriting();
		}
	}


private:
	// Private
	/***/
	void prepareGeometry() {
		if (m_sphere->existsYet() && m_cone->existsYet() && m_cube->existsYet()) {
			// Create VBO's
			const auto size = sizeof(glm::vec3) * (m_sphere->m_geometry.vertices.size() + m_cone->m_geometry.vertices.size() + m_cube->m_geometry.vertices.size());
			std::vector<glm::vec3> mergedData;
			mergedData.insert(mergedData.end(), m_sphere->m_geometry.vertices.begin(), m_sphere->m_geometry.vertices.end());
			mergedData.insert(mergedData.end(), m_cone->m_geometry.vertices.begin(), m_cone->m_geometry.vertices.end());
			mergedData.insert(mergedData.end(), m_cube->m_geometry.vertices.begin(), m_cube->m_geometry.vertices.end());
			glCreateBuffers(1, &m_vboID);
			glNamedBufferStorage(m_vboID, size, &mergedData[0], GL_CLIENT_STORAGE_BIT);

			// Create VAO
			glCreateVertexArrays(1, &m_vaoID);
			glEnableVertexArrayAttrib(m_vaoID, 0);
			glVertexArrayAttribBinding(m_vaoID, 0, 0);
			glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));

			m_sphereSize = m_sphere->m_geometry.vertices.size();
			m_coneSize = m_cone->m_geometry.vertices.size();
			m_cubeSize = m_cube->m_geometry.vertices.size();
			m_sphereOffset = 0;
			m_coneOffset = m_sphereSize;
			m_cubeOffset = m_sphereSize + m_coneSize;
		}
	}


	// Private Attributes
	Engine* m_engine = nullptr;
	Shared_Shader m_shader;
	Shared_Mesh m_sphere, m_cone, m_cube;
	int m_sphereSize = 0, m_sphereOffset = 0, m_coneSize = 0, m_coneOffset = 0, m_cubeSize = 0, m_cubeOffset = 0;
	StaticBuffer m_indirectGeometry;
	DynamicBuffer m_ssbo;
	GLuint m_vaoID = 0, m_vboID = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WIREFRAME_SYSTEM_H