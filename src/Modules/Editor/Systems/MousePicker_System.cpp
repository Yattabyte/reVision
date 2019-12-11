#include "Modules/Editor/Systems/MousePicker_System.h"
#include "Modules/ECS/component_types.h"
#include "Utilities/Intersection.h"
#include "Engine.h"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"


// Forward Declarations
static bool RayProp(const Transform_Component* transformComponent, Prop_Component* prop, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, glm::vec3& normal, float& distanceFromScreen, int& confidence) noexcept;
static bool RayCollider(const Collider_Component* collider, const void* const closestPhysicsShape, const float& closetstPhysicsHit, float& distanceFromScreen, int& confidence) noexcept;
static bool RayBBox(Transform_Component* transformComponent, const BoundingBox_Component* bBox, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) noexcept;
static bool RayBSphere(const Transform_Component* transformComponent, const BoundingSphere_Component* bSphere, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) noexcept;
static bool RayOrigin(const Transform_Component* transformComponent, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence, Engine& engine) noexcept;

MousePicker_System::~MousePicker_System() noexcept 
{
	// Update indicator
	*m_aliveIndicator = false;
}

MousePicker_System::MousePicker_System(Engine& engine) noexcept :
	m_engine(engine)
{
	// Declare component types used
	addComponentType(Transform_Component::Runtime_ID);
	addComponentType(BoundingBox_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
	addComponentType(BoundingSphere_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
	addComponentType(Collider_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);
	addComponentType(Prop_Component::Runtime_ID, RequirementsFlag::FLAG_OPTIONAL);

	// Preferences
	auto& preferences = m_engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.x = (int)f;
		});
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) noexcept {
		m_renderSize.y = (int)f;
		});
}

