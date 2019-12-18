#include "Modules/Editor/Gizmos/Scaling.h"
#include "Modules/ECS/component_types.h"
#include "Utilities/Intersection.h"
#include "Engine.h"
#include "imgui.h"


Scaling_Gizmo::~Scaling_Gizmo() noexcept
{
	// Update indicator
	*m_aliveIndicator = false;

	glDeleteBuffers(1, &m_axisVBO);
	glDeleteVertexArrays(1, &m_axisVAO);
}

Scaling_Gizmo::Scaling_Gizmo(Engine& engine, LevelEditor_Module& editor) :
	m_engine(engine),
	m_editor(editor),
	m_model(Shared_Auto_Model(engine, "Editor\\scale")),
	m_gizmoShader(Shared_Shader(engine, "Editor\\gizmoShader")),
	m_axisShader(Shared_Shader(engine, "Editor\\axis_line"))
{
	// Update indicator
	*m_aliveIndicator = true;

	// Asset-Finished Callbacks
	m_model->addCallback(m_aliveIndicator, [&]() noexcept {
		m_indirectIndicator = IndirectDraw<1>((GLuint)m_model->getSize(), 1, 0, GL_CLIENT_STORAGE_BIT);
		});

	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.x = (int)f;
		});
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.y = (int)f;
		});
	preferences.getOrSetValue(PreferenceState::Preference::E_GIZMO_SCALE, m_renderScale);
	preferences.addCallback(PreferenceState::Preference::E_GIZMO_SCALE, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderScale = f;
		});
	preferences.getOrSetValue(PreferenceState::Preference::E_GRID_SNAP, m_gridSnap);
	preferences.addCallback(PreferenceState::Preference::E_GRID_SNAP, m_aliveIndicator, [&](const float& f) noexcept {
		m_gridSnap = f;
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

bool Scaling_Gizmo::checkMouseInput(const float&)
{
	// See if the mouse intersects any entities.
	checkMouseHover();
	if (!ImGui::GetIO().WantCaptureMouse && ImGui::IsMouseDown(0))
		return checkMousePress();
	else {
		if (m_selectedAxes != NONE) {
			m_selectedAxes = NONE;
			return true; // block input as we just finished doing an action here
		}
	}
	return false;
}

void Scaling_Gizmo::render(const float&)
{
	// Safety check first
	if (Asset::All_Ready(m_model, m_gizmoShader) && m_editor.getSelection().size()) {
		// Set up state
		m_editor.bindFBO();

		// Get camera matrices
		const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
		const auto& position = m_transform.m_position;
		const auto& pMatrix = clientCamera->pMatrix;
		const auto& vMatrix = clientCamera->vMatrix;
		const auto trans = glm::translate(glm::mat4(1.0f), position);
		const auto mScale = glm::scale(glm::mat4(1.0f), glm::vec3(glm::distance(position, clientCamera->EyePosition) * m_renderScale));
		const auto aScale = glm::scale(glm::mat4(1.0f), glm::vec3(clientCamera->FarPlane * 2.0f));

		// Render Gizmo Model
		m_model->bind();
		m_gizmoShader->bind();
		m_gizmoShader->setUniform(0, pMatrix * vMatrix * trans * mScale);
		m_gizmoShader->setUniform(4, GLuint(m_selectedAxes));
		m_gizmoShader->setUniform(5, GLuint(m_hoveredAxes));
		m_indirectIndicator.drawCall();

		// Render Axis Lines
		m_axisShader->bind();
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
		Shader::Release();
	}
}

void Scaling_Gizmo::setTransform(const Transform& transform) noexcept
{
	m_transform = transform;
}

void Scaling_Gizmo::checkMouseHover()
{
	const auto& actionState = m_engine.getActionState();
	const auto& position = m_transform.m_position;
	const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
	const auto ray_origin = clientCamera->EyePosition;
	const auto ray_nds = glm::vec2(2.0f * actionState[ActionState::Action::MOUSE_X] / m_renderSize.x - 1.0f, 1.0f - (2.0f * actionState[ActionState::Action::MOUSE_Y]) / m_renderSize.y);
	const auto ray_eye = glm::vec4(glm::vec2(clientCamera->pMatrixInverse * glm::vec4(ray_nds, -1.0f, 1.0F)), -1.0f, 0.0f);
	const auto ray_world = glm::normalize(glm::vec3(clientCamera->vMatrixInverse * ray_eye));

	const auto scalingFactor = glm::distance(position, clientCamera->EyePosition) * m_renderScale;
	const auto mMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::vec3 arrowAxes_min[3]{}, arrowAxes_max[3]{}, doubleAxes_min[3]{}, doubleAxes_max[3]{}, plane_normals[3]{};
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
	plane_normals[0] = glm::vec3(0, 0, 1);
	plane_normals[1] = glm::vec3(0, 1, 0);
	plane_normals[2] = glm::vec3(1, 0, 0);

	// Find the closest axis that the user may have clicked on
	int hoveredAxis = -1;
	float closestIntersection = FLT_MAX;
	for (int x = 0; x < 3; ++x) {
		float intDistance;
		if (!RayPlaneIntersection(ray_origin, ray_world, position, plane_normals[x], intDistance))
			RayPlaneIntersection(ray_origin, ray_world, position, -plane_normals[x], intDistance);
		if (RayOOBBIntersection(ray_origin, ray_world, arrowAxes_min[x], arrowAxes_max[x], mMatrix, intDistance))
			if (intDistance < closestIntersection) {
				closestIntersection = intDistance;
				hoveredAxis = x;
			}
		m_hoveredEnds[x] = ray_origin + intDistance * ray_world;
	}
	// Check against double-axis
	for (int x = 0; x < 3; ++x) {
		float intDistance;
		if (RayOOBBIntersection(ray_origin, ray_world, doubleAxes_min[x], doubleAxes_max[x], mMatrix, intDistance))
			if (intDistance < closestIntersection) {
				closestIntersection = intDistance;
				hoveredAxis = x + 3;
			}
	}

	// Set the appropriate selected axis
	m_hoveredAxes = NONE;
	if (hoveredAxis == 0)
		m_hoveredAxes |= X_AXIS;
	else if (hoveredAxis == 1)
		m_hoveredAxes |= Y_AXIS;
	else if (hoveredAxis == 2)
		m_hoveredAxes |= Z_AXIS;
	else if (hoveredAxis == 3)
		m_hoveredAxes |= X_AXIS | Y_AXIS;
	else if (hoveredAxis == 4)
		m_hoveredAxes |= X_AXIS | Z_AXIS;
	else if (hoveredAxis == 5)
		m_hoveredAxes |= Y_AXIS | Z_AXIS;
}

bool Scaling_Gizmo::checkMousePress()
{
	const auto& position = m_transform.m_position;
	const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
	const auto ray_origin = clientCamera->EyePosition;

	// Check if the user selected an axis
	if (m_selectedAxes == NONE && !ImGui::IsMouseDragging(0)) {
		m_selectedAxes = m_hoveredAxes;
		m_prevScale = m_transform.m_scale;
		m_startingPosition = position;
		m_startingOffset = position;
		// Set the appropriate selected axis
		if (m_hoveredAxes == X_AXIS) {
			// Check which of the ray-plane inter. point from XY and XZ planes is closest to the camera
			if (glm::distance(m_hoveredEnds[0], ray_origin) < glm::distance(m_hoveredEnds[1], ray_origin))
				m_startingOffset.x = m_hoveredEnds[0].x;
			else
				m_startingOffset.x = m_hoveredEnds[1].x;
		}
		else if (m_hoveredAxes == Y_AXIS) {
			if (glm::distance(m_hoveredEnds[0], ray_origin) < glm::distance(m_hoveredEnds[2], ray_origin))
				m_startingOffset.y = m_hoveredEnds[0].y;
			else
				m_startingOffset.y = m_hoveredEnds[2].y;
		}
		else if (m_hoveredAxes == Z_AXIS) {
			if (glm::distance(m_hoveredEnds[1], ray_origin) < glm::distance(m_hoveredEnds[2], ray_origin))
				m_startingOffset.z = m_hoveredEnds[1].z;
			else
				m_startingOffset.z = m_hoveredEnds[2].z;
		}
		else if (m_hoveredAxes == (X_AXIS | Y_AXIS)) {
			m_startingOffset.x = m_hoveredEnds[0].x;
			m_startingOffset.y = m_hoveredEnds[0].y;
		}
		else if (m_hoveredAxes == (X_AXIS | Z_AXIS)) {
			m_startingOffset.x = m_hoveredEnds[1].x;
			m_startingOffset.z = m_hoveredEnds[1].z;
		}
		else if (m_hoveredAxes == (Y_AXIS | Z_AXIS)) {
			m_startingOffset.y = m_hoveredEnds[2].y;
			m_startingOffset.z = m_hoveredEnds[2].z;
		}

		m_axisDelta = m_startingOffset - position;
		return (m_selectedAxes != NONE);
	}

	// An axis is now selected, perform dragging operation
	else {
		constexpr auto gridSnapValue = [](const float& value, const float& delta, const float& prevValue, const float& startingValue, const float& snapAmt) -> float {
			const float scale = prevValue + (((value - delta) - startingValue) * 2.0f);
			return snapAmt ? (float(int((scale + (snapAmt / 2.0F)) / snapAmt)) * snapAmt) : scale;
		};
		auto scale = m_prevScale;
		if (m_selectedAxes == X_AXIS) {
			// Check which of the ray-plane inter. point from XY and XZ planes is closest to the camera
			if (glm::distance(m_hoveredEnds[0], ray_origin) < glm::distance(m_hoveredEnds[1], ray_origin))
				scale.x = gridSnapValue(m_hoveredEnds[0].x, m_axisDelta.x, m_prevScale.x, m_startingPosition.x, m_gridSnap);
			else
				scale.x = gridSnapValue(m_hoveredEnds[1].x, m_axisDelta.x, m_prevScale.x, m_startingPosition.x, m_gridSnap);
		}
		else if (m_selectedAxes == Y_AXIS) {
			if (glm::distance(m_hoveredEnds[0], ray_origin) < glm::distance(m_hoveredEnds[2], ray_origin))
				scale.y = gridSnapValue(m_hoveredEnds[0].y, m_axisDelta.y, m_prevScale.y, m_startingPosition.y, m_gridSnap);
			else
				scale.y = gridSnapValue(m_hoveredEnds[2].y, m_axisDelta.y, m_prevScale.y, m_startingPosition.y, m_gridSnap);
		}
		else if (m_selectedAxes == Z_AXIS) {
			if (glm::distance(m_hoveredEnds[1], ray_origin) < glm::distance(m_hoveredEnds[2], ray_origin))
				scale.z = gridSnapValue(m_hoveredEnds[1].z, m_axisDelta.z, m_prevScale.z, m_startingPosition.z, m_gridSnap);
			else
				scale.z = gridSnapValue(m_hoveredEnds[2].z, m_axisDelta.z, m_prevScale.z, m_startingPosition.z, m_gridSnap);
		}
		else if ((m_selectedAxes & X_AXIS) && (m_selectedAxes & Y_AXIS)) {
			scale.x = gridSnapValue(m_hoveredEnds[0].x, m_axisDelta.x, m_prevScale.x, m_startingPosition.x, m_gridSnap);
			scale.y = gridSnapValue(m_hoveredEnds[0].y, m_axisDelta.y, m_prevScale.y, m_startingPosition.y, m_gridSnap);
		}
		else if ((m_selectedAxes & X_AXIS) && (m_selectedAxes & Z_AXIS)) {
			scale.x = gridSnapValue(m_hoveredEnds[1].x, m_axisDelta.x, m_prevScale.x, m_startingPosition.x, m_gridSnap);
			scale.z = gridSnapValue(m_hoveredEnds[1].z, m_axisDelta.z, m_prevScale.z, m_startingPosition.z, m_gridSnap);
		}
		else if ((m_selectedAxes & Y_AXIS) && (m_selectedAxes & Z_AXIS)) {
			scale.y = gridSnapValue(m_hoveredEnds[2].y, m_axisDelta.y, m_prevScale.y, m_startingPosition.y, m_gridSnap);
			scale.z = gridSnapValue(m_hoveredEnds[2].z, m_axisDelta.z, m_prevScale.z, m_startingPosition.z, m_gridSnap);
		}
		if (scale.x == 0.0f)
			scale.x += 0.0001F;
		if (scale.y == 0.0f)
			scale.y += 0.0001F;
		if (scale.z == 0.0f)
			scale.z += 0.0001F;
		m_transform.m_scale = scale;

		struct Scale_Selection_Command final : Editor_Command {
			Engine& m_engine;
			LevelEditor_Module& m_editor;
			glm::vec3 m_oldScale, m_newScale;
			const unsigned int m_axis = NONE;
			const std::vector<EntityHandle> m_uuids;
			Scale_Selection_Command(Engine& engine, LevelEditor_Module& editor, const glm::vec3& newRotation, const unsigned int& axis) noexcept
				: m_engine(engine), m_editor(editor), m_oldScale(m_editor.getGizmoTransform().m_scale), m_newScale(newRotation), m_axis(axis), m_uuids(m_editor.getSelection()) {}
			void scale(const glm::vec3& scale) noexcept {
				const auto& ecsWorld = m_editor.getWorld();
				std::vector<Transform_Component*> transformComponents;
				glm::vec3 center(0.0f);
				for (const auto& entityHandle : m_uuids)
					if (auto* transform = ecsWorld.getComponent<Transform_Component>(entityHandle)) {
						transformComponents.push_back(transform);
						center += transform->m_localTransform.m_position;
					}
				center /= transformComponents.size();
				for (auto* transform : transformComponents) {
					const auto delta = transform->m_localTransform.m_position - center;
					transform->m_localTransform.m_position = ((delta / transform->m_localTransform.m_scale) * scale) + center;
					transform->m_localTransform.m_scale = scale;
					transform->m_localTransform.update();
				}
				auto gizmoTransform = m_editor.getGizmoTransform();
				gizmoTransform.m_scale = scale;
				gizmoTransform.update();
				m_editor.setGizmoTransform(gizmoTransform);
			}
			void execute() noexcept final {
				scale(m_newScale);
			}
			void undo() noexcept final {
				scale(m_oldScale);
			}
			bool join(Editor_Command* other) noexcept final {
				if (const auto& newCommand = dynamic_cast<Scale_Selection_Command*>(other)) {
					if (m_axis == newCommand->m_axis && std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
						m_newScale = newCommand->m_newScale;
						return true;
					}
				}
				return false;
			}
		};
		m_editor.doReversableAction(std::make_shared<Scale_Selection_Command>(m_engine, m_editor, scale, m_selectedAxes));
		return true;
	}
}