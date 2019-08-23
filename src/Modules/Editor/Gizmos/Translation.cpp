#include "Modules/Editor/Gizmos/Translation.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Utilities/Intersection.h"
#include "Engine.h"


Translation_Gizmo::~Translation_Gizmo()
{
	// Update indicator
	*m_aliveIndicator = false;

	glDeleteBuffers(1, &m_axisVBO);
	glDeleteVertexArrays(1, &m_axisVAO);
}

Translation_Gizmo::Translation_Gizmo(Engine* engine, LevelEditor_Module* editor)
	: m_engine(engine), m_editor(editor)
{
	// Update indicator
	*m_aliveIndicator = true;

	// Assets
	m_colorPalette = Shared_Texture(engine, "Editor\\colors.png");
	m_model = Shared_Auto_Model(engine, "Editor\\translate");
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

	// Create VAO
	glCreateVertexArrays(1, &m_axisVAO);
	glEnableVertexArrayAttrib(m_axisVAO, 0);
	glVertexArrayAttribBinding(m_axisVAO, 0, 0);
	glVertexArrayAttribFormat(m_axisVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(m_axisVAO, 0, m_axisVBO, 0, sizeof(glm::vec3));
}

bool Translation_Gizmo::checkMouseInput(const float& deltaTime)
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

void Translation_Gizmo::render(const float& deltaTime)
{
	// Safety check first
	if (m_model->existsYet() && m_colorPalette->existsYet() && m_gizmoShader->existsYet() && m_editor->getSelection().size()) {
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
				
		// Render Axis Lines
		glBindVertexArray(m_axisVAO);
		m_axisShader->bind();
		if (m_selectedAxes & X_AXIS) {
			m_axisShader->setUniform(0, pMatrix * vMatrix * glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(m_engine->getModule_Graphics().getClientCamera()->get()->FarPlane * 2.0f, 0, 0)));
			m_axisShader->setUniform(4, glm::vec3(1, 0, 0));
			glDrawArrays(GL_LINES, 0, 2);
		}
		if (m_selectedAxes & Y_AXIS) {
			m_axisShader->setUniform(0, pMatrix * vMatrix * glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0,0,1)) * glm::scale(glm::mat4(1.0f), glm::vec3(m_engine->getModule_Graphics().getClientCamera()->get()->FarPlane * 2.0f, 0, 0)));
			m_axisShader->setUniform(4, glm::vec3(0, 1, 0));
			glDrawArrays(GL_LINES, 0, 2);
		}
		if (m_selectedAxes & Z_AXIS) {
			m_axisShader->setUniform(0, pMatrix * vMatrix * glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(m_engine->getModule_Graphics().getClientCamera()->get()->FarPlane * 2.0f, 0, 0)));
			m_axisShader->setUniform(4, glm::vec3(0, 0, 1));
			glDrawArrays(GL_LINES, 0, 2);
		}

		// Render Gizmo Model
		m_model->bind();
		m_colorPalette->bind(0);
		m_gizmoShader->bind();
		m_gizmoShader->setUniform(0, pMatrix * vMatrix * glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), directions * glm::vec3(glm::distance(position, m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition) * 0.02f)));
		m_indicatorIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Revert State
		m_gizmoShader->Release();
	}
}

void Translation_Gizmo::setTransform(const Transform & transform)
{
	m_transform = transform;
}

bool Translation_Gizmo::rayCastMouse(const float& deltaTime)
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
			if (!RayPlaneIntersection(ray_origin, ray_world, position, plane_normals[x], intDistance))
				RayPlaneIntersection(ray_origin, ray_world, position, -plane_normals[x], intDistance);
			if (RayOOBBIntersection(ray_origin, ray_world, arrowAxes_min[x], arrowAxes_max[x], mMatrix, intDistance))
				if (intDistance < closestIntersection) {
					closestIntersection = intDistance;
					closestClickedAxis = x;
				}			
			plane_intersections[x] = ray_origin + intDistance * ray_world;
		}
		// Check against double-axis
		for (int x = 0; x < 3; ++x) {
			float intDistance;
			if (RayOOBBIntersection(ray_origin, ray_world, doubleAxes_min[x], doubleAxes_max[x], mMatrix, intDistance))
				if (intDistance < closestIntersection) {
					closestIntersection = intDistance;
					closestClickedAxis = x+3;
				}
		}

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

		m_transform.m_position = endingOffset - m_axisDelta;
		m_editor->moveSelection(m_transform.m_position);
	}

	return false;
}