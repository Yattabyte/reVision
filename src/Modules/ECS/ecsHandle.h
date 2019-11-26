#pragma once
#ifndef ECS_HANDLE_H
#define ECS_HANDLE_H

#include <cstring>
#include <iterator>
#include <utility>


/** A structure used to uniquely identify elements in the engine's ECS 'system'. */
struct ecsHandle {
	char uuid[32] = { '\0' };
	/** Default Constructor. */
	inline ecsHandle() = default;
	/** Explicit Constructor. */
	inline explicit ecsHandle(const char id[32]) noexcept {
		std::copy(&id[0], &id[32], &uuid[0]);
	}
	/** Copy Constructor. */
	inline ecsHandle(const ecsHandle& other) noexcept {
		std::copy(&other.uuid[0], &other.uuid[32], &uuid[0]);
	}
	/** Move Constructor. */
	inline ecsHandle(ecsHandle&& other) noexcept {
		std::move(std::begin(other.uuid), std::end(other.uuid), uuid);
	}
	// Copy from another handle
	inline ecsHandle& operator=(const ecsHandle& other) noexcept {
		std::copy(&other.uuid[0], &other.uuid[32], &uuid[0]);
		return *this;
	}
	// Compare against another handle
	inline bool operator==(const ecsHandle& other) const noexcept {
		return bool(std::strncmp(uuid, other.uuid, 32ull) == 0);
	}
	// Compare if this should be ordered before another handle
	inline bool operator<(const ecsHandle& other) const noexcept {
		return bool(std::strncmp(uuid, other.uuid, 32ull) < 0);
	}
	// Return if handle is valid
	inline operator bool() const noexcept {
		constexpr const ecsHandle empty;
		return !bool((*this) == empty);
	}
	// Return if handle is valid
	inline bool isValid() const noexcept {
		constexpr const ecsHandle empty;
		return !bool((*this) == empty);
	}
};

struct EntityHandle final : ecsHandle {
	inline EntityHandle() = default;
	inline explicit EntityHandle(const ecsHandle& handle) noexcept : ecsHandle(handle) {}
};
struct ComponentHandle final : ecsHandle {
	inline ComponentHandle() = default;
	explicit ComponentHandle(const ecsHandle& handle) noexcept : ecsHandle(handle) {}
};

#endif // ECS_HANDLE_H
