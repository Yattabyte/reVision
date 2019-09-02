#include "Modules/Editor/Gizmos/Rotation.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Utilities/Intersection.h"
#include "Engine.h"
#include "glm/gtx/vector_angle.hpp"

constexpr float DISK_VERTICES = 32.0F;
constexpr float DISK_RADIUS = 8.0F;
constexpr size_t DISK_MAX_POINTS = (size_t)DISK_VERTICES * 6ull;


Rotation_Gizmo::~Rotation_Gizmo()
{
	// Update indicator
	*m_aliveIndicator = false;

	glDeleteBuffers(1, &m_axisVBO);
	glDeleteVertexArrays(1, &m_axisVAO);
}

Rotation_Gizmo::Rotation_Gizmo(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	// Update indicator
	*m_aliveIndicator = true;

	// Assets
	m_model = Shared_Auto_Model(engine, "Editor\\rotate");
	m_gizmoShader = Shared_Shader(engine, "Editor\\gizmoShader");
	m_axisShader = Shared_Shader(engine, "Editor\\axis_line");

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

	// Axis Lines
	const glm::vec3 axisData[] = { glm::vec3(-1,0,0), glm::vec3(1,0,0) };
	glCreateBuffers(1, &m_axisVBO);
	glNamedBufferStorage(m_axisVBO, sizeof(glm::vec3) * 2, axisData, GL_CLIENT_STORAGE_BIT);
	glCreateVertexArrays(1, &m_axisVAO);
	glEnableVertexArrayAttrib(m_axisVAO, 0);
	glVertexArrayAttribBinding(m_axisVAO, 0, 0);
	glVertexArrayAttribFormat(m_axisVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(m_axisVAO, 0, m_axisVBO, 0, sizeof(glm::vec3));

	// Disk
	const GLuint diskData[4] = { 0, 1, 0, 0 };
	m_diskIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, diskData, GL_DYNAMIC_STORAGE_BIT);
	glCreateBuffers(1, &m_diskVBO);
	glNamedBufferStorage(m_diskVBO, sizeof(glm::vec3) * DISK_MAX_POINTS, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glCreateVertexArrays(1, &m_diskVAO);
	glEnableVertexArrayAttrib(m_diskVAO, 0);
	glVertexArrayAttribBinding(m_diskVAO, 0, 0);
	glVertexArrayAttribFormat(m_diskVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(m_diskVAO, 0, m_diskVBO, 0, sizeof(glm::vec3));
}

bool Rotation_Gizmo::checkMouseInput(const float& deltaTime)
{
	// See if the mouse intersects any entities
	// In any case move the translation gizmo to where the mouse is
	if (!ImGui::GetIO().WantCaptureMouse && ImGui::IsMouseDown(0)) {
		return rayCastMouse(deltaTime);
	}
	else {
		if (m_selectedAxes != NONE) {
			m_selectedAxes = NONE;
			return true; // block input as we just finished doing an action here
		}
		// use difference in position for undo/redo
		// const auto deltaPos = m_position - m_startingPosition;
	}
	return false;
}

void Rotation_Gizmo::render(const float& deltaTime)
{
	// Safety check first
	if (m_model->existsYet() && m_gizmoShader->existsYet() && m_editor->getSelection().size()) {
		// Set up state
		m_editor->bindFBO();

		// Flip the model's axes based on which side of it were on
		const auto& position = m_transform.m_position;
		const auto& camPos = m_editor->getCameraPosition();
		const auto dir = glm::normalize(camPos - position);
		const auto directions = glm::vec3(
			dir.x > 0 ? 1 : -1,
			dir.y > 0 ? 1 : -1,
			dir.z > 0 ? 1 : -1
		);

		// Get camera matrices
		const auto& pMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->pMatrix;
		const auto& vMatrix = m_engine->getModule_Graphics().getClientCamera()->get()->vMatrix;
		const auto trans = glm::translate(glm::mat4(1.0f), position);
		const auto mScale = glm::scale(glm::mat4(1.0f), directions * glm::vec3(glm::distance(position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.02f));
		const auto aScale = glm::scale(glm::mat4(1.0f), glm::vec3(m_engine->getModule_Graphics().getClientCamera()->get()->FarPlane * 2.0f, 0, 0));

		// Render Gizmo Model
		m_model->bind();
		m_gizmoShader->bind();
		m_gizmoShader->setUniform(0, pMatrix * vMatrix * trans * mScale);
		m_gizmoShader->setUniform(4, GLuint(m_selectedAxes));
		m_indicatorIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Render Disk
		if (m_selectedAxes != NONE) {
			updateDisk();
			const auto diskScale = glm::scale(glm::mat4(1.0f), glm::vec3(glm::distance(position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.02f));
			m_axisShader->bind();
			m_axisShader->setUniform(0, pMatrix * vMatrix * trans * diskScale);
			m_axisShader->setUniform(4, glm::vec3(1, 0.8, 0));
			glBindVertexArray(m_diskVAO);
			m_diskIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
						
		// Render Axis Lines
		glBindVertexArray(m_axisVAO);
		if (m_selectedAxes & X_AXIS) {
			m_axisShader->setUniform(0, pMatrix * vMatrix * trans * aScale);
			m_axisShader->setUniform(4, glm::vec3(1, 0, 0));
			glDrawArrays(GL_LINES, 0, 2);
		}
		if (m_selectedAxes & Y_AXIS) {
			const auto rot = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 0, 1));
			m_axisShader->setUniform(0, pMatrix * vMatrix * trans * rot * aScale);
			m_axisShader->setUniform(4, glm::vec3(0, 1, 0));
			glDrawArrays(GL_LINES, 0, 2);
		}
		if (m_selectedAxes & Z_AXIS) {
			const auto rot = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
			m_axisShader->setUniform(0, pMatrix * vMatrix * trans * rot * aScale);
			m_axisShader->setUniform(4, glm::vec3(0, 0, 1));
			glDrawArrays(GL_LINES, 0, 2);
		}

		// Revert State
		m_gizmoShader->Release();
	}
}

void Rotation_Gizmo::setTransform(const Transform & transform)
{
	m_transform = transform;
}

bool Rotation_Gizmo::rayCastMouse(const float& deltaTime)
{
	const auto& actionState = m_engine->getActionState();
	const auto& position = m_transform.m_position;
	const auto& rotation = m_transform.m_orientation;
	const auto& clientCamera = *m_engine->getModule_Graphics().getClientCamera()->get();
	const auto ray_origin = clientCamera.EyePosition;
	const auto ray_nds = glm::vec2(2.0f * actionState.at(ActionState::MOUSE_X) / m_renderSize.x - 1.0f, 1.0f - (2.0f * actionState.at(ActionState::MOUSE_Y)) / m_renderSize.y);
	const auto ray_eye = glm::vec4(glm::vec2(clientCamera.pMatrixInverse * glm::vec4(ray_nds, -1.0f, 1.0F)), -1.0f, 0.0f);
	const auto ray_world = glm::normalize(glm::vec3(clientCamera.vMatrixInverse * ray_eye));
	m_startingAngle = 0.0f;
	m_deltaAngle = 0.0f;

	// Flip the model's axes based on which side of it were on
	const auto dir = glm::normalize(ray_origin - position);
	const auto direction = glm::vec3(
		dir.x > 0 ? 1 : -1,
		dir.y > 0 ? 1 : -1,
		dir.z > 0 ? 1 : -1
	);

	// Check if the user selected an axis
	if ((m_selectedAxes == NONE) && !ImGui::IsMouseDragging(0)) {
		const auto scalingFactor = direction * glm::distance(position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.02f;
		const auto mMatrix = glm::translate(glm::mat4(1.0f), position);
		glm::vec3 disk_normals[3], disk_intersections[3];
		disk_normals[0] = glm::vec3(1, 0, 0);
		disk_normals[1] = glm::vec3(0, 1, 0);
		disk_normals[2] = glm::vec3(0, 0, 1);

		// Find the closest axis that the user may have clicked on
		int clickedAxis = -1;
		float closestIntersection = FLT_MAX;
		for (int x = 0; x < 3; ++x) {
			float intDistance = FLT_MAX;
			if (!RayPlaneIntersection(ray_origin, ray_world, position, disk_normals[x], intDistance))
				RayPlaneIntersection(ray_origin, ray_world, position, -disk_normals[x], intDistance);
			disk_intersections[x] = ray_origin + intDistance * ray_world;
			const auto distance = fabsf(glm::distance(disk_intersections[x], position) * (1.0f / scalingFactor.x));	
			if ((intDistance < closestIntersection) && (distance <= 8.25f) && (distance >= 6.75f)) {
				closestIntersection = intDistance;
				clickedAxis = x;
				m_startPoint = disk_intersections[x];
			}
		}	
		m_prevRot = rotation;
		// Set the appropriate selected axis
		if (clickedAxis == 0) 
			m_selectedAxes |= X_AXIS;		
		else if (clickedAxis == 1)
			m_selectedAxes |= Y_AXIS;		
		else if (clickedAxis == 2)
			m_selectedAxes |= Z_AXIS;		
		return (m_selectedAxes != NONE);
	}
	
	// An axis is now selected, perform dragging operation
	else if ((m_selectedAxes != NONE) && ImGui::IsMouseDragging(0)) {
		const auto normal = m_selectedAxes & X_AXIS ? glm::vec3(1, 0, 0) : m_selectedAxes & Y_AXIS ? glm::vec3(0, 1, 0) : glm::vec3(0, 0, 1);
		float intDistance = FLT_MAX;
		if (!RayPlaneIntersection(ray_origin, ray_world, position, normal, intDistance))
			RayPlaneIntersection(ray_origin, ray_world, position, -normal, intDistance);
		const auto endPoint = ray_origin + intDistance * ray_world;
		const auto a = glm::normalize(m_startPoint - position), b = glm::normalize(endPoint - position);

		// Measure the angle between the first point we clicked and the spot we've dragged to
		const auto angle = glm::orientedAngle(a, b, normal);
		const auto newQuat = glm::rotate(glm::quat(1, 0, 0, 0), angle, normal);
		if (m_selectedAxes == X_AXIS)
			m_startingAngle = glm::atan(m_startPoint.z - position.z, m_startPoint.y - position.y);
		else if (m_selectedAxes == Y_AXIS)
			m_startingAngle = glm::atan(m_startPoint.x - position.x, m_startPoint.z - position.z);
		else if (m_selectedAxes == Z_AXIS)
			m_startingAngle = glm::atan(m_startPoint.y - position.y, m_startPoint.x - position.x);
		m_deltaAngle = glm::degrees(angle);
		

		// Undo the previous rotation, rotate selection to the current quaternion
		// Note: editor rotations aren't explicit, they are deltas, as a group of objects could be selected and need to orbit their center point
		m_editor->rotateSelection(newQuat * glm::inverse(m_prevRot));
		m_prevRot = newQuat;		
		return true;
	}

	return false;
}

void Rotation_Gizmo::updateDisk()
{
	int steps = (int)ceilf((abs(m_deltaAngle) / 360.0f) * DISK_VERTICES);
	std::vector<glm::vec3> points(size_t(steps) * 6ull, glm::vec3(0.0f));
	for (int n = 0, v = 0; n < steps; ++n, v += 6) {
		const auto startAngle = glm::radians(float(n) * (m_deltaAngle / float(steps))) + m_startingAngle;
		const auto endAngle = glm::radians(float(n + 1) * (m_deltaAngle / float(steps))) + m_startingAngle;

		const auto x1 = 8.0f * cosf(startAngle), y1 = 8.0f * sinf(startAngle), 
			x2 = 8.0f * cosf(endAngle), y2 = 8.0f * sinf(endAngle);

		glm::vec3 v1(0.0f), v2(0.0f);
		if (m_selectedAxes == X_AXIS) {
			v1 = glm::vec3(0, x1, y1);
			v2 = glm::vec3(0, x2, y2);
		}
		else if (m_selectedAxes == Y_AXIS) {
			v1 = glm::vec3(y1, 0, x1);
			v2 = glm::vec3(y2, 0, x2);
		}
		else if (m_selectedAxes == Z_AXIS) {
			v1 = glm::vec3(x1, y1, 0);
			v2 = glm::vec3(x2, y2, 0);
		}
		points[v + 0] = glm::vec3(0);
		points[v + 1] = v1;
		points[v + 2] = v2;
		points[v + 3] = glm::vec3(0);
		points[v + 4] = v2;
		points[v + 5] = v1;
	}
	const auto stepCount = GLuint(points.size());
	m_diskIndirectBuffer.write(0, sizeof(GLuint), &stepCount);
	glNamedBufferSubData(m_diskVBO, 0, sizeof(glm::vec3) * points.size(), points.data());
}
