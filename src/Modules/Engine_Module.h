#pragma once
#ifndef ENGINE_MODULE_H
#define ENGINE_MODULE_H


class Engine;

/** An interface for engine modules to implement. */
class Engine_Module {
public:
	// Public (De)Constructors
	/** Destroy this engine module. */
	inline virtual ~Engine_Module() noexcept = default;
	/** Construct an engine module.
	@param	engine	reference to the engine to use. */
	explicit Engine_Module(Engine& engine) noexcept;


	// Public Interface Declarations
	/** Initialize the module. */
	virtual void initialize();
	/** De-initialize the module. */
	virtual void deinitialize();


protected:
	// Protected Attributes
	Engine& m_engine;


private:
	// Private and deleted
	/** Disallow module move constructor. */
	inline Engine_Module(Engine_Module&&) noexcept = delete;
	/** Disallow module copy constructor. */
	inline Engine_Module(const Engine_Module&) noexcept = delete;
	/** Disallow module move assignment. */
	inline const Engine_Module& operator =(Engine_Module&&) noexcept = delete;
	/** Disallow module copy assignment. */
	inline const Engine_Module& operator =(const Engine_Module&) noexcept = delete;
};

#endif // ENGINE_MODULE_H