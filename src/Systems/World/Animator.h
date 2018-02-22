#pragma once
#ifndef ANIMATOR
#define ANIMATOR
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

class System_World;


/**
 * A utility that controls animation-supporting objects.
 **/
class DT_ENGINE_API Animator 
{
public:
	// (de)Constructors
	/** Destroy the animation utility. */
	~Animator() {}
	/** Construct the animation utility. */
	Animator(System_World * world) : m_world(world) {}

	
	// Public Methods
	/** Increment all things that need to be animated by deltaTime
	 * @param	deltaTime	the amount of time to update by */
	void animate(const float & deltaTime);


private:
	// Private Attributes
	System_World *m_world;
};

#endif // ANIMATOR