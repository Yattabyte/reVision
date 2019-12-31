#include "Modules/Editor/Gizmos/Rotation.h"
#include "Modules/ECS/component_types.h"
#include "Utilities/Intersection.h"
#include "Engine.h"
#include "imgui.h"
#include "glm/gtx/vector_angle.hpp"


constexpr float DISK_VERTICES = 32.0F;
constexpr float DISK_RADIUS = 8.0F;
constexpr size_t DISK_MAX_POINTS = static_cast<size_t>(DISK_VERTICES) * 6ULL;

Rotation_Gizmo::~Rotation_Gizmo()
{
	// Update indicator
	*m_aliveIndicator = false;

	glDeleteBuffers(1, &m_axisVBO);
	glDeleteVertexArrays(1, &m_axisVAO);
}

Rotation_Gizmo::Rotation_Gizmo(Engine& engine, LevelEditor_Module& editor) :
	m_engine(engine),
	m_editor(editor),
	m_model(Shared_Auto_Model(engine, "Editor\\rotate")),
	m_gizmoShader(Shared_Shader(engine, "Editor\\ringShader")),
	m_axisShader(Shared_Shader(engine, "Editor\\axis_line"))
{
	// Update indicator
	*m_aliveIndicator = true;

	// Asset-Finished Callbacks
	m_model->addCallback(m_aliveIndicator, [&]() noexcept {
		m_indirectIndicator = IndirectDraw<1>(static_cast<GLuint>(m_model->getSize()), 1, 0, GL_CLIENT_STORAGE_BIT);
		});

	// Preferences
	auto& preferences = engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.x = static_cast<int>(f);
		});
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.y = static_cast<int>(f);
		});
	preferences.getOrSetValue(PreferenceState::Preference::E_GIZMO_SCALE, m_renderScale);
	preferences.addCallback(PreferenceState::Preference::E_GIZMO_SCALE, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderScale = f;
		});
	preferences.getOrSetValue(PreferenceState::Preference::E_ANGLE_SNAP, m_angleSnapping);
	preferences.addCallback(PreferenceState::Preference::E_ANGLE_SNAP, m_aliveIndicator, [&](const float& f) noexcept {
		m_angleSnapping = f;
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
	m_indirectDisk = IndirectDraw<>(0, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	glCreateBuffers(1, &m_diskVBO);
	glNamedBufferStorage(m_diskVBO, sizeof(glm::vec3) * DISK_MAX_POINTS, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glCreateVertexArrays(1, &m_diskVAO);
	glEnableVertexArrayAttrib(m_diskVAO, 0);
	glVertexArrayAttribBinding(m_diskVAO, 0, 0);
	glVertexArrayAttribFormat(m_diskVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(m_diskVAO, 0, m_diskVBO, 0, sizeof(glm::vec3));
}

bool Rotation_Gizmo::checkMouseInput(const float& /*unused*/)
{
	// See if the mouse intersects any entities
	checkMouseHover();
	if (!ImGui::GetIO().WantCaptureMouse && ImGui::IsMouseDown(0))
		return checkMousePress();

	if (m_selectedAxes != NONE) {
		m_selectedAxes = NONE;
		return true; // block input as we just finished doing an action here
	}

	return false;
}

void Rotation_Gizmo::render(const float& /*unused*/)
{
	// Safety check first
	if (Asset::All_Ready(m_model, m_gizmoShader) && (!m_editor.getSelection().empty())) {
		// Set up state
		m_editor.bindFBO();

		// Get camera matrices
		const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
		const auto& position = m_transform.m_position;
		const auto& pMatrix = clientCamera->pMatrix;
		const auto& vMatrix = clientCamera->vMatrix;
		const auto camRotation = vMatrix * glm::translate(clientCamera->EyePosition);

		const auto trans = glm::translate(position);
		const auto mScale = glm::scale(glm::vec3(glm::distance(position, clientCamera->EyePosition) * m_renderScale));
		const auto aScale = glm::scale(glm::vec3(clientCamera->FarPlane * 2.0F));

		// Render Gizmo Model
		m_model->bind();
		m_gizmoShader->bind();
		m_gizmoShader->setUniform(0, pMatrix * vMatrix * trans * mScale);
		m_gizmoShader->setUniform(4, pMatrix * vMatrix * (trans * glm::inverse(camRotation)) * mScale * glm::scale(glm::mat4(1.0F), glm::vec3(1.15F)));
		m_gizmoShader->setUniform(8, GLuint(m_selectedAxes));
		m_gizmoShader->setUniform(9, GLuint(m_hoveredAxes));
		m_indirectIndicator.drawCall();

		// Render Disk
		if (m_selectedAxes != NONE) {
			m_indirectDisk.beginWriting();
			updateDisk();
			m_indirectDisk.endWriting();
			const auto diskScale = glm::scale(glm::mat4(1.0F), glm::vec3(glm::distance(position, clientCamera->EyePosition) * m_renderScale)) * glm::scale(glm::mat4(1.0F), glm::vec3(m_selectedAxes == ALL_AXES ? 1.15F : 1.0F));
			m_axisShader->bind();
			m_axisShader->setUniform(0, pMatrix * vMatrix * trans * diskScale);
			m_axisShader->setUniform(4, glm::vec3(1, 0.8, 0));
			glBindVertexArray(m_diskVAO);
			m_indirectDisk.drawCall();
			m_indirectDisk.endReading();
		}

		// Render Axis Lines
		glBindVertexArray(m_axisVAO);
		if ((m_selectedAxes & X_AXIS) != 0U) {
			m_axisShader->setUniform(0, pMatrix * vMatrix * trans * aScale);
			m_axisShader->setUniform(4, glm::vec3(1, 0, 0));
			glDrawArrays(GL_LINES, 0, 2);
		}
		if ((m_selectedAxes & Y_AXIS) != 0U) {
			const auto rot = glm::rotate(glm::mat4(1.0F), glm::radians(-90.0F), glm::vec3(0, 0, 1));
			m_axisShader->setUniform(0, pMatrix * vMatrix * trans * rot * aScale);
			m_axisShader->setUniform(4, glm::vec3(0, 1, 0));
			glDrawArrays(GL_LINES, 0, 2);
		}
		if ((m_selectedAxes & Z_AXIS) != 0U) {
			const auto rot = glm::rotate(glm::mat4(1.0F), glm::radians(90.0F), glm::vec3(0, 1, 0));
			m_axisShader->setUniform(0, pMatrix * vMatrix * trans * rot * aScale);
			m_axisShader->setUniform(4, glm::vec3(0, 0, 1));
			glDrawArrays(GL_LINES, 0, 2);
		}

		// Revert State
		Shader::Release();
	}
}

void Rotation_Gizmo::setTransform(const Transform& transform) noexcept
{
	m_transform = transform;
}

void Rotation_Gizmo::checkMouseHover()
{
	const auto& actionState = m_engine.getActionState();
	const auto& position = m_transform.m_position;
	const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
	const auto ray_origin = clientCamera->EyePosition;
	const auto ray_nds = glm::vec2(2.0F * actionState[ActionState::Action::MOUSE_X] / m_renderSize.x - 1.0F, 1.0F - (2.0F * actionState[ActionState::Action::MOUSE_Y]) / m_renderSize.y);
	const auto ray_eye = glm::vec4(glm::vec2(clientCamera->pMatrixInverse * glm::vec4(ray_nds, -1.0F, 1.0F)), -1.0F, 0.0F);
	const auto ray_world = glm::normalize(glm::vec3(clientCamera->vMatrixInverse * ray_eye));

	const auto scalingFactor = glm::distance(position, clientCamera->EyePosition) * m_renderScale;
	const auto fourthAxisMat = glm::inverse(glm::mat4_cast(glm::quat_cast(clientCamera->vMatrix)));
	const auto fourthAxisNormal = fourthAxisMat * glm::vec4(0, 0, 1, 1);

	glm::vec3 disk_normals[4] = {
		glm::vec3(1, 0, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 1),
		glm::normalize(glm::vec3(fourthAxisNormal / fourthAxisNormal.w))
	};


	// Find the closest axis that the user may have hovered over
	int hoveredAxis = -1;
	float closestIntersection = FLT_MAX;
	constexpr float OUTER_DIAMETER = 10.0F;
	constexpr float INNER_DIAMETER = 7.5F;
	for (int x = 0; x < 4; ++x) {
		float intDistance = FLT_MAX;
		if (!RayPlaneIntersection(ray_origin, ray_world, position, disk_normals[x], intDistance))
			RayPlaneIntersection(ray_origin, ray_world, position, -disk_normals[x], intDistance);
		m_hoveredEnds[x] = ray_origin + intDistance * ray_world;
		const auto distance = fabsf(glm::distance(m_hoveredEnds[x], position) * (1.0F / scalingFactor));
		if ((intDistance < closestIntersection) && (distance <= OUTER_DIAMETER) && (distance >= INNER_DIAMETER)) {
			closestIntersection = intDistance;
			hoveredAxis = x;
			m_hoveredPoint = m_hoveredEnds[x];
		}
	}

	// Set the appropriate hovered axis
	m_hoveredAxes = NONE;
	if (hoveredAxis == 0)
		m_hoveredAxes |= X_AXIS;
	else if (hoveredAxis == 1)
		m_hoveredAxes |= Y_AXIS;
	else if (hoveredAxis == 2)
		m_hoveredAxes |= Z_AXIS;
	else if (hoveredAxis == 3)
		m_hoveredAxes = ALL_AXES;
}

bool Rotation_Gizmo::checkMousePress()
{
	const auto& position = m_transform.m_position;
	m_startingAngle = 0.0F;
	m_deltaAngle = 0.0F;

	// Check if the user selected an axis
	if ((m_selectedAxes == NONE) && !ImGui::IsMouseDragging(0)) {
		m_startPoint = m_hoveredPoint;
		m_selectedAxes = m_hoveredAxes;
		m_startingOrientation = glm::quat(1,0,0,0);
		return (m_selectedAxes != NONE);
	}

	// An axis is now selected, perform dragging operation
	if ((m_selectedAxes != NONE) && ImGui::IsMouseDragging(0)) {
		const auto index = (m_selectedAxes & X_AXIS) != 0U ? 0 : (m_selectedAxes & Y_AXIS) != 0U ? 1 : (m_selectedAxes & Z_AXIS) != 0U ? 2 : 3;
		const auto fourthAxisMat = glm::inverse(glm::mat4_cast(glm::quat_cast(m_engine.getModule_Graphics().getClientCamera()->vMatrix)));
		const auto fourthAxisNormal = fourthAxisMat * glm::vec4(0, 0, 1, 1);
		const glm::vec3 normals[4] = { glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::normalize(glm::vec3(fourthAxisNormal / fourthAxisNormal.w)) };
		const auto endPoint = m_hoveredEnds[index];
		const auto a = glm::normalize(m_startPoint - position);
		const auto b = glm::normalize(endPoint - position);

		// Measure the angle between the first point we clicked and the spot we've dragged to, then snap to grid
		const auto angle = glm::degrees(glm::orientedAngle(a, b, normals[index]));
		const auto gridSnappedAngle = m_angleSnapping != 0.0F ? glm::radians(float(int((angle + (m_angleSnapping / 2.0F)) / m_angleSnapping) * m_angleSnapping)) : angle;
		const auto newQuat = glm::rotate(glm::quat(1, 0, 0, 0), gridSnappedAngle, normals[index]);
		if (m_selectedAxes == X_AXIS)
			m_startingAngle = glm::atan(m_startPoint.z - position.z, m_startPoint.y - position.y);
		else if (m_selectedAxes == Y_AXIS)
			m_startingAngle = glm::atan(m_startPoint.x - position.x, m_startPoint.z - position.z);
		else if (m_selectedAxes == Z_AXIS)
			m_startingAngle = glm::atan(m_startPoint.y - position.y, m_startPoint.x - position.x);
		else if (m_selectedAxes == ALL_AXES) {
			auto t1 = glm::inverse(fourthAxisMat) * glm::vec4(m_startPoint, 1);
			t1 /= t1.w;
			auto t2 = glm::inverse(fourthAxisMat) * glm::vec4(position, 1);
			t2 /= t2.w;
			m_startingAngle = glm::atan(t1.y - t2.y, t1.x - t2.x);
		}
		m_deltaAngle = glm::degrees(gridSnappedAngle);

		struct Rotate_Selection_Command final : Editor_Command {
			Engine& m_engine;
			LevelEditor_Module& m_editor;
			glm::quat& m_startingOrientation;
			glm::quat m_oldRotation, m_newRotation;
			const unsigned int m_axis = NONE;
			const std::vector<EntityHandle> m_uuids;
			Rotate_Selection_Command(Engine& engine, LevelEditor_Module& editor, glm::quat& oldRotation, const glm::quat& newRotation, const unsigned int& axis)
				: m_engine(engine), m_editor(editor), m_startingOrientation(oldRotation),  m_oldRotation(oldRotation), m_newRotation(newRotation), m_axis(axis), m_uuids(m_editor.getSelection()) {}
			void rotate(const glm::quat& rotation) {
				const auto& ecsWorld = m_editor.getWorld();
				std::vector<Transform_Component*> transformComponents;
				glm::vec3 center(0.0F);
				for (const auto& entityHandle : m_uuids)
					if (auto* transform = ecsWorld.getComponent<Transform_Component>(entityHandle)) {
						transformComponents.push_back(transform);
						center += transform->m_localTransform.m_position;
					}
				center /= transformComponents.size();
				for (auto* transform : transformComponents) {
					const auto delta = transform->m_localTransform.m_position - center;
					auto rotatedDelta = glm::mat4_cast(rotation) * glm::vec4(delta, 1.0F);
					rotatedDelta /= rotatedDelta.w;
					transform->m_localTransform.m_position = glm::vec3(rotatedDelta) + center;
					transform->m_localTransform.m_orientation = rotation * transform->m_localTransform.m_orientation;
					transform->m_localTransform.update();
				}
			}
			void execute() final {
				rotate(m_newRotation * glm::inverse(m_oldRotation));
				m_startingOrientation = m_newRotation;
			}
			void undo() final {
				rotate(glm::inverse(m_newRotation) * m_oldRotation);
				m_startingOrientation = m_oldRotation;
			}
			bool join(Editor_Command* other) final {
				if (const auto& newCommand = dynamic_cast<Rotate_Selection_Command*>(other)) {
					if (m_axis == newCommand->m_axis && std::equal(m_uuids.cbegin(), m_uuids.cend(), newCommand->m_uuids.cbegin())) {
						m_newRotation = newCommand->m_newRotation;
						return true;
					}
				}
				return false;
			}
		};
		m_editor.doReversableAction(std::make_shared<Rotate_Selection_Command>(m_engine, m_editor, m_startingOrientation, newQuat, m_selectedAxes));
		return true;
	}

	return false;
}

void Rotation_Gizmo::updateDisk()
{
	const auto fourthAxisMat = glm::inverse(glm::mat4_cast(glm::quat_cast(m_engine.getModule_Graphics().getClientCamera()->vMatrix)));
	const int steps = static_cast<int>(ceilf((abs(m_deltaAngle) / 360.0F) * DISK_VERTICES));
	std::vector<glm::vec3> points(size_t(steps) * 6ULL, glm::vec3(0.0F));
	for (size_t n = 0ULL, v = 0ULL; n < steps; ++n, v += 6ULL) {
		const auto startAngle = glm::radians(float(n) * (m_deltaAngle / float(steps))) + m_startingAngle;
		const auto endAngle = glm::radians(float(n + 1) * (m_deltaAngle / float(steps))) + m_startingAngle;

		const auto x1 = DISK_RADIUS * cosf(startAngle);
		const auto y1 = DISK_RADIUS * sinf(startAngle);
		const auto x2 = DISK_RADIUS * cosf(endAngle);
		const auto y2 = DISK_RADIUS * sinf(endAngle);

		glm::vec3 v1(0.0F);
		glm::vec3 v2(0.0F);
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
		else if (m_selectedAxes == ALL_AXES) {
			const auto t1 = fourthAxisMat * glm::vec4(x1, y1, 0, 1.0F);
			const auto t2 = fourthAxisMat * glm::vec4(x2, y2, 0, 1.0F);
			v1 = glm::vec3(t1 / t1.w);
			v2 = glm::vec3(t2 / t2.w);
		}
		points[v + 0ULL] = glm::vec3(0);
		points[v + 1ULL] = v1;
		points[v + 2ULL] = v2;
		points[v + 3ULL] = glm::vec3(0);
		points[v + 4ULL] = v2;
		points[v + 5ULL] = v1;
	}
	m_indirectDisk.setCount(GLuint(points.size()));
	glNamedBufferSubData(m_diskVBO, 0, sizeof(glm::vec3) * points.size(), points.data());
}