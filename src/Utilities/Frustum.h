#pragma once
#ifndef FRUSTUM
#define FRUSTUM
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "glm\glm.hpp"

using namespace glm;


/**
 * A viewing frustum object. 
 * Used in visibility calculations, this object takes in a mat4 and generates planes for intersection tests.
 **/
class Frustum
{
private:
	/** 
	 * Nested plane class
	 **/
	static class Plane 
	{
	public:
		// (de)Constructors
		~Plane();
		Plane();
		Plane(const Plane&);
		Plane(const vec3 & v1, const vec3 & v2, const vec3 & v3);

		// Public Methods
		void set3Points(const vec3 & v1, const vec3 & v2, const vec3 & v3);
		void setNormalAndPoint(const vec3 & normal, const vec3 & point);
		void setCoefficients(const vec4 & plane);
		float distance(const vec3 & p);

		// Public Attributes
		float m_d;
		vec3 m_normal, m_point;
		vec4 m_values;
	};
	static const enum {
		TOP = 0, BOTTOM, LEFT,
		RIGHT, NEARP, FARP
	};

public:
	/** Enumerations for testing intersections. */
	static const enum { OUTSIDE, INTERSECT, INSIDE };


	// (de)Constructors
	/** Destroys the frustum */
	~Frustum();
	/** Zero-Initialize the frustum */
	Frustum();
	/** Deep-copy another frustum */
	Frustum(const Frustum &);
	/** Construct a frustum from a perspective-projection. */
	Frustum(const mat4 & m);


	// Public Methods
	/** Sets a matrix for this frustum to be based upon.
	 * @param	m			the perspective-projection matrix to use */
	void setFrustum(const mat4 & m);
	/** Tests if a supplied sphere is visible within this frustum. 
	 * @param	p			the position of the sphere
	 * @param	radius		the radius of the sphere
	 * @return				integer representing outside, intersect, or inside (0-2)
	 * @note				see this class's public enumeration */
	int sphereInFrustum(const vec3 & p, const vec3 & radius);
	/** Tests if a supplied axis-aligned bounding-box is visible within this frustum.
	 * @param	bbox_min	the minimum extents of this box
	 * @param	bbox_max	the maximum extents of this box
	 * @return				integer representing outside, intersect, or inside(0 - 2)
	 * @note				see this class's public enumeration */
	int AABBInFrustom(const vec3 & bbox_min, const vec3 & bbox_max);

	
private:
	// Private Attributes
	Plane pl[6];
	vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float nearD, farD, ratio, angle, tang;
	float nw, nh, fw, fh;
};

#endif // FRUSTUM