#include "Utilities\Frustum.h"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtc\matrix_access.hpp"

#define ANG2RAD 3.14159265358979323846/180.0

Plane::~Plane()
{
}

Plane::Plane()
{
}

Plane::Plane(const Plane &p)
{
	normal = p.normal;
	point = p.point;
	d = p.d;
}

Plane::Plane(vec3 &v1, vec3 &v2, vec3 &v3) {

	set3Points(v1, v2, v3);
}

void Plane::set3Points(vec3 &v1, vec3 &v2, vec3 &v3)
{
	vec3 aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	normal = aux2 * aux1;

	normal = glm::normalize(normal);
	point = (v2);

	d = -glm::dot(normal, point);
}

void Plane::setNormalAndPoint(vec3 &normal, vec3 &point)
{

	this->normal = (normal);
	this->normal = glm::normalize(normal);
	d = -glm::dot(this->normal, point);
}

void Plane::setCoefficients(const vec4 &plane)
{
	normal = vec3(plane.x, plane.y, plane.z);
	float length = glm::length(normal);
	normal /= length;
	values = vec4(plane.x, plane.y, plane.z, plane.w);
	values = values / length;
	this->d = values.w;
}

float Plane::distance(vec3 &p)
{
	return (d + glm::dot(normal, p));
}

Frustum::~Frustum()
{
}

Frustum::Frustum()
{
}

Frustum::Frustum(const Frustum &f)
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

Frustum::Frustum(const mat4 &m)
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

int Frustum::sphereInFrustum(vec3 &p, vec3 radius)
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

vec3 getVertexP(const vec3 &bbox_min, const vec3 &normal)
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

vec3 getVertexN(const vec3 &bbox_max, const vec3 &normal)
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

bool Frustum::AABBInFrustom(const vec3 &bbox_min, const vec3 &bbox_max)
{
	const vec3 box[] = { bbox_min, bbox_max };

	static const int NUM_PLANES = 6;

	for (int i = 0; i < NUM_PLANES; ++i) {
		const Plane &p = pl[i];

		const int px = static_cast<int>(p.values[0] > 0.0f);
		const int py = static_cast<int>(p.values[1] > 0.0f);
		const int pz = static_cast<int>(p.values[2] > 0.0f);

		const float dp = (p.values[0] * box[px].x) + (p.values[1] * box[py].x) + (p.values[2] * box[pz].x);
		if (dp < -p.values[3])
			return false;
	}
	return true;
}


void Frustum::setCamInternals(float angle, float ratio, float nearD, float farD)
{
	// store the information
	this->ratio = ratio;
	this->angle = angle;
	this->nearD = nearD;
	this->farD = farD;

	// compute width and height of the near and far plane sections
	tang = (float)tan(ANG2RAD * angle * 0.5);
	nh = nearD * tang;
	nw = nh * ratio;
	fh = farD  * tang;
	fw = fh * ratio;
}

void Frustum::setCamDef(vec3 &p, vec3 &l, vec3 &u)
{
	vec3 dir, nc, fc, X, Y, Z;

	// compute the Z axis of camera
	// this axis points in the opposite direction from
	// the looking direction
	Z = p - l;
	Z = glm::normalize(Z);

	// X axis of camera with given "up" vector and Z axis
	X = u * Z;
	X = glm::normalize(X);

	// the real "up" vector is the cross product of Z and X
	Y = Z * X;

	// compute the centers of the near and far planes
	nc = p - Z * nearD;
	fc = p - Z * farD;

	pl[NEARP].setNormalAndPoint(-Z, nc);
	pl[FARP].setNormalAndPoint(Z, fc);

	vec3 aux, normal;

	aux = (nc + Y*nh) - p;
	aux = glm::normalize(aux);
	normal = aux * X;
	pl[TOP].setNormalAndPoint(normal, nc + Y*nh);

	aux = (nc - Y*nh) - p;
	aux = glm::normalize(aux);
	normal = X * aux;
	pl[BOTTOM].setNormalAndPoint(normal, nc - Y*nh);

	aux = (nc - X*nw) - p;
	aux = glm::normalize(aux);
	normal = aux * Y;
	pl[LEFT].setNormalAndPoint(normal, nc - X*nw);

	aux = (nc + X*nw) - p;
	aux = glm::normalize(aux);
	normal = Y * aux;
	pl[RIGHT].setNormalAndPoint(normal, nc + X*nw);
}


