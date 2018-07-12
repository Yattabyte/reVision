#pragma once
#ifndef FX_TECHNIQUE_H
#define FX_TECHNIQUE_H


/**
 * An interface for applying post processing effects in the rendering pipeline.
 **/
class FX_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~FX_Technique() {};
	/** Constructor. */
	FX_Technique() {}


	// Public Methods
	/** Return std::string name of this technique.
	 * @return	std::string name of this technique */
	virtual const char * getName() const = 0;
	/** Apply this lighting technique. */
	virtual void applyEffect() = 0;
	/** Bind the output textures of this effect as input textures for something else to use. */
	virtual void bindForReading() = 0;
};

#endif // FX_TECHNIQUE_H