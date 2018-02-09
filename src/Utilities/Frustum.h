/*
	Frustum

	- Represents the viewing frustum, used for visibility calculations
*/

#pragma once
#ifndef FRUSTUM
#define FRUSTUM
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "glm\glm.hpp"

using namespace glm;

class Plane
{
public:
	vec3 normal, point;
	float d;
	vec4 values;
	Plane();
	Plane(const Plane&);
	Plane(vec3 &v1, vec3 &v2, vec3 &v3);
	~Plane();

	void set3Points(vec3 &v1, vec3 &v2, vec3 &v3);
	void setNormalAndPoint(vec3 &normal, vec3 &point);
	void setCoefficients(const vec4 & plane);
	float distance(vec3 &p);
};

class Frustum
{
public:
	Frustum();
	Frustum(const Frustum&);
	Frustum(const mat4 &m);
	~Frustum();

private:
	enum {
		TOP = 0, BOTTOM, LEFT,
		RIGHT, NEARP, FARP
	};

public:
	enum { OUTSIDE, INTERSECT, INSIDE };
	Plane pl[6];
	vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float nearD, farD, ratio, angle, tang;
	float nw, nh, fw, fh;

	void setFrustum(const mat4 &m);
	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(vec3 &p, vec3 &l, vec3 &u);
	int sphereInFrustum(vec3 &p, vec3 raio);
	bool AABBInFrustom(const vec3 & bbox_min, const vec3 & bbox_max);
};

#endif // FRUSTUM