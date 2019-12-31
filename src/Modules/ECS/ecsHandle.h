#pragma once
#ifndef ECS_HANDLE_H
#define ECS_HANDLE_H


/** A structure used to uniquely identify elements in the engine's ECS 'system'. */
struct ecsHandle {
	/** The UUID container. */
	char m_uuid[32] = { '\0' };
	/** Destroy this handle. */
	inline virtual ~ecsHandle() = default;
	/** Construct an empty handle. */
	inline ecsHandle() noexcept = default;
	/** Construct a specific handle.
	@param	id			specific handle name as char array of size 32. */
	explicit ecsHandle(const char id[32]);
	/** Copy Constructor.
	@param	other		an other handle to copy from. */
	inline ecsHandle(const ecsHandle& other) noexcept = default;
	/** Move Constructor.
	@param	other		an other handle to move from. */
	inline ecsHandle(ecsHandle&& other) noexcept = default;
	/** Move from another handle.
	@param	other		an other handle to move from.
	@return				reference to this. */
	inline ecsHandle& operator=(ecsHandle&& other) = default;
	/** Copy from another handle.
	@param	other		an other handle to copy from.
	@return				reference to this. */
	inline ecsHandle& operator=(const ecsHandle& other) = default;
	/** Compare against another handle.
	@param	other		an other handle to compare against.
	@return				true if this handle is the same as the other handle, false otherwise. */
	bool operator==(const ecsHandle& other) const noexcept;
	/** Compare if this should be ordered before another handle.
	@param	other		an other handle to compare against.
	@return				true if this handle is the less than the other handle, false otherwise. */
	bool operator<(const ecsHandle& other) const noexcept;
	/** Conversion to bool operator.
	@return				true if this handle is valid, false otherwise. */
	operator bool() const noexcept;
	/** Retrieve if this handle is valid.
	@return				true if this handle is valid, false otherwise. */
	bool isValid() const noexcept;
};

/** Specialized handle for labeling Entities. */
struct EntityHandle final : ecsHandle {
	inline ~EntityHandle() = default;
	inline EntityHandle() noexcept = default;
	inline EntityHandle(EntityHandle&&) noexcept = default;
	inline EntityHandle(const EntityHandle&) noexcept = default;
	explicit EntityHandle(const ecsHandle& handle) noexcept;
	inline EntityHandle& operator=(EntityHandle&& other) noexcept = default;
	inline EntityHandle& operator=(const EntityHandle& other) noexcept = default;
};

/** Specialized handle for labeling Components. */
struct ComponentHandle final : ecsHandle {
	inline ~ComponentHandle() = default;
	inline ComponentHandle() noexcept = default;
	inline ComponentHandle(ComponentHandle&&) noexcept = default;
	inline ComponentHandle(const ComponentHandle&) noexcept = default;
	explicit ComponentHandle(const ecsHandle& handle) noexcept;
	inline ComponentHandle& operator=(ComponentHandle&& other) noexcept = default;
	inline ComponentHandle& operator=(const ComponentHandle& other) noexcept = default;
};

#endif // ECS_HANDLE_H