void MousePicker_System::updateComponents(const float&, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept 
{
	const auto& actionState = m_engine.getActionState();
	const auto& clientCamera = m_engine.getModule_Graphics().getClientCamera();
	const auto ray_origin = clientCamera->EyePosition;
	const auto ray_nds = glm::vec2(2.0f * actionState[ActionState::Action::MOUSE_X] / m_renderSize.x - 1.0f, 1.0f - (2.0f * actionState[ActionState::Action::MOUSE_Y]) / m_renderSize.y);
	const auto ray_eye = glm::vec4(glm::vec2(clientCamera->pMatrixInverse * glm::vec4(ray_nds, -1.0f, 1.0F)), -1.0f, 0.0f);
	const auto ray_direction = glm::normalize(glm::vec3(clientCamera->vMatrixInverse * ray_eye));

	// Set the selection position for the worst-case scenario
	const auto ray_end = ray_origin + (ray_direction * glm::vec3(50.0f));
	const auto ray_end_far = ray_origin + (ray_direction * clientCamera->FarPlane);
	m_selectionTransform.m_position = ray_end;
	m_intersectionTransform.m_position = ray_end;

	const btVector3 origin(ray_origin.x, ray_origin.y, ray_origin.z);
	const btVector3 direction(ray_end_far.x, ray_end_far.y, ray_end_far.z);
	btCollisionWorld::ClosestRayResultCallback closestResults(origin, direction);
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
	m_engine.getModule_Physics().getWorld().rayTest(origin, direction, closestResults);
	void* closestPhysicsShape = nullptr;
	float closetstPhysicsHit = FLT_MAX;
	glm::vec3 intersectionNormal(0, 1, 0);
	if (closestResults.hasHit()) {
		const auto p = origin.lerp(direction, closestResults.m_closestHitFraction);
		closetstPhysicsHit = glm::distance(ray_origin, glm::vec3(p.x(), p.y(), p.z()));
		// We won't change this at all, we just need the pointer address
		closestPhysicsShape = const_cast<btCollisionShape*>(closestResults.m_collisionObject->getCollisionShape());
		intersectionNormal = glm::vec3(closestResults.m_hitNormalWorld.x(), closestResults.m_hitNormalWorld.y(), closestResults.m_hitNormalWorld.z());
	}

	float closestDistance = FLT_MAX;
	int highestConfidence = 0;
	bool found = false;
	for (const auto& componentParam : components) {
		auto* transformComponent = static_cast<Transform_Component*>(componentParam[0]);
		auto* bBox = static_cast<BoundingBox_Component*>(componentParam[1]);
		auto* bSphere = static_cast<BoundingSphere_Component*>(componentParam[2]);
		auto* collider = static_cast<Collider_Component*>(componentParam[3]);
		auto* prop = static_cast<Prop_Component*>(componentParam[4]);
		float distanceFromScreen = FLT_MAX;
		int confidence = 0;
		const bool hasBoundingShape = bSphere || bBox;
		bool result = false;

		// Attempt cheap tests first
		result = RayOrigin(transformComponent, ray_origin, ray_direction, distanceFromScreen, confidence, m_engine);
		if (bBox)
			result = RayBBox(transformComponent, bBox, ray_origin, ray_direction, distanceFromScreen, confidence);
		else if (bSphere)
			result = RayBSphere(transformComponent, bSphere, ray_origin, ray_direction, distanceFromScreen, confidence);

		// Attempt more complex tests
		if (collider && ((hasBoundingShape && result) || (!hasBoundingShape))) {
			distanceFromScreen = FLT_MAX;
			result = RayCollider(collider, closestPhysicsShape, closetstPhysicsHit, distanceFromScreen, confidence);
		}
		else if (prop && ((hasBoundingShape && result) || (!hasBoundingShape))) {
			distanceFromScreen = FLT_MAX;
			result = RayProp(transformComponent, prop, ray_origin, ray_direction, intersectionNormal, distanceFromScreen, confidence);
		}

		// Find the closest best match
		if (result && ((distanceFromScreen < closestDistance) || (confidence > highestConfidence))) {
			closestDistance = distanceFromScreen;
			highestConfidence = confidence;
			m_selection = transformComponent->m_entity;
			m_selectionTransform = transformComponent->m_worldTransform;
			found = true;
		}
	}
	if (found) {
		m_intersectionTransform.m_position = ray_origin + closestDistance * ray_direction;
		m_intersectionTransform.m_orientation = glm::orientation(intersectionNormal, glm::vec3(0, 1, 0));
		m_intersectionTransform.update();
	}
}

std::tuple<EntityHandle, Transform, Transform> MousePicker_System::getSelection() const noexcept
{
	return { m_selection, m_selectionTransform, m_intersectionTransform };
}

/** Perform a ray-prop intersection test.
@param	transformComponent		the transform component of interest.
@param	prop					the prop component of interest.
@param	ray_origin				the mouse ray's origin.
@param	ray_direction			the mouse ray's direction.
@param	normal					reference updated with the intersection normal.
@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
@param	confidence				reference updated with the confidence level for this function.
@return							true on successful intersection, false if disjoint. */
static bool RayProp(const Transform_Component* transformComponent, Prop_Component* prop, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, glm::vec3& normal, float& distanceFromScreen, int& confidence) noexcept 
{
	bool intersection = false;
	if (prop->m_model->ready()) {
		float distance = FLT_MAX;
		for (size_t x = 0; x < prop->m_model->m_data.m_vertices.size(); x += 3) {
			auto v0 = transformComponent->m_worldTransform.m_modelMatrix * glm::vec4(prop->m_model->m_data.m_vertices[x].vertex, 1);
			auto v1 = transformComponent->m_worldTransform.m_modelMatrix * glm::vec4(prop->m_model->m_data.m_vertices[x + 1].vertex, 1);
			auto v2 = transformComponent->m_worldTransform.m_modelMatrix * glm::vec4(prop->m_model->m_data.m_vertices[x + 2].vertex, 1);
			v0 /= v0.w;
			v1 /= v1.w;
			v2 /= v2.w;
			glm::vec2 unused;
			if (RayTriangleIntersection(
				ray_origin, ray_direction,
				glm::vec3(v0), glm::vec3(v1), glm::vec3(v2), normal, unused,
				distance
			)) {
				distanceFromScreen = distance;
				confidence = 3;
				intersection = true;
			}
		}
	}
	return intersection;
}

/** Perform a ray-collider intersection test.
@param	transformComponent		the transform component of interest.
@param	collider				the collider component of interest.
@param	ray_origin				the mouse ray's origin.
@param	ray_direction			the mouse ray's direction.
@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
@param	confidence				reference updated with the confidence level for this function.
@return							true on successful intersection, false if disjoint. */
static bool RayCollider(const Collider_Component* collider, const void* const closestPhysicsShape, const float& closetstPhysicsHit, float& distanceFromScreen, int& confidence) noexcept
{
	if (collider->m_shape && collider->m_shape.get() == closestPhysicsShape) {
		distanceFromScreen = closetstPhysicsHit;
		confidence = 3;
		return true;
	}
	return false;
}

/** Perform a ray-bounding-box intersection test.
@param	transformComponent		the transform component of interest.
@param	bBox					the bounding box component of interest.
@param	ray_origin				the mouse ray's origin.
@param	ray_direction			the mouse ray's direction.
@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
@param	confidence				reference updated with the confidence level for this function.
@return							true on successful intersection, false if disjoint. */
static bool RayBBox(Transform_Component* transformComponent, const BoundingBox_Component* bBox, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) noexcept
{
	float distance(FLT_MAX);
	const auto& scale = transformComponent->m_worldTransform.m_scale;
	Transform newTransform = transformComponent->m_worldTransform;
	newTransform.m_position += bBox->m_positionOffset;
	newTransform.m_scale = glm::vec3(1.0f);
	newTransform.update();
	const auto matrixWithoutScale = newTransform.m_modelMatrix;

	// Check if the distance is closer than the last entity found, so we can find the 'best' selection
	if (RayOOBBIntersection(ray_origin, ray_direction, bBox->m_min * scale, bBox->m_max * scale, matrixWithoutScale, distance)) {
		distanceFromScreen = distance;
		confidence = 2;
		return true;
	}
	return false;
}

/** Perform a ray-bounding-sphere intersection test.
@param	transformComponent		the transform component of interest.
@param	bSphere					the bounding sphere component of interest.
@param	ray_origin				the mouse ray's origin.
@param	ray_direction			the mouse ray's direction.
@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
@param	confidence				reference updated with the confidence level for this function.
@return							true on successful intersection, false if disjoint. */
static bool RayBSphere(const Transform_Component* transformComponent, const BoundingSphere_Component* bSphere, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) noexcept
{
	// Check if the distance is closer than the last entity found, so we can find the 'best' selection
	if (auto distance = RaySphereIntersection(ray_origin, ray_direction, transformComponent->m_worldTransform.m_position + bSphere->m_positionOffset, bSphere->m_radius); distance >= 0.0f) {
		distanceFromScreen = distance;
		confidence = 2;
		return true;
	}
	return false;
}

/** Perform a ray-origin intersection test.
@param	transformComponent		the transform component of interest.
@param	ray_origin				the mouse ray's origin.
@param	ray_direction			the mouse ray's direction.
@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
@param	confidence				reference updated with the confidence level for this function.
@param	engine					reference to the engine to use.
@return							true on successful intersection, false if disjoint. */
static bool RayOrigin(const Transform_Component* transformComponent, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence, Engine& engine) noexcept
{
	// Create scaling factor to keep all origins same screen size
	const auto radius = glm::distance(transformComponent->m_worldTransform.m_position, engine.getModule_Graphics().getClientCamera()->EyePosition) * 0.033f;

	// Check if the distance is closer than the last entity found, so we can find the 'best' selection
	if (auto distance = RaySphereIntersection(ray_origin, ray_direction, transformComponent->m_worldTransform.m_position, radius); distance >= 0.0f) {
		distanceFromScreen = distance;
		confidence = 2;
		return true;
	}
	return false;
}