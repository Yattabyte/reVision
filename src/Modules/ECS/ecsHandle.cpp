#include "Modules/ECS/ecsHandle.h"
#include <algorithm>


ecsHandle::ecsHandle(const char id[32])
{
	std::copy(&id[0], &id[32], &m_uuid[0]);
}

bool ecsHandle::operator==(const ecsHandle& other) const noexcept
{
	return bool(std::strncmp(m_uuid, other.m_uuid, 32ULL) == 0);
}

bool ecsHandle::operator<(const ecsHandle& other) const noexcept
{
	return bool(std::strncmp(m_uuid, other.m_uuid, 32ULL) < 0);
}

ecsHandle::operator bool() const noexcept
{
	static const ecsHandle empty;
	return !bool((*this) == empty);
}

bool ecsHandle::isValid() const noexcept 
{
	static const ecsHandle empty;
	return !bool((*this) == empty);
}

EntityHandle::EntityHandle(const ecsHandle& handle) noexcept 
	: ecsHandle(handle) 
{
}

ComponentHandle::ComponentHandle(const ecsHandle& handle) noexcept 
	: ecsHandle(handle) 
{
}