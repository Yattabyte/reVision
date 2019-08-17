#include "Modules/Editor/Gizmos/Scaling.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Engine.h"
#include "glm/gtx/intersect.hpp"


Scaling_Gizmo::~Scaling_Gizmo()
{
	// Update indicator
	*m_aliveIndicator = false;
}

Scaling_Gizmo::Scaling_Gizmo(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	// Update indicator
	*m_aliveIndicator = true;

	// Assets
	m_colorPalette = Shared_Texture(engine, "Editor\\colors.png");
	m_model = Shared_Auto_Model(engine, "Editor\\scale");
	m_gizmoShader = Shared_Shader(engine, "Editor\\gizmoShader");

	// Asset-Finished Callbacks
	m_model->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint data[4] = { (GLuint)m_model->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
		m_indicatorIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, data, GL_CLIENT_STORAGE_BIT);
		});

	auto& preferences = m_engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) {
		m_renderSize.x = (int)f;
		});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) {
		m_renderSize.y = (int)f;
		});
}

bool Scaling_Gizmo::checkMouseInput(const float& deltaTime)
{
	// See if the mouse intersects any entities.
	// In any case move the translation gizmo to where the mouse is.
	if (!ImGui::GetIO().WantCaptureMouse && ImGui::IsMouseDown(0)) {
		return rayCastMouse(deltaTime);
	}
	else {
		m_selectedAxes = NONE;
		// use difference in position for undo/redo
		// const auto deltaPos = m_position - m_startingPosition;
	}
	return false;
}

void Scaling_Gizmo::render(const float& deltaTime)
{
	// Safety check first
	if (m_model->existsYet() && m_colorPalette->existsYet() && m_gizmoShader->existsYet() && m_editor->getSelection().size()) {
		// Set up state
		m_editor->bindFBO();
		m_model->bind();
		m_colorPalette->bind(0);
		m_indicatorIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);

		// Flip the model's axes based on which side of it were on
		const auto& position = m_transform.m_position;
		const auto& camPos = m_editor->getCameraPosition();
		const auto dir = glm::normalize(camPos - position);
		const auto directions = glm::vec3(
			dir.x > 0 ? 1 : -1,
			dir.y > 0 ? 1 : -1,
			dir.z > 0 ? 1 : -1
		);

		// Generate matrices
		auto pMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->pMatrix;
		auto vMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->vMatrix;
		auto mMatrix = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), directions * glm::vec3(glm::distance(position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.02f));
		m_gizmoShader->setUniform(0, pMatrix * vMatrix * mMatrix);

		// Render
		m_gizmoShader->bind();
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Revert State
		m_gizmoShader->Release();
	}
}

