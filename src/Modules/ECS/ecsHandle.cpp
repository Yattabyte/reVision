#include "Modules/ECS/ecsHandle.h"
#include <algorithm>


ecsHandle::ecsHandle(const char id[32])
{
	std::copy(&id[0], &id[32], &m_uuid[0]);
}

ecsHandle::ecsHandle(const ecsHandle& other) noexcept 
{
	std::copy(&other.m_uuid[0], &other.m_uuid[32], &m_uuid[0]);
}

ecsHandle::ecsHandle(ecsHandle&& other)
{
	std::move(std::begin(other.m_uuid), std::end(other.m_uuid), m_uuid);
}

ecsHandle& ecsHandle::operator=(const ecsHandle& other)
{
	std::copy(&other.m_uuid[0], &other.m_uuid[32], &m_uuid[0]);
	return *this;
}

bool ecsHandle::operator==(const ecsHandle& other) const noexcept
{
	return bool(std::strncmp(m_uuid, other.m_uuid, 32ull) == 0);
}

bool ecsHandle::operator<(const ecsHandle& other) const noexcept
{
	return bool(std::strncmp(m_uuid, other.m_uuid, 32ull) < 0);
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

EntityHandle& EntityHandle::operator=(const EntityHandle& other)
{
	std::copy(&other.m_uuid[0], &other.m_uuid[32], &m_uuid[0]);
	return *this;
}

ComponentHandle::ComponentHandle(const ecsHandle& handle) noexcept 
	: ecsHandle(handle) 
{
}

ComponentHandle& ComponentHandle::operator=(const ComponentHandle& other)
{
	std::copy(&other.m_uuid[0], &other.m_uuid[32], &m_uuid[0]);
	return *this;
}
