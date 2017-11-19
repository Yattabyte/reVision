/*
	ECS_Messages

	- All of the types of messages can be stored here
*/

#pragma once
#ifndef ECS_MESSAGES
#define ECS_MESSAGES
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

enum ECS_MSG_TYPES
{
	SET_MODEL_DIR,
	SET_TRANSFORM,
	SET_POSITION,
	SET_ORIENTATION,
	SET_SCALE,
	SET_LIGHT_COLOR,
	SET_LIGHT_INTENSITY,
};

#include "Systems\ECS\ECSMessage.h"
#include "Utilities\Transform.h"
#include <string>

using namespace std;
using namespace glm;

class MSG_Set_Model_Dir : public ECSMessage
{
public:
	DELTA_CORE_API ~MSG_Set_Model_Dir() {};
	DELTA_CORE_API MSG_Set_Model_Dir(const string &data) : m_payload(data) {
		m_typeID = SET_MODEL_DIR;
	};
	DELTA_CORE_API string GetPayload() const { return m_payload; };
	DELTA_CORE_API void SetPayload(const string &data) { m_payload = data; };
	
protected:
	string m_payload;
};

class MSG_Set_Transform : public ECSMessage
{
public:
	DELTA_CORE_API ~MSG_Set_Transform() {};
	DELTA_CORE_API MSG_Set_Transform(const Transform &data) : m_payload(data) {
		m_typeID = SET_TRANSFORM;
	};
	DELTA_CORE_API Transform GetPayload() const { return m_payload; };
	DELTA_CORE_API void SetPayload(const Transform &data) { m_payload = data; };

protected:
	Transform m_payload;
};

class MSG_Set_Position : public ECSMessage
{
public:
	DELTA_CORE_API ~MSG_Set_Position() {};
	DELTA_CORE_API MSG_Set_Position(const vec3 &data) : m_payload(data) {
		m_typeID = SET_POSITION;
	};
	DELTA_CORE_API vec3 GetPayload() const { return m_payload; };
	DELTA_CORE_API void SetPayload(const vec3 &data) { m_payload = data; };

protected:
	vec3 m_payload;
};

class MSG_Set_Orientation : public ECSMessage
{
public:
	DELTA_CORE_API ~MSG_Set_Orientation() {};
	DELTA_CORE_API MSG_Set_Orientation(const quat &data) : m_payload(data) {
		m_typeID = SET_ORIENTATION;
	};
	DELTA_CORE_API quat GetPayload() const { return m_payload; };
	DELTA_CORE_API void SetPayload(const quat &data) { m_payload = data; };

protected:
	quat m_payload;
};

class MSG_Set_Scale : public ECSMessage
{
public:
	DELTA_CORE_API ~MSG_Set_Scale() {};
	DELTA_CORE_API MSG_Set_Scale(const vec3 &data) : m_payload(data) {
		m_typeID = SET_SCALE;
	};
	DELTA_CORE_API vec3 GetPayload() const { return m_payload; };
	DELTA_CORE_API void SetPayload(const vec3 &data) { m_payload = data; };

protected:
	vec3 m_payload;
};

class MSG_Set_Light_Color : public ECSMessage
{
public:
	DELTA_CORE_API ~MSG_Set_Light_Color() {};
	DELTA_CORE_API MSG_Set_Light_Color(const vec3 &data) : m_payload(data) {
		m_typeID = SET_LIGHT_COLOR;
	};
	DELTA_CORE_API vec3 GetPayload() const { return m_payload; };
	DELTA_CORE_API void SetPayload(const vec3 &data) { m_payload = data; };

protected:
	vec3 m_payload;
};

class MSG_Set_Light_Intensity : public ECSMessage
{
public:
	DELTA_CORE_API ~MSG_Set_Light_Intensity() {};
	DELTA_CORE_API MSG_Set_Light_Intensity(const float &data) : m_payload(data) {
		m_typeID = SET_LIGHT_INTENSITY;
	};
	DELTA_CORE_API float GetPayload() const { return m_payload; };
	DELTA_CORE_API void SetPayload(const float &data) { m_payload = data; };

protected:
	float m_payload;
};

#endif // ECS_MESSAGES