/*
	ECS_Messages

	- All of the types of messages can be stored here
*/

#pragma once
#ifndef ECS_MESSAGES
#define ECS_MESSAGES
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#define EXPIMP_TEMPLATE
#else
#define	DELTA_CORE_API __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
#endif

#include "Systems\ECS\ECSmessage.h"
#include "Utilities\Transform.h"
#include <string>

using namespace std;
using namespace glm;

template<typename T>
class DELTA_CORE_API ECSmessage_payload : public ECSmessage
{
public:
	~ECSmessage_payload() {};
	ECSmessage_payload(const T &data) : m_payload(data) {};
	virtual const char* GetTypeID() {
		return typeid(T).name();
	}
	T GetPayload() const { return m_payload; };
	void SetPayload(const T &data) { m_payload = data; };

protected:
	T m_payload;
};


// Give useful names for the specific payload types
typedef ECSmessage_payload<string>			ECS_Payload_Set_Model_Dir;
typedef ECSmessage_payload<Transform>		ECS_Payload_Set_Transform;
typedef ECSmessage_payload<vec3>			ECS_Payload_Set_Position;
typedef ECSmessage_payload<quat>			ECS_Payload_Set_Orientation;
typedef ECSmessage_payload<vec3>			ECS_Payload_Set_Scale;
typedef ECSmessage_payload<vec3>			ECS_Payload_Set_Light_Color;
typedef ECSmessage_payload<float>			ECS_Payload_Set_Light_Intensity;


// Lookup Codes
static const char* ofTypeInt		= typeid(int).name();
static const char* ofTypeDouble		= typeid(double).name();
static const char* ofTypeFloat		= typeid(float).name();
static const char* ofTypeVec2		= typeid(vec2).name();
static const char* ofTypeVec3		= typeid(vec3).name();
static const char* ofTypeVec4		= typeid(vec4).name();
static const char* ofTypeIVec2		= typeid(ivec2).name();
static const char* ofTypeIVec3		= typeid(ivec3).name();
static const char* ofTypeIVec4		= typeid(ivec4).name();
static const char* ofTypeQuat		= typeid(quat).name();
static const char* ofTypeString		= typeid(string).name();
static const char* ofTypeTransform	= typeid(Transform).name();


// Explicitly declare the supported payload types ahead of time so they can export
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<int>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<double>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<float>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<vec2>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<vec3>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<vec4>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<ivec2>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<ivec3>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<ivec4>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<quat>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<string>;
EXPIMP_TEMPLATE template class DELTA_CORE_API ECSmessage_payload<Transform>;

#endif // ECS_MESSAGES