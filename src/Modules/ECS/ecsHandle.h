#pragma once
#ifndef ECS_HANDLE_H
#define ECS_HANDLE_H

#include <cstring>
#include <iterator>
#include <utility>


/** A structure used to uniquely identify elements in the engine's ECS 'system'. */
struct ecsHandle {
	char uuid[32] = { '\0' };
	/** Destroy this handle. */
	inline virtual ~ecsHandle() = default;
	/** Construct an empty handle. */
	inline ecsHandle() = default;
	/** Construct a specific handle.
	@param	id[32]		specific handle name as char array of size 32. */
	inline explicit ecsHandle(const char id[32]) noexcept {
		std::copy(&id[0], &id[32], &uuid[0]);
	}
	/** Copy Constructor. 
	@param	other		an other handle to copy from. */
	inline ecsHandle(const ecsHandle& other) noexcept {
		std::copy(&other.uuid[0], &other.uuid[32], &uuid[0]);
	}
	/** Move Constructor. 
	@param	other		an other handle to move from. */
	inline ecsHandle(ecsHandle&& other) noexcept {
		std::move(std::begin(other.uuid), std::end(other.uuid), uuid);
	}
	/** Copy from another handle.
	@param	other		an other handle to copy from. 
	@return				reference to this. */
	inline ecsHandle& operator=(const ecsHandle& other) noexcept {
		std::copy(&other.uuid[0], &other.uuid[32], &uuid[0]);
		return *this;
	}
	/** Compare against another handle.
	@param	other		an other handle to compare against.
	@return				true if this handle is the same as the other handle, false otherwise. */
	inline bool operator==(const ecsHandle& other) const noexcept {
		return bool(std::strncmp(uuid, other.uuid, 32ull) == 0);
	}
	/** Compare if this should be ordered before another handle. 
	@param	other		an other handle to compare against.
	@return				true if this handle is the less than the other handle, false otherwise. */
	inline bool operator<(const ecsHandle& other) const noexcept {
		return bool(std::strncmp(uuid, other.uuid, 32ull) < 0);
	}
	/** Conversion to bool operator.
	@return				true if this handle is valid, false otherwise. */
	inline operator bool() const noexcept {
		static const ecsHandle empty;
		return !bool((*this) == empty);
	}
	/** Retrieve if this handle is valid.
	@return				true if this handle is valid, false otherwise. */
	inline bool isValid() const noexcept {
		static const ecsHandle empty;
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
