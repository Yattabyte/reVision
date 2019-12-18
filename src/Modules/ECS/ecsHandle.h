#pragma once
#ifndef ECS_HANDLE_H
#define ECS_HANDLE_H


/** A structure used to uniquely identify elements in the engine's ECS 'system'. */
struct ecsHandle {
public:
	/** The UUID container. */
	char m_uuid[32] = { '\0' };
	/** Destroy this handle. */
	inline virtual ~ecsHandle() noexcept = default;
	/** Construct an empty handle. */
	inline ecsHandle() noexcept = default;
	/** Construct a specific handle.
	@param	id[32]		specific handle name as char array of size 32. */
	explicit ecsHandle(const char id[32]) noexcept;
	/** Copy Constructor. 
	@param	other		an other handle to copy from. */
	ecsHandle(const ecsHandle& other) noexcept;
	/** Move Constructor. 
	@param	other		an other handle to move from. */
	ecsHandle(ecsHandle&& other) noexcept;
	/** Copy from another handle.
	@param	other		an other handle to copy from. 
	@return				reference to this. */
	ecsHandle& operator=(const ecsHandle& other) noexcept;
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

struct EntityHandle final : ecsHandle {
	inline ~EntityHandle() noexcept = default;
	inline EntityHandle() noexcept = default;
	inline EntityHandle(EntityHandle&&) noexcept = default;
	inline EntityHandle(const EntityHandle&) noexcept = default;
	explicit EntityHandle(const ecsHandle& handle) noexcept;
	EntityHandle& operator=(const EntityHandle& other) noexcept;
};
struct ComponentHandle final : ecsHandle {
	inline ~ComponentHandle() noexcept = default;
	inline ComponentHandle() noexcept = default;
	inline ComponentHandle(ComponentHandle&&) noexcept = default;
	inline ComponentHandle(const ComponentHandle&) noexcept = default;
	explicit ComponentHandle(const ecsHandle& handle) noexcept;
	ComponentHandle& operator=(const ComponentHandle& other) noexcept;
};

#endif // ECS_HANDLE_H