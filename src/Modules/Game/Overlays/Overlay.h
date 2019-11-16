#pragma once
#ifndef OVERLAY_H
#define OVERLAY_H


/** An interface for game overlay effects. */
class Overlay {
public:
	// Public (De)Constructors
	/** Virtual Destructor. */
	inline virtual ~Overlay() = default;
	/** Constructor. */
	inline Overlay() = default;


	// Public Interface
	/** Apply this overlay effect.
	@param	deltaTime	the amount of time passed since last frame. */
	virtual void applyEffect(const float& deltaTime) noexcept = 0;
};

#endif // OVERLAY_H
