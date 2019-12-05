#pragma once
#ifndef MOUSEPICKER_SYSTEM_H
#define MOUSEPICKER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Utilities/Intersection.h"
#include "Engine.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"


/** An ECS system allowing the user to ray-pick entities by selecting against their components. */
class MousePicker_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	~MousePicker_System() noexcept;
	/** Construct this system.
	@param	engine		reference to the engine to use. */
	explicit MousePicker_System(Engine& engine)  noexcept;


	// Public Interface Implementation
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


	// Public Methods
	/** Retrieve this system's last selection result.
	@return							the last selection result. */
	std::tuple<EntityHandle, Transform, Transform> getSelection() const noexcept;


private:
	// Private Methods
	/** Perform a ray-prop intersection test.
	@param	transformComponent		the transform component of interest.
	@param	prop					the prop component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	normal					reference updated with the intersection normal.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@return							true on successful intersection, false if disjoint. */
	static bool RayProp(Transform_Component* transformComponent, Prop_Component* prop, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, glm::vec3& normal, float& distanceFromScreen, int& confidence) noexcept;
	/** Perform a ray-collider intersection test.
	@param	transformComponent		the transform component of interest.
	@param	collider				the collider component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@return							true on successful intersection, false if disjoint. */
	static bool RayCollider(Collider_Component* collider, const void* const closestPhysicsShape, const float& closetstPhysicsHit, float& distanceFromScreen, int& confidence) noexcept;
	/** Perform a ray-bounding-box intersection test.
	@param	transformComponent		the transform component of interest.
	@param	bBox					the bounding box component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@return							true on successful intersection, false if disjoint. */
	static bool RayBBox(Transform_Component* transformComponent, BoundingBox_Component* bBox, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) noexcept;
	/** Perform a ray-bounding-sphere intersection test.
	@param	transformComponent		the transform component of interest.
	@param	bSphere					the bounding sphere component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@return							true on successful intersection, false if disjoint. */
	static bool RayBSphere(Transform_Component* transformComponent, BoundingSphere_Component* bSphere, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence) noexcept;
	/** Perform a ray-origin intersection test.
	@param	transformComponent		the transform component of interest.
	@param	ray_origin				the mouse ray's origin.
	@param	ray_direction			the mouse ray's direction.
	@param	distanceFromScreen		reference updated with the distance of the intersection point to the screen.
	@param	confidence				reference updated with the confidence level for this function.
	@param	engine					reference to the engine to use. 
	@return							true on successful intersection, false if disjoint. */
	static bool RayOrigin(Transform_Component* transformComponent, const glm::vec3& ray_origin, const glm::highp_vec3& ray_direction, float& distanceFromScreen, int& confidence, Engine& engine) noexcept;


	// Private Attributes
	Engine& m_engine;
	EntityHandle m_selection;
	Transform m_selectionTransform, m_intersectionTransform;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // MOUSEPICKER_SYSTEM_H