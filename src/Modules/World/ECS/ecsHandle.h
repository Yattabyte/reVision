#pragma once
#ifndef ECSHANDLE_H
#define ECSHANDLE_H

#include <cstring>
#include <iterator>
#include <utility>


/** A structure used to uniquely identify elements in the engine's ECS 'system'. */
struct ecsHandle {
	char uuid[32] = { '\0' };
	/** Default Constructor. */
	ecsHandle() = default;
	/** E/xplicit Constructor. */
	explicit ecsHandle(const char id[32]) {
		std::memcpy(uuid, id, size_t(sizeof(char) * 32));
	}
	/** Copy Constructor. */
	ecsHandle(const ecsHandle& other) {
		std::memcpy(uuid, other.uuid, size_t(sizeof(char) * 32));
	}
	/** Move Constructor. */
	ecsHandle(ecsHandle&& other) noexcept {
		std::move(std::begin(other.uuid), std::end(other.uuid), uuid);
	}
	// Copy from another handle
	inline ecsHandle& operator=(const ecsHandle& other) noexcept {
		std::memcpy(uuid, other.uuid, size_t(sizeof(char) * 32));
		return *this;
	}
	// Compare against another handle
	inline bool operator==(const ecsHandle& other) const {
		return bool(std::strncmp(uuid, other.uuid, 32ull) == 0);
	}
	// Compare if this should be ordered before another handle
	inline bool operator<(const ecsHandle& other) const {
		return bool(std::strncmp(uuid, other.uuid, 32ull) < 0);
	}
	// Return if handle is valid
	inline operator bool() const {
		constexpr const ecsHandle empty;
		return !bool((*this) == empty);
	}
	// Return if handle is valid
	inline bool isValid() const {
		constexpr const ecsHandle empty;
		return !bool((*this) == empty);
	}
};

#endif // ECSHANDLE_H