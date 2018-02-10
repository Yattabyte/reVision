#include "Utilities\Frustum.h"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtc\matrix_access.hpp"

#define ANG2RAD 3.14159265358979323846/180.0

Frustum::Plane::~Plane()
{
}

Frustum::Plane::Plane()
{
}

Frustum::Plane::Plane(const Plane & p)
{
	m_normal = p.m_normal;
	m_point = p.m_point;
	m_d = p.m_d;
	m_values = p.m_values;
}

Frustum::Plane::Plane(const vec3 & v1, const vec3 & v2, const vec3 & v3) {

	set3Points(v1, v2, v3);
}

void Frustum::Plane::set3Points(const vec3 & v1, const vec3 & v2, const vec3 & v3)
{
	vec3 aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	m_normal = aux2 * aux1;

	m_normal = glm::normalize(m_normal);
	m_point = (v2);

	m_d = -glm::dot(m_normal, m_point);
}

void Frustum::Plane::setNormalAndPoint(const vec3 & normal, const vec3 & point)
{
	m_normal = (normal);
	m_normal = glm::normalize(normal);
	m_d = -glm::dot(m_normal, point);
}

void Frustum::Plane::setCoefficients(const vec4 & plane)
{
	m_normal = vec3(plane.x, plane.y, plane.z);
	const float length = glm::length(m_normal);
	m_normal /= length;
	m_values = vec4(plane.x, plane.y, plane.z, plane.w);
	m_values = m_values / length;
	m_d = m_values.w;
}

float Frustum::Plane::distance(const vec3 & p)
{
	return (m_d + glm::dot(m_normal, p));
}

Frustum::~Frustum()
{
}

Frustum::Frustum()
{
}

Frustum::Frustum(const Frustum & f)
{
	for (int x = 0; x < 6; x++)
		pl[x] = f.pl[x];
	ntl = f.ntl;
	ntr = f.ntr;
	nbl = f.nbl;
	nbr = f.nbr;
	ftl = f.ftl;
	ftr = f.ftr;
	fbl = f.fbl;
	fbr = f.fbr;
	nearD = f.nearD;
	farD = f.farD;
	ratio = f.ratio;
	angle = f.angle;
	tang = f.tang;
	nw = f.nw;
	nh = f.nh;
	fw = f.fw;
	fh = f.fh;
}

Frustum::Frustum(const mat4 & m)
{
	setFrustum(m);
}

void Frustum::setFrustum(const mat4 &m)
{
	vec4 rowX = glm::row(m, 0);
	vec4 rowY = glm::row(m, 1);
	vec4 rowZ = glm::row(m, 2);
	vec4 rowW = glm::row(m, 3);

	pl[0].setCoefficients(glm::normalize(rowW + rowX));
	pl[1].setCoefficients(glm::normalize(rowW - rowX));
	pl[2].setCoefficients(glm::normalize(rowW + rowY));
	pl[3].setCoefficients(glm::normalize(rowW - rowY));
	pl[4].setCoefficients(glm::normalize(rowW + rowZ));
	pl[5].setCoefficients(glm::normalize(rowW - rowZ));
}

int Frustum::sphereInFrustum(const vec3 & p, const vec3 & radius)
{
	float distance;
	int result = INSIDE;

	float desiredRadius = 0.0f;
	float fabX = fabs(radius.x), fabY = fabs(radius.y), fabZ = fabs(radius.z);
	desiredRadius = (fabX > desiredRadius) ? fabX : desiredRadius;
	desiredRadius = (fabY > desiredRadius) ? fabY : desiredRadius;
	desiredRadius = (fabZ > desiredRadius) ? fabZ : desiredRadius;

	for (int i = 0; i < 6; i++) {
		distance = pl[i].distance(p);
		if (distance < -desiredRadius)
			return OUTSIDE;
		else if (distance < desiredRadius)
			result = INTERSECT;
	}
	return(result);
}

vec3 getVertexP(const vec3 & bbox_min, const vec3 & normal)
{
	vec3 p = bbox_min;
	if (normal.x >= 0)
		p.x = bbox_min.x;
	if (normal.y >= 0)
		p.y = bbox_min.y;
	if (normal.z >= 0)
		p.z = bbox_min.z;
	return p;
}

vec3 getVertexN(const vec3 & bbox_max, const vec3 & normal)
{
	vec3 n = bbox_max;
	if (normal.x >= 0)
		n.x = bbox_max.x;
	if (normal.y >= 0)
		n.y = bbox_max.y;
	if (normal.z >= 0)
		n.z = bbox_max.z;
	return n;
}

int Frustum::AABBInFrustom(const vec3 & bbox_min, const vec3 & bbox_max)
{
	const vec3 box[] = { bbox_min, bbox_max };

	static const int NUM_PLANES = 6;

	for (int i = 0; i < NUM_PLANES; ++i) {
		const Plane &p = pl[i];

		const int px = static_cast<int>(p.m_values[0] > 0.0f);
		const int py = static_cast<int>(p.m_values[1] > 0.0f);
		const int pz = static_cast<int>(p.m_values[2] > 0.0f);

		const float dp = (p.m_values[0] * box[px].x) + (p.m_values[1] * box[py].x) + (p.m_values[2] * box[pz].x);
		if (dp < -p.m_values[3])
			return OUTSIDE;
	}
	return INSIDE;
}