void Scaling_Gizmo::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool Scaling_Gizmo::rayCastMouse(const float& deltaTime)
{
	const auto& actionState = m_engine->getActionState();
	const auto& position = m_transform.m_position;
	const auto& clientCamera = *m_engine->getModule_Graphics().getClientCamera()->get();
	const auto ray_origin = clientCamera.EyePosition;
	const auto ray_nds = glm::vec2(2.0f * actionState.at(ActionState::MOUSE_X) / m_renderSize.x - 1.0f, 1.0f - (2.0f * actionState.at(ActionState::MOUSE_Y)) / m_renderSize.y);
	const auto ray_eye = glm::vec4(glm::vec2(clientCamera.pMatrixInverse * glm::vec4(ray_nds, -1.0f, 1.0F)), -1.0f, 0.0f);
	const auto ray_world = glm::normalize(glm::vec3(clientCamera.vMatrixInverse * ray_eye));

	// Flip the model's axes based on which side of it were on
	const auto dir = glm::normalize(ray_origin - position);
	const auto direction = glm::vec3(
		dir.x > 0 ? 1 : -1,
		dir.y > 0 ? 1 : -1,
		dir.z > 0 ? 1 : -1
	);

	// Check if the user selected an axis
	if (m_selectedAxes == NONE) {
		const auto TestRayOBBIntersection = [](
			const glm::vec3& ray_origin,        // Ray origin, in world space
			const glm::vec3& ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
			const glm::vec3& aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
			const glm::vec3& aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
			const glm::mat4& ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
			float& intersection_distance		// Output : distance between ray_origin and the intersection with the OBB
			) -> bool {
				float tMin = 0.0f;
				float tMax = 100000.0f;
				glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);
				glm::vec3 delta = OBBposition_worldspace - ray_origin;

				// Test intersection with the 2 planes perpendicular to the OBB's X axis
				{
					glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
					float e = glm::dot(xaxis, delta);
					float f = glm::dot(ray_direction, xaxis);

					if (fabs(f) > 0.001f) { // Standard case
						float t1 = (e + aabb_min.x) / f; // Intersection with the "left" plane
						float t2 = (e + aabb_max.x) / f; // Intersection with the "right" plane
						// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

						// We want t1 to represent the nearest intersection, 
						// so if it's not the case, invert t1 and t2
						if (t1 > t2) {
							float w = t1;
							t1 = t2;
							t2 = w; // swap t1 and t2
						}

						// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
						if (t2 < tMax)
							tMax = t2;
						// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
						if (t1 > tMin)
							tMin = t1;

						// And here's the trick :
						// If "far" is closer than "near", then there is NO intersection.
						// See the images in the tutorials for the visual explanation.
						if (tMax < tMin)
							return false;
					}
					else // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
						if (-e + aabb_min.x > 0.0f || -e + aabb_max.x < 0.0f)
							return false;
				}


				// Test intersection with the 2 planes perpendicular to the OBB's Y axis
				// Exactly the same thing than above.
				{
					glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
					float e = glm::dot(yaxis, delta);
					float f = glm::dot(ray_direction, yaxis);

					if (fabs(f) > 0.001f) {
						float t1 = (e + aabb_min.y) / f;
						float t2 = (e + aabb_max.y) / f;

						if (t1 > t2) {
							float w = t1;
							t1 = t2;
							t2 = w;
						}

						if (t2 < tMax)
							tMax = t2;
						if (t1 > tMin)
							tMin = t1;
						if (tMin > tMax)
							return false;
					}
					else
						if (-e + aabb_min.y > 0.0f || -e + aabb_max.y < 0.0f)
							return false;
				}


				// Test intersection with the 2 planes perpendicular to the OBB's Z axis
				// Exactly the same thing than above.
				{
					glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
					float e = glm::dot(zaxis, delta);
					float f = glm::dot(ray_direction, zaxis);

					if (fabs(f) > 0.001f) {
						float t1 = (e + aabb_min.z) / f;
						float t2 = (e + aabb_max.z) / f;

						if (t1 > t2) {
							float w = t1;
							t1 = t2;
							t2 = w;
						}

						if (t2 < tMax)
							tMax = t2;
						if (t1 > tMin)
							tMin = t1;
						if (tMin > tMax)
							return false;
					}
					else
						if (-e + aabb_min.z > 0.0f || -e + aabb_max.z < 0.0f)
							return false;
				}

				intersection_distance = tMin;
				return true;
		};


		const auto scalingFactor = direction * glm::distance(position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.02f;
		const auto mMatrix = glm::translate(glm::mat4(1.0f), position);
		glm::vec3 arrowAxes_min[3], arrowAxes_max[3], doubleAxes_min[3], doubleAxes_max[3], plane_normals[3], plane_intersections[3];
		arrowAxes_min[0] = glm::vec3(2, -0.5, -0.5) * scalingFactor;
		arrowAxes_max[0] = glm::vec3(8, 0.5, 0.5) * scalingFactor;
		arrowAxes_min[1] = glm::vec3(-0.5, 2, -0.5) * scalingFactor;
		arrowAxes_max[1] = glm::vec3(0.5, 8, 0.5) * scalingFactor;
		arrowAxes_min[2] = glm::vec3(-0.5, -0.5, 2) * scalingFactor;
		arrowAxes_max[2] = glm::vec3(0.5, 0.5, 8) * scalingFactor;
		doubleAxes_min[0] = glm::vec3(0.0f) * scalingFactor;
		doubleAxes_max[0] = glm::vec3(2.0f, 2.0f, 0.5f) * scalingFactor;
		doubleAxes_min[1] = glm::vec3(0.0f) * scalingFactor;
		doubleAxes_max[1] = glm::vec3(2.0f, 0.5f, 2.0f) * scalingFactor;
		doubleAxes_min[2] = glm::vec3(0.0f) * scalingFactor;
		doubleAxes_max[2] = glm::vec3(0.5, 2.0f, 2.0f) * scalingFactor;
		plane_normals[0] = glm::vec3(0, 0, 1) * direction; // xy
		plane_normals[1] = glm::vec3(0, 1, 0) * direction; // xz
		plane_normals[2] = glm::vec3(1, 0, 0) * direction; // yz

		// Find the closest axis that the user may have clicked on
		int closestClickedAxis = -1;
		float closestIntersection = FLT_MAX;
		for (int x = 0; x < 3; ++x) {
			float intDistance;
			if (!glm::intersectRayPlane(ray_origin, ray_world, position, plane_normals[x], intDistance))
				glm::intersectRayPlane(ray_origin, ray_world, position, -plane_normals[x], intDistance);
			if (TestRayOBBIntersection(ray_origin, ray_world, arrowAxes_min[x], arrowAxes_max[x], mMatrix, intDistance))
				if (intDistance < closestIntersection) {
					closestIntersection = intDistance;
					closestClickedAxis = x;
				}
			plane_intersections[x] = ray_origin + intDistance * ray_world;
		}
		// Check against double-axis
		for (int x = 0; x < 3; ++x) {
			float intDistance;
			if (TestRayOBBIntersection(ray_origin, ray_world, doubleAxes_min[x], doubleAxes_max[x], mMatrix, intDistance))
				if (intDistance < closestIntersection) {
					closestIntersection = intDistance;
					closestClickedAxis = x + 3;
				}
		}

		m_startingScale = m_transform.m_scale;
		m_startingPosition = position;
		m_startingOffset = position;
		// Set the appropriate selected axis
		if (closestClickedAxis == 0) {
			m_selectedAxes |= X_AXIS;
			// Check which of the ray-plane inter. point from xy and xz planes is closest to the camera
			if (glm::distance(plane_intersections[0], ray_origin) < glm::distance(plane_intersections[1], ray_origin))
				m_startingOffset.x = plane_intersections[0].x;
			else
				m_startingOffset.x = plane_intersections[1].x;
		}
		else if (closestClickedAxis == 1) {
			m_selectedAxes |= Y_AXIS;
			if (glm::distance(plane_intersections[0], ray_origin) < glm::distance(plane_intersections[2], ray_origin))
				m_startingOffset.y = plane_intersections[0].y;
			else
				m_startingOffset.y = plane_intersections[2].y;
		}
		else if (closestClickedAxis == 2) {
			m_selectedAxes |= Z_AXIS;
			if (glm::distance(plane_intersections[1], ray_origin) < glm::distance(plane_intersections[2], ray_origin))
				m_startingOffset.z = plane_intersections[1].z;
			else
				m_startingOffset.z = plane_intersections[2].z;
		}
		else if (closestClickedAxis == 3) {
			m_selectedAxes |= X_AXIS | Y_AXIS;
			m_startingOffset.x = plane_intersections[0].x;
			m_startingOffset.y = plane_intersections[0].y;
		}
		else if (closestClickedAxis == 4) {
			m_selectedAxes |= X_AXIS | Z_AXIS;
			m_startingOffset.x = plane_intersections[1].x;
			m_startingOffset.z = plane_intersections[1].z;
		}
		else if (closestClickedAxis == 5) {
			m_selectedAxes |= Y_AXIS | Z_AXIS;
			m_startingOffset.y = plane_intersections[2].y;
			m_startingOffset.z = plane_intersections[2].z;
		}

		m_axisDelta = m_startingOffset - position;
		return (m_selectedAxes != NONE);
	}

	// An axis is now selected, perform dragging operation
	else {
		glm::vec3 plane_normals[3], plane_intersections[3];
		plane_normals[0] = glm::vec3(0, 0, 1) * direction; // xy
		plane_normals[1] = glm::vec3(0, 1, 0) * direction; // xz
		plane_normals[2] = glm::vec3(1, 0, 0) * direction; // yz

		for (int x = 0; x < 3; ++x) {
			float intDistance;
			if (!glm::intersectRayPlane(ray_origin, ray_world, position, plane_normals[x], intDistance))
				glm::intersectRayPlane(ray_origin, ray_world, position, -plane_normals[x], intDistance);
			plane_intersections[x] = ray_origin + intDistance * ray_world;
		}

		auto endingOffset = m_startingOffset;
		if (m_selectedAxes == X_AXIS) {
			// Check which of the ray-plane inter. point from xy and xz planes is closest to the camera
			if (glm::distance(plane_intersections[0], ray_origin) < glm::distance(plane_intersections[1], ray_origin))
				endingOffset.x = plane_intersections[0].x;
			else
				endingOffset.x = plane_intersections[1].x;
		}
		else if (m_selectedAxes == Y_AXIS) {
			if (glm::distance(plane_intersections[0], ray_origin) < glm::distance(plane_intersections[2], ray_origin))
				endingOffset.y = plane_intersections[0].y;
			else
				endingOffset.y = plane_intersections[2].y;
		}
		else if (m_selectedAxes == Z_AXIS) {
			if (glm::distance(plane_intersections[1], ray_origin) < glm::distance(plane_intersections[2], ray_origin))
				endingOffset.z = plane_intersections[1].z;
			else
				endingOffset.z = plane_intersections[2].z;
		}
		else if ((m_selectedAxes & X_AXIS) && (m_selectedAxes & Y_AXIS)) {
			endingOffset.x = plane_intersections[0].x;
			endingOffset.y = plane_intersections[0].y;
		}
		else if ((m_selectedAxes & X_AXIS) && (m_selectedAxes & Z_AXIS)) {
			endingOffset.x = plane_intersections[1].x;
			endingOffset.z = plane_intersections[1].z;
		}
		else if ((m_selectedAxes & Y_AXIS) && (m_selectedAxes & Z_AXIS)) {
			endingOffset.y = plane_intersections[2].y;
			endingOffset.z = plane_intersections[2].z;
		}

		m_transform.m_scale = m_startingScale + (((endingOffset - m_axisDelta) - m_startingPosition) * 2.0f);
		m_editor->scaleSelection(m_transform.m_scale);
	}

	return false;
}