#pragma once
#ifndef OVERLAY_H
#define OVERLAY_H


/** An interface for game overlay effects. */
class Overlay {
public:
	// Public (De)Constructors
	/** Virtual Destructor. */
	inline virtual ~Overlay() = default;
	/** Default constructor. */
	inline Overlay() noexcept = default;
	/** Move constructor. */
	inline Overlay(Overlay&&) noexcept = default;
	/** Copy constructor. */
	inline Overlay(const Overlay&) noexcept = default;

	// Public Methods
	/** Move assignment. */
	inline Overlay& operator =(Overlay&&) noexcept = default;
	/** Copy assignment. */
	inline Overlay& operator =(const Overlay&) noexcept = default;

	// Public Interface
	/** Apply this overlay effect.
	@param	deltaTime	the amount of time passed since last frame. */
	virtual void applyEffect(const float& deltaTime) = 0;
};

#endif // OVERLAY_H