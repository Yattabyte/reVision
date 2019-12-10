#pragma once
#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "glm/glm.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/normal.hpp"


/** Test if a ray intersects a plane.
@param	ray_origin				the ray origin point.
@param	ray_direction			the ray direction vector.
@param	plane_origin			the plane origin point.
@param	plane_normal			the plane normal vector.
@param	intersectionDistance	reference updated with the intersection distance from the ray origin.
@return							true on intersection, false otherwise. */
inline static bool RayPlaneIntersection(
	const glm::vec3& ray_origin,
	const glm::vec3& ray_direction,
	const glm::vec3& plane_origin,
	const glm::vec3& plane_normal,
	float& intersectionDistance
) noexcept {
	return glm::intersectRayPlane(ray_origin, ray_direction, plane_origin, plane_normal, intersectionDistance);
}

/** Test if a ray intersects a triangle.
@param	ray_origin				the ray origin point.
@param	ray_direction			the ray direction vector.
@param	v0						the first triangle vertex point.
@param	v1						the second triangle vertex point.
@param	v2						the third triangle vertex point.
@param	normal					reference updated with the triangle vector.
@param	baryPos					reference updated with the barometric position of the intersection.
@param	intersectionDistance	reference updated with the intersection distance from the ray origin.
@return							true on intersection, false otherwise. */
inline static bool RayTriangleIntersection(
	const glm::vec3& ray_origin,
	const glm::vec3& ray_direction,
	const glm::vec3& v0,
	const glm::vec3& v1,
	const glm::vec3& v2,
	glm::vec3& normal,
	glm::vec2& baryPos,
	float& intersectionDistance
) noexcept {
	const bool result = glm::intersectRayTriangle(ray_origin, ray_direction, v0, v1, v2, baryPos, intersectionDistance);
	if (result) {
		normal = -glm::triangleNormal(v0, v1, v2);
		return true;
	}
	return false;
}

/** Test if a ray intersects an Object Oriented Bounding Box.
@param	ray_origin				the ray origin point.
@param	ray_direction			the ray direction vector.
@param	aabb_min				the minimum bounds of the box.
@param	aabb_max				the maximum bounds of the box.
@param	ModelMatrix				the transformation matrix for the OOBB.
@param	intersectionDistance	reference updated with the intersection distance from the ray origin.
@return							true on intersection, false otherwise. */
inline static bool RayOOBBIntersection(
	const glm::vec3& ray_origin,
	const glm::vec3& ray_direction,
	const glm::vec3& aabb_min,
	const glm::vec3& aabb_max,
	const glm::mat4& ModelMatrix,
	float& intersection_distance
) noexcept {
	float tMin = 0.0f;
	float tMax = 100000.0f;
	const glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);
	const glm::vec3 delta = OBBposition_worldspace - ray_origin;

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
		const glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
		const float e = glm::dot(xaxis, delta);
		const float f = glm::dot(ray_direction, xaxis);

		if (fabs(f) > 0.001f) { // Standard case
			float t1 = (e + aabb_min.x) / f; // Intersection with the "left" plane
			float t2 = (e + aabb_max.x) / f; // Intersection with the "right" plane
			// t1 and t2 now contain distances between ray origin and ray-plane intersections

			// We want t1 to represent the nearest intersection,
			// so if it's not the case, invert t1 and t2
			if (t1 > t2)
				std::swap(t1, t2);

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
		const glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
		const float e = glm::dot(yaxis, delta);
		const float f = glm::dot(ray_direction, yaxis);

		if (fabs(f) > 0.001f) {
			float t1 = (e + aabb_min.y) / f;
			float t2 = (e + aabb_max.y) / f;

			if (t1 > t2)
				std::swap(t1, t2);

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
		const glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
		const float e = glm::dot(zaxis, delta);
		const float f = glm::dot(ray_direction, zaxis);

		if (fabs(f) > 0.001f) {
			float t1 = (e + aabb_min.z) / f;
			float t2 = (e + aabb_max.z) / f;

			if (t1 > t2) 
				std::swap(t1, t2);			

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
}

/** Test if a ray intersects a sphere.
@param	ray_origin				the ray origin point.
@param	ray_direction			the ray direction vector.
@param	center					the center position of the sphere.
@param	radius					the radius of the sphere.
@return							true on intersection, false otherwise. */
inline static float RaySphereIntersection(
	const glm::vec3& ray_origin,
	const glm::vec3& ray_direction,
	const glm::vec3& center,
	const float& radius
) noexcept {
	const auto oc = ray_origin - center;
	const auto a = glm::dot(ray_direction, ray_direction);
	const auto b = 2.0f * glm::dot(oc, ray_direction);
	const auto c = glm::dot(oc, oc) - radius * radius;
	const auto discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f)
		return -1.0f;
	else {
		float numerator = -b - glm::sqrt(discriminant);
		if (numerator > 0.0)
			return numerator / (2.0f * a);
		numerator = -b + glm::sqrt(discriminant);
		if (numerator > 0.0)
			return numerator / (2.0f * a);
		else
			return -1.0f;
	}
};

#endif // INTERSECTION